
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
    : _frameNum(0),
      _sonarFrameNum(0),
  //    _pending(0),
//      _mutex(),
      _writer(FileExtension, AV_CODEC_ID_PRORES),
      _writerThread( _writer ),
      _videoTracks(),
      _dataTrack( doSonar ? new DataTrack(_writer) : nullptr ),
      _gpmfEncoder()
  {
    for( int i = 0; i < numStreams; ++i ) {
      _videoTracks.push_back( std::shared_ptr<VideoTrack>(new VideoTrack(_writer, width, height, frameRate) ) );
    }

    _writer.open( filename );
  }

  VideoRecorder::~VideoRecorder()
  {
    sleep(2);
    _writer.close();
  }

  bool VideoRecorder::addMats( const std::vector<cv::Mat> &mats ) {

    std::deque< std::shared_ptr<std::thread> > recordThreads;

    for( unsigned int i=0; i < mats.size(); ++i ) {
       recordThreads.push_back( shared_ptr<std::thread>( new std::thread( &VideoRecorder::addMat, this, mats[i],  _frameNum, i ) ) );

       // AVPacket *pkt = _videoTracks[i]->encodeFrame( mats[i], _frameNum );
       // _writerThread.writePacket( pkt );
    }

    // // Wait for all recorder threads
    for( auto thread : recordThreads ) thread->join();

    ++_frameNum;

    return true;
  }


  bool VideoRecorder::addMat( const cv::Mat &image, unsigned int frameNum, unsigned int stream )
  {
    AVPacket *pkt = _videoTracks[stream]->encodeFrame( image, frameNum );
    if( !pkt ) return false;

    // Insert into the _writerThread queue
    _writerThread.writePacket( pkt );
    return true;
  }

  bool VideoRecorder::addSonar( const liboculus::SimplePingResult &ping, const std::chrono::time_point< std::chrono::system_clock > timePt ) {

    if( !_dataTrack ) return false;
    //
    // {
    //   std::lock_guard<std::mutex> lock(_mutex);
    //   ++_pending;
    // }

    {
      uint32_t *buffer;

      auto payloadSize = _gpmfEncoder.writeSonar( ping, &buffer, 0 );
      if( payloadSize == 0) {
        LOG(WARNING) << "Failed to write sonar!";
        // {
        //   std::lock_guard<std::mutex> lock(_mutex);
        //   --_pending;
        // }
        return false;
      }

      //LOG(WARNING) << "   packet->pts " << pkt->pts;
      AVPacket *pkt = _dataTrack->encodeData( (uint8_t *)buffer, payloadSize, timePt );
      _writerThread.writePacket( pkt );

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


  //=================================================================

  VideoRecorder::VideoWriterThread::VideoWriterThread( libvideoencoder::VideoWriter &writer )
    : _writer(writer),
      _thread( active_object::Active::createActive() )
    {;}

  VideoRecorder::VideoWriterThread::~VideoWriterThread()
    {;}

  void VideoRecorder::VideoWriterThread::writePacket( AVPacket *packet ) {
    if( _thread->size() > 10 ) {
      LOG( WARNING ) << "More than 10 frames queued ... skipping";
      return;
    }

    _thread->send( std::bind( &VideoRecorder::VideoWriterThread::writePacketImpl, this, packet ) );
  }

  void VideoRecorder::VideoWriterThread::writePacketImpl( AVPacket *packet ) {
    _writer.writePacket( packet );
  }




}
