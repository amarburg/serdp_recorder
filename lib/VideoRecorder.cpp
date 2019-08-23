
#include <ctime>
#include <memory>

#include "libg3logger/g3logger.h"

#include "serdp_recorder/VideoRecorder.h"

#include "gpmf-write/GPMF_writer.h"

using namespace libvideoencoder;

namespace serdp_recorder {

  using namespace std;

  static char FileExtension[] = "mov";

  using namespace libvideoencoder;

  //=================================================================

  VideoRecorder::VideoRecorder( const std::string &filename, int width, int height,
            float frameRate, int numStreams, bool doSonar )
    : GPMFRecorder(),
      _frameNum(0),
      _sonarFrameNum(0),
  //    _pending(0),
//      _mutex(),
      _writer(FileExtension, AV_CODEC_ID_PRORES),
      _videoTracks(),
      _dataTrack( doSonar ? new DataTrack(_writer) : nullptr )
  {
    for( int i = 0; i < numStreams; ++i )
      _videoTracks.push_back( std::shared_ptr<VideoTrack>(new VideoTrack(_writer, width, height, frameRate) ) );

    _writer.open( filename );
  }

  VideoRecorder::~VideoRecorder()
  {
    _writer.close();
  }

  bool VideoRecorder::addMats( const std::vector<cv::Mat> &mats ) {
    if( !isRecording() ) return false;

    std::deque< std::shared_ptr<std::thread> > recordThreads;

    // Video corruption when this encoding is split into two threads.
    for( unsigned int i=0; i < mats.size(); ++i ) {
       recordThreads.push_back( shared_ptr<std::thread>( new std::thread( &VideoRecorder::addMat, this, mats[i], i ) ) );
    }

    // // Wait for all recorder threads
    for( auto thread : recordThreads ) thread->join();

    ++_frameNum;

    return true;
  }


  bool VideoRecorder::addMat( const cv::Mat &image, unsigned int stream ) {

    // {
    //   std::lock_guard<std::mutex> lock(_mutex);
    //   ++_pending;
    // }

    // TODO:  Validate valid stream number
    // TODO:  Validate image.size() == videoTrack.size()

    auto sz = image.size();
    AVFrame *frame = _videoTracks[stream]->makeFrame();

    CHECK( frame->format == AV_PIX_FMT_RGB24 );
    CHECK( image.type() == CV_8UC3 );

    // This depends on both the image and the frame being 24bit RGB
    // memcpy for now
    cv::Mat frameMat( sz.height, sz.width, CV_8UC3, frame->data[0]);
    image.copyTo( frameMat );

    auto res = _videoTracks[stream]->addFrame( frame, _frameNum );

    av_frame_free( &frame );

    // {
    //   std::lock_guard<std::mutex> lock(_mutex);
    //   --_pending;
    // }

    return res;
  }

  bool VideoRecorder::addSonar( const std::shared_ptr<liboculus::SimplePingResult> &ping ) {

    if( !_dataTrack ) return false;
    //
    // {
    //   std::lock_guard<std::mutex> lock(_mutex);
    //   ++_pending;
    // }

    {
      uint32_t *buffer;

      auto payloadSize = writeSonar( ping, &buffer, 0 );
      if( payloadSize == 0) {
        LOG(WARNING) << "Failed to write sonar!";
        // {
        //   std::lock_guard<std::mutex> lock(_mutex);
        //   --_pending;
        // }
        return false;
      }

      //LOG(WARNING) << "   packet->pts " << pkt->pts;

      std::chrono::microseconds timePt =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - _startTime);
      _dataTrack->writeData( (uint8_t *)buffer, payloadSize,  timePt.count() );

      ++_sonarFrameNum;
    }

    // {
    //   std::lock_guard<std::mutex> lock(_mutex);
    //   --_pending;
    // }
    return true;
  }


  fs::path VideoRecorder::MakeFilename( std::string &outputDir ) {

    std::time_t t = std::time(nullptr);

    char mbstr[100];
    std::strftime(mbstr, sizeof(mbstr), "vid_%Y%m%d_%H%M%S", std::localtime(&t));

    // TODO.  Test if file is existing

    strcat( mbstr, ".");
    strcat( mbstr, FileExtension);

    return fs::path(outputDir) /= mbstr;
  }



}
