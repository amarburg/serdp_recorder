

#include "serdp_recorder/SonarClient.h"

#include "serdp_common/DrawSonar.h"

namespace serdprecorder {


  SonarClient::SonarClient( const std::string &ipAddr,
                            const shared_ptr<Recorder> &recorder,
                            const shared_ptr<OpenCVDisplay> &display )
      : _ipAddr( ipAddr ),
      _thread(),
      _recorder( recorder ),
      _display( display ),
      _ioSrv(),
      _statusRx( _ioSrv.service() ),
      _dataRx( nullptr ),
      _pingCount(0)
  {

    if( _ipAddr != "auto" ) {
      LOG(INFO) << "Connecting to sonar with IP address " << _ipAddr;
      auto addr( boost::asio::ip::address_v4::from_string( _ipAddr ) );

      LOG_IF(FATAL,addr.is_unspecified()) << "Couldn't parse IP address" << _ipAddr;

      _dataRx.reset( new DataRx( _ioSrv.service(), addr ) );
    }

  }


  SonarClient::~SonarClient()
  {
    stop();
  }

  void SonarClient::start() {
    _thread = std::thread( [=] { run(); } );
  }

  void SonarClient::stop() {
    _ioSrv.stop();
    if( _thread.joinable() ) {
      _thread.join();
    }
  }


  // Runs in thread
  void SonarClient::run() {
    LOG(DEBUG) << "Starting SonarClient in thread";
    try {

      _ioSrv.start();

      while( !_dataRx ) {

        /// Todo.  Change this to a callback...
        // Attempt auto detection
        if( _statusRx.status().wait_for(std::chrono::seconds(1)) ) {
          if( _statusRx.status().valid() ) {
            auto addr( _statusRx.status().ipAddr() );
            LOG(INFO) << "Using sonar detected at " << addr;
            _dataRx.reset( new DataRx( _ioSrv.service(), addr ) );
          }
        } else {
          LOG(INFO) << "No sonars detected, still waiting...";
        }

      }

      _dataRx->setCallback( std::bind( &SonarClient::receivePing, this, std::placeholders::_1 ) );

      _ioSrv.join();
    }
    catch (std::exception& e)
    {
      LOG(WARNING) << "Exception: " << e.what();
    }

  }


    void SonarClient::receivePing( const shared_ptr<SimplePingResult> &ping ) {

      ++_pingCount;

      // Do something
      auto valid = ping->valid();
      LOG(DEBUG) << "Got " << (valid ? "valid" : "invalid") << " ping";

      // Send to recorder
      _recorder->addSonar( ping );

      if( _display ) _display->showSonar( ping );

  }

}
