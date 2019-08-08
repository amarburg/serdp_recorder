
#include <ctime>
#include <memory>

#include "libg3logger/g3logger.h"

#include "serdp_recorder/VideoRecorder.h"

#include "gpmf-write/GPMF_writer.h"

using namespace libvideoencoder;

namespace serdprecorder {

  using namespace std;

  static char FileExtension[] = "mov";


  //=================================================================

  VideoRecorder::VideoRecorder( )
    : GPMFRecorder(),
      _frameNum(0),
      _doSonar( false ),
      _sonarTrack( -1 ),
      _outputDir( "/tmp/" ),
      _isReady( false ),
      _pending(0),
      _mutex(),
      _encoder( new Encoder(FileExtension, AV_CODEC_ID_PRORES) ),
      _writer(nullptr)
  {
    ;
  }

  VideoRecorder::~VideoRecorder()
  {
    if( _writer ) _writer.reset();
    if( _encoder ) _encoder.reset();
  }


  bool VideoRecorder::open( int width, int height, float frameRate, int numStreams ) {
    auto filename = makeFilename();

    _writer.reset( _encoder->makeWriter() );
    CHECK( _writer != nullptr );

    _writer->addVideoTrack( width, height, frameRate, numStreams );

    if( _doSonar )
      _sonarTrack = _writer->addDataTrack();

    LOG(INFO) << "Opening video file " << filename;

    _writer->open(filename.string());

    _frameNum = 0;
    _sonarWritten = 0;
    _startTime = std::chrono::system_clock::now();

    flushGPMF();

    _isReady = true;
    {
      std::lock_guard<std::mutex> lock(_mutex);
      _pending = 0;
    }
    return true;
  }


  void VideoRecorder::close() {

    while(true) {
      std::lock_guard<std::mutex> lock(_mutex);
      if( _pending == 0 ) break;
    }

    LOG(INFO) << "Closing video with " << _frameNum << " frames";
    LOG_IF(INFO, _doSonar) << "     Wrote " << _sonarWritten << " frames of sonar";

    _isReady = false;
    _frameNum = 0;
    _writer.reset();
  }


  bool VideoRecorder::addMats( std::vector<cv::Mat> &mats ) {
    if( !isRecording() ) return false;

    std::deque< std::shared_ptr<std::thread> > recordThreads;

    // Video corruption when this encoding is split into two threads.
    for( unsigned int i=0; i < mats.size(); ++i ) {
       recordThreads.push_back( shared_ptr<std::thread>( new std::thread( &VideoRecorder::addMat, this, mats[i], i ) ) );
    }

    // // Wait for all recorder threads
    for( auto thread : recordThreads ) thread->join();

    advanceFrame();

    return true;
  }


  bool VideoRecorder::addMat( cv::Mat image, unsigned int stream ) {
      if( !isRecording() ) return false;

    if( !_writer ) return false;

    {
      std::lock_guard<std::mutex> lock(_mutex);
      ++_pending;
    }

    //Convert to AVFrame
    AVFrame *frame = av_frame_alloc();   ///avcodec_alloc_frame();
    CHECK( frame != nullptr ) << "Cannot create frame";

    auto sz = image.size();

    frame->width = sz.width;
    frame->height = sz.height;
    frame->format = AV_PIX_FMT_BGRA;
    auto res = av_frame_get_buffer(frame, 0);

    // Try this_`
    // memcpy for now
    cv::Mat frameMat( sz.height, sz.width, CV_8UC4, frame->data[0]);
    image.copyTo( frameMat );

    res = _writer->addFrame( frame, _frameNum, stream );

    av_frame_free( &frame );

    {
      std::lock_guard<std::mutex> lock(_mutex);
      --_pending;
    }

    return res;
  }

  bool VideoRecorder::addSonar( const std::shared_ptr<liboculus::SimplePingResult> &ping ) {
    if( !isRecording() ) return false;

    if( !_writer ) return false;
    if( _sonarTrack < 0 ) return false;

    {
      std::lock_guard<std::mutex> lock(_mutex);
      ++_pending;
    }

    {
      uint32_t *buffer;

      auto payloadSize = writeSonar( ping, &buffer, 0 );
      if( payloadSize == 0) {
        LOG(WARNING) << "Failed to write sonar!";
        {
          std::lock_guard<std::mutex> lock(_mutex);
          --_pending;
        }
        return false;
      }

      // And data track
      AVPacket *pkt = av_packet_alloc();
      av_packet_from_data(pkt, (uint8_t *)buffer, payloadSize );

      pkt->stream_index = _sonarTrack;

      std::chrono::microseconds timePt =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - _startTime);

      pkt->dts = timePt.count();
      pkt->pts = pkt->dts;

      LOG(WARNING) << "   packet->pts " << pkt->pts;

      ++_sonarWritten;
      _writer->addPacket( pkt );
    }

    {
      std::lock_guard<std::mutex> lock(_mutex);
      --_pending;
    }
    return true;
  }


  fs::path VideoRecorder::makeFilename() {

    std::time_t t = std::time(nullptr);

    char mbstr[100];
    std::strftime(mbstr, sizeof(mbstr), "vid_%Y%m%d_%H%M%S", std::localtime(&t));

    // TODO.  Test if file is existing

    strcat( mbstr, ".");
    strcat( mbstr, FileExtension);

    return fs::path(_outputDir) /= mbstr;
  }



}
