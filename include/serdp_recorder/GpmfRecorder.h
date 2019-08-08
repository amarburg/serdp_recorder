#pragma once

#include <memory>
#include <fstream>
#include <chrono>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <opencv2/opencv.hpp>

#include "libvideoencoder/VideoEncoder.h"

#include "liboculus/SimplePingResult.h"

#include "serdp_recorder/Recorder.h"

namespace serdprecorder {

  /// Specilization of Recorder which saves the sonar data in a GPMF stream;
  /// Writes resulting GPMF stream to a file
  class GPMFRecorder : public Recorder {
  public:

    GPMFRecorder();
    //VideoRecorder( const fs::path &outputDir, bool doSonar = false );
    virtual ~GPMFRecorder();

    bool open( const std::string &filename );
    void close();

    bool isRecording() const { return _out.is_open(); }

    virtual bool addMats( std::vector<cv::Mat> &mats ) { return false; }
    virtual bool addSonar( const std::shared_ptr<liboculus::SimplePingResult> &ping );

  protected:

    void initGPMF();
    void flushGPMF();
    size_t writeSonar( const std::shared_ptr<liboculus::SimplePingResult> &ping, uint32_t **buffer, size_t bufferSize );


    std::unique_ptr<uint32_t> _scratch;

    std::ofstream _out;

    size_t _gpmfHandle;
    size_t _sonarHandle;

  };

}
