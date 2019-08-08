#pragma once

#include <memory>
#include <fstream>
#include <chrono>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <opencv2/opencv.hpp>

#include "libvideoencoder/VideoEncoder.h"

#include "liboculus/SimplePingResult.h"

namespace serdprecorder {

  /// Abstract base class for a recorder which an handle multiple tracks of
  /// video (supplied at cv:Mats) and a single track of sonar data
  class Recorder {
  public:
    Recorder()
      {;}
      
    virtual ~Recorder()
      {;}

    virtual bool addMats( std::vector<cv::Mat> &mats ) = 0;
    virtual bool addSonar( const std::shared_ptr<liboculus::SimplePingResult> &ping ) = 0;

  };

}
