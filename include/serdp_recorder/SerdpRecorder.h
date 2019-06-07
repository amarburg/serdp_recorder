#pragma once


#include <CLI/CLI.hpp>

#include <libg3logger/g3logger.h>

#include "libblackmagic/DeckLink.h"
#include "libblackmagic/DataTypes.h"
using namespace libblackmagic;

#include "libbmsdi/helpers.h"

#include "libvideoencoder/VideoEncoder.h"
using libvideoencoder::Encoder;

#include "serdp_recorder/CameraState.h"
#include "serdp_recorder/VideoRecorder.h"
#include "serdp_recorder/SonarClient.h"
#include "serdp_common/OpenCVDisplay.h"


using cv::Mat;


namespace serdprecorder {

  class SerdpRecorder {
  public:

    SerdpRecorder();

    bool keepGoing( bool kg ) { return _keepGoing = kg; }

    int run( int argc, char **argv );

    void handleKey( const char c );

  protected:

    bool _keepGoing;

    std::shared_ptr<DeckLink> _deckLink;
    std::shared_ptr<CameraState> _camState;
    std::shared_ptr<VideoRecorder> _recorder;
    std::shared_ptr<SonarClient> _sonar;
    std::shared_ptr<OpenCVDisplay> _display;


  };

}
