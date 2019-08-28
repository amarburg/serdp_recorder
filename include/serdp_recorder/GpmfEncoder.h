#pragma once

#include <memory>
#include <fstream>
#include <chrono>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <opencv2/opencv.hpp>

#include "liboculus/SimplePingResult.h"

namespace serdp_recorder {

  /// Specialization of Recorder which saves the sonar data in a GPMF stream;
  /// Ignores any video Mats.
  /// Writes resulting GPMF-encoded stream to a file
  class GPMFEncoder {
  public:

    GPMFEncoder( );
    virtual ~GPMFEncoder();

    size_t writeSonar( const std::shared_ptr<liboculus::SimplePingResult> &ping, uint32_t **buffer, size_t bufferSize );

    void free( uint32_t *buffer );


  protected:

    void initGPMF();
    void flushGPMF();

    std::unique_ptr<uint32_t> _scratch;

    size_t _gpmfHandle;
    size_t _sonarHandle;

  };

}
