#pragma once


#include <CLI/CLI.hpp>

#include "active_object/active.h"

#include <libg3logger/g3logger.h>

#include "libblackmagic/InputOutputClient.h"
#include "libblackmagic/DataTypes.h"
using namespace libblackmagic;

#include "libbmsdi/helpers.h"

#include "liboculus/SonarClient.h"

#include "serdp_recorder/CameraState.h"
#include "serdp_recorder/VideoRecorder.h"

#include "serdp_common/OpenCVDisplay.h"

using cv::Mat;


namespace serdp_recorder {

  using liboculus::SonarClient;
  using liboculus::SimplePingResult;

  using serdp_common::OpenCVDisplay;

  class SerdpRecorder {
  public:

    SerdpRecorder( libg3logger::G3Logger &logger );
    ~SerdpRecorder();

    bool keepGoing( bool kg ) { return _keepGoing = kg; }

    int run( int argc, char **argv );

    void handleKey( const char c );

  protected:

    libg3logger::G3Logger &_logger;

    bool _keepGoing;
    std::string _outputDir;
    bool _doSonar;

    std::shared_ptr<InputOutputClient> _bmClient;

    std::shared_ptr<CameraState> _camState;

    std::shared_ptr<SonarClient> _sonar;

    std::shared_ptr<OpenCVDisplay> _display;
    std::shared_ptr<VideoRecorder> _recorder;

    // Callback from libblackmagic::InputHandler
    void receiveImages( const libblackmagic::InputHandler::MatVector &images );
    void receiveImagesImpl( const libblackmagic::InputHandler::MatVector &images, const std::chrono::time_point< std::chrono::system_clock > time );
    int _displayed;


    // Callback from liboculus::DataRx
    void receivePing( const std::shared_ptr<SimplePingResult> & );
    void receivePingImpl( const std::shared_ptr<SimplePingResult> &, const std::chrono::time_point< std::chrono::system_clock > time );
    int _pingCount;

    std::unique_ptr<active_object::Active> _thread;

  };

}
