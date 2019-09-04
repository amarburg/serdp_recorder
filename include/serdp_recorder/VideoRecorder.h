#pragma once

#include <memory>
#include <fstream>
#include <chrono>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <opencv2/opencv.hpp>

#include "active_object/active.h"

#include "libvideoencoder/VideoWriter.h"
#include "libvideoencoder/OutputTrack.h"

#include "liboculus/SimplePingResult.h"

#include "serdp_recorder/GpmfEncoder.h"

namespace serdp_recorder {


  class VideoRecorder {
  protected:

    class VideoWriterThread {
    public:
      VideoWriterThread( libvideoencoder::VideoWriter &writer );
      ~VideoWriterThread();

      void writePacket( AVPacket *packet );

    protected:

      void writePacketImpl( AVPacket *packet );

      libvideoencoder::VideoWriter &_writer;
      std::unique_ptr<active_object::Active> _thread;
    };



  public:

    VideoRecorder( const std::string &filename, int width, int height, float frameRate, int numVideoStreams = 1, bool doSonar = false );
    virtual ~VideoRecorder();

    virtual bool addMats( const std::vector<cv::Mat> &mats );
    bool addMat( const cv::Mat &image, unsigned int frameNum, unsigned int stream=0  );

    virtual bool addSonar( const std::shared_ptr<liboculus::SimplePingResult> &ping,
                          const std::chrono::time_point< std::chrono::system_clock > time = std::chrono::system_clock::now() );

    static fs::path MakeFilename( std::string &outputDir );


  protected:

    bool open( );
    void close();

    unsigned int _frameNum;
    unsigned int _sonarFrameNum;

    //std::mutex _mutex;
    std::chrono::time_point< std::chrono::system_clock > _startTime;

    libvideoencoder::VideoWriter _writer;
    VideoWriterThread _writerThread;

    std::vector< std::shared_ptr<libvideoencoder::VideoTrack> > _videoTracks;
    std::shared_ptr<libvideoencoder::DataTrack> _dataTrack;

    GPMFEncoder _gpmfEncoder;
  };


}
