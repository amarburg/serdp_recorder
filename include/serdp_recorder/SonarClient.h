#pragma once

#include <string>
#include <thread>
#include "serdp_recorder/IoServiceThread.h"

#include "liboculus/StatusRx.h"
#include "liboculus/DataRx.h"
using namespace liboculus;

#include "serdp_recorder/VideoRecorder.h"
#include "serdp_common/OpenCVDisplay.h"

namespace serdprecorder {

  using namespace serdp_common;

  class SonarClient {
  public:

    SonarClient( const std::string &ipAddr,
                  const shared_ptr<Recorder> &recorder = shared_ptr<Recorder>(nullptr),
                  const shared_ptr<OpenCVDisplay> &display = shared_ptr<OpenCVDisplay>(nullptr) );

    ~SonarClient();

    void start();

    void receiveStatus( const SonarStatus & status );
    void receivePing( const shared_ptr<SimplePingResult> &ping );

    void stop();

    unsigned int pingCount() const { return _pingCount; }

  protected:

    // Runs in thread
    void run();

  private:

    std::string _ipAddr;
    std::thread _thread;

    shared_ptr<Recorder> _recorder;
    shared_ptr<OpenCVDisplay> _display;

    IoServiceThread _ioSrv;
    StatusRx _statusRx;
    std::unique_ptr<DataRx> _dataRx;

    unsigned int _pingCount;

  };
}
