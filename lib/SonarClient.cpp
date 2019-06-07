

#include "serdp_recorder/SonarClient.h"

#include "serdp_common/draw_sonar.h"

namespace serdprecorder {


  SonarClient::SonarClient( const std::string &ipAddr,
                            const shared_ptr<Recorder> &recorder,
                            const shared_ptr<OpenCVDisplay> &display )
      : _ipAddr( ipAddr ),
      _thread(),
      _recorder( recorder ),
      _display( display ),
      _done( false ),
      _pingCount(0)
  {;}


  SonarClient::~SonarClient()
  {
    stop();
  }

  void SonarClient::start() {
    _thread = std::thread( [=] { run(); } );
  }

  void SonarClient::stop() {
    _done = true;
    if( _thread.joinable() ) {
      _thread.join();
    }
  }


  // Runs in thread
  void SonarClient::run() {
    LOG(DEBUG) << "Starting SonarClient in thread";
    try {
      IoServiceThread ioSrv;

      StatusRx statusRx( ioSrv.service() );
      std::unique_ptr<DataRx> dataRx( nullptr );

      if( _ipAddr != "auto" ) {
        LOG(INFO) << "Connecting to sonar with IP address " << _ipAddr;
        auto addr( boost::asio::ip::address_v4::from_string( _ipAddr ) );

        LOG_IF(FATAL,addr.is_unspecified()) << "Couldn't parse IP address" << _ipAddr;

        dataRx.reset( new DataRx( ioSrv.service(), addr ) );
      }

      ioSrv.start();

      while( !_done ) {

        if( !dataRx ) {

          // Attempt auto detection
          if( statusRx.status().wait_for(std::chrono::seconds(1)) ) {
            if( statusRx.status().valid() ) {
              auto addr( statusRx.status().ipAddr() );
              LOG(INFO) << "Using sonar detected at " << addr;
              dataRx.reset( new DataRx( ioSrv.service(), addr ) );
            }
          } else {
            LOG(INFO) << "No sonars detected, still waiting...";
          }

        } else {

          shared_ptr<SimplePingResult> ping(nullptr);

          if( dataRx->queue().wait_for_pop( ping, std::chrono::milliseconds(100) ) ) {

            ++_pingCount;

            // Do something
            auto valid = ping->validate();
            LOG(DEBUG) << "Got " << (valid ? "valid" : "invalid") << " ping";

            // Send to recorder
            _recorder->addSonar( ping );

            if( _display ) _display->showSonar( ping );
          }
        }

      }

      ioSrv.stop();

    }
    catch (std::exception& e)
    {
      LOG(WARNING) << "Exception: " << e.what();
    }


  }

}
