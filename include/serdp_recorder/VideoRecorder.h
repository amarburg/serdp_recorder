#pragma once

#include <memory>
#include <fstream>
#include <chrono>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <opencv2/opencv.hpp>

#include "libvideoencoder/VideoEncoder.h"

#include "liboculus/SimplePingResult.h"

#include "serdp_recorder/GpmfRecorder.h"

namespace serdprecorder {

  class VideoRecorder : public GPMFRecorder {
  public:

    VideoRecorder();
    //VideoRecorder( const fs::path &outputDir, bool doSonar = false );
    virtual ~VideoRecorder();

    bool setDoSonar( bool d )                   { return _doSonar = d; }
    void setOutputDir( const std::string &dir ) { _outputDir = dir; }

    bool open( int width, int height, float frameRate, int numStreams = 1);

    void close();

    bool isRecording() const { return bool(_writer != nullptr) && _isReady; }

    virtual bool addMats( std::vector<cv::Mat> &mats );

    bool addMat( cv::Mat image, unsigned int stream = 0  );
    //bool addFrame( AVFrame *frame, unsigned int stream = 0 );

    virtual bool addSonar( const std::shared_ptr<liboculus::SimplePingResult> &ping );

    void advanceFrame() { ++_frameNum; }

    fs::path makeFilename();

  protected:

    int _frameNum;

    unsigned int _sonarWritten;

    bool _doSonar;
    int _sonarTrack;

    fs::path _outputDir;
    bool _isReady;

    unsigned int _pending;
    std::mutex _mutex;

    std::chrono::time_point< std::chrono::system_clock > _startTime;

    std::shared_ptr<libvideoencoder::Encoder> _encoder;
    std::shared_ptr<libvideoencoder::VideoWriter> _writer;


  };


}
