///
///  Records either live sonar or sonar from a raw binary file to a
///  stream of GPMF records
///

#include <memory>
#include <thread>
#include <string>
#include <fstream>

using std::string;

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <libg3logger/g3logger.h>
#include <CLI/CLI.hpp>

#include <opencv2/highgui.hpp>

#include "liboculus/SonarPlayer.h"
#include "liboculus/SonarClient.h"
using namespace liboculus;

#include "serdp_common/OpenCVDisplay.h"
using namespace serdp_common;

#include "serdp_recorder/GpmfRecorder.h"
using namespace serdp_recorder;

using std::ofstream;
using std::ios_base;

using std::shared_ptr;
using std::unique_ptr;

int playbackSonarFile( const std::string &filename,
                        const std::shared_ptr<OpenCVDisplay> &display = std::shared_ptr<OpenCVDisplay>(nullptr),
                        const shared_ptr<Recorder> &recorder = shared_ptr<Recorder>(nullptr),
                        int count = -1 );


int main( int argc, char **argv ) {

  libg3logger::G3Logger logger("ocClient");

  CLI::App app{"Like oc_client but records in GPMF using serdp_recorder::GpfmRecorder"};

  int verbosity = 0;
  app.add_flag("-v", verbosity, "Additional output (use -vv for even more!)");

  string sonarIp("auto");
  app.add_option("--ip", sonarIp, "IP address of sonar or \"auto\" to automatically detect.");

  string outputFilename("");
  app.add_option("-o,--output", outputFilename, "Filename to save sonar data to.");

  string inputFilename("");
  app.add_option("-i,--input", inputFilename, "Filename to read sonar data from.");

  int stopAfter = -1;
  app.add_option("-c,--count", stopAfter, "");

  CLI11_PARSE(app, argc, argv);

  if( verbosity == 1 ) {
    logger.stderrHandle->call( &ColorStderrSink::setThreshold, INFO );
  } else if (verbosity > 1 ) {
    logger.stderrHandle->call( &ColorStderrSink::setThreshold, DEBUG );
  }

  shared_ptr<OpenCVDisplay> display;

  shared_ptr<GPMFRecorder> output( new GPMFRecorder );

  if( !outputFilename.empty() ) {
    output.reset( new GPMFRecorder( outputFilename ) );

    if( !output->isRecording() ) {
      LOG(WARNING) << "Unable to open " << outputFilename << " for output.";
      exit(-1);
    }
  }

  if( !inputFilename.empty() ) {
    return playbackSonarFile( inputFilename, display, output, stopAfter );
  }

  int count = 0;

  LOG(INFO) << "Enabling sonar";
  std::unique_ptr<SonarClient> sonar( new SonarClient( sonarIp ) );

  sonar->setDataRxCallback( [&]( const shared_ptr<SimplePingResult> &ping ) {
      // Do something
    auto valid = ping->valid();
    LOG(INFO) << "Got " << (valid ? "valid" : "invalid") << " ping";

    if( output ) output->addSonar( ping );

    count++;
    if( (stopAfter>0) && (count >= stopAfter)) sonar->stop();
  });

  sonar->start();

  sonar->join();
  exit(0);
}


int playbackSonarFile( const std::string &filename, const std::shared_ptr<OpenCVDisplay> &display,
                          const shared_ptr<Recorder> &recorder, int count ) {

  std::shared_ptr<SonarPlayerBase> player( SonarPlayerBase::OpenFile(filename) );

  if( !player ) {
    LOG(WARNING) << "Unable to open sonar file";
    return -1;
  }

  if( !player->open(filename) ) {
    LOG(INFO) << "Failed to open " << filename;
    return -1;
  }



  std::shared_ptr<SimplePingResult> ping( player->nextPing() );
  int numPings = 1;

  while( ping && (count >= 0 && numPings <= count) ) {
    if( ping->valid()) {
      //if( display ) display->showSonar( ping );
      if( recorder ) recorder->addSonar( ping );
    }


    cv::waitKey(1);

    ping = player->nextPing();
    numPings++;
  }

  return 0;
}
