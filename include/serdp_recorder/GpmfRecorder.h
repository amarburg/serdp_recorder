#pragma once

#include <memory>
#include <fstream>
#include <chrono>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <opencv2/opencv.hpp>

#include "liboculus/SimplePingResult.h"

#include "serdp_recorder/Recorder.h"

namespace serdp_recorder {

  /// Specialization of Recorder which saves the sonar data in a GPMF stream;
  /// Ignores any video Mats.
  /// Writes resulting GPMF-encoded stream to a file
  class GPMFRecorder : public Recorder {
  public:

    GPMFRecorder( const std::string &filename = "" );
    //VideoRecorder( const fs::path &outputDir, bool doSonar = false );
    virtual ~GPMFRecorder();

    bool open( const std::string &filename );
    void close();

    bool isRecording() const { return _out.is_open(); }

    virtual bool addMats( std::vector<cv::Mat> &mats ) { return false; }
    virtual bool addSonar( const std::shared_ptr<liboculus::SimplePingResult> &ping,
                          const std::chrono::time_point< std::chrono::system_clock > time = std::chrono::system_clock::now() );

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
