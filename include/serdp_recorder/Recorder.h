#pragma once

#include <memory>
#include <fstream>
#include <chrono>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <opencv2/opencv.hpp>

#include "liboculus/SimplePingResult.h"

namespace serdp_recorder {

  /// Abstract base class for a recorder which an handle multiple tracks of
  /// video (supplied at cv:Mats) and a single track of sonar data
  class Recorder {
  public:
    Recorder()
      {;}

    virtual ~Recorder()
      {;}

    virtual bool addMats( std::vector<cv::Mat> &mats ) = 0;
    virtual bool addSonar( const std::shared_ptr<liboculus::SimplePingResult> &ping,
                          const std::chrono::time_point< std::chrono::system_clock > time = std::chrono::system_clock::now()) = 0;

  };

}
