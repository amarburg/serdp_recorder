#pragma once

#include <memory>
#include <fstream>
#include <chrono>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <opencv2/opencv.hpp>

#include "libvideoencoder/VideoWriter.h"
#include "libvideoencoder/OutputTrack.h"

#include "liboculus/SimplePingResult.h"

#include "serdp_recorder/GpmfRecorder.h"

namespace serdp_recorder {


  class VideoRecorder : public GPMFRecorder {
  public:

    VideoRecorder( const std::string &filename, int width, int height, float frameRate, int numStreams = 1, bool doSonar = false );
    virtual ~VideoRecorder();

    virtual bool addMats( std::vector<cv::Mat> &mats );

    bool addMat( cv::Mat image, unsigned int stream = 0  );
    virtual bool addSonar( const std::shared_ptr<liboculus::SimplePingResult> &ping );

    static fs::path MakeFilename( std::string &outputDir );


  protected:

    bool open( );
    void close();

    void advanceFrame() { ++_frameNum; }

    int _frameNum;

    unsigned int _sonarFrameNum;

    unsigned int _pending;
    std::mutex _mutex;

    std::chrono::time_point< std::chrono::system_clock > _startTime;

    libvideoencoder::VideoWriter _writer;
    std::vector< std::shared_ptr<libvideoencoder::VideoTrack> > _videoTracks;
    std::shared_ptr<libvideoencoder::DataTrack> _dataTrack;


  };


}
