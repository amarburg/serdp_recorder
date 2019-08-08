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

#include "serdp_recorder/SonarClient.h"

#include "gpmf-write/GPMF_writer.h"


using namespace liboculus;
using namespace serdprecorder;

using std::ofstream;
using std::ios_base;

using std::shared_ptr;
using std::unique_ptr;

int playbackSonarFile( const std::string &filename, const std::shared_ptr<OpenCVDisplay> &display = std::shared_ptr<OpenCVDisplay>(nullptr) );


int main( int argc, char **argv ) {

  libg3logger::G3Logger logger("ocClient");

  CLI::App app{"Simple Oculus Sonar app"};

  int verbosity = 0;
  app.add_flag("-v", verbosity, "Additional output (use -vv for even more!)");

  string sonarIp("auto");
  app.add_option("--ip", sonarIp, "IP address of sonar or \"auto\" to automatically detect.");

  string outputFilename("");
  app.add_option("-o,--output", outputFilename, "Filename to save sonar data to.");

  string inputFilename("");
  app.add_option("-i,--input", inputFilename, "Filename to read sonar data from.");

  int count = -1;
  app.add_option("-c,--count", count, "");

  CLI11_PARSE(app, argc, argv);

  if( verbosity == 1 ) {
    logger.stderrHandle->call( &ColorStderrSink::setThreshold, INFO );
  } else if (verbosity > 1 ) {
    logger.stderrHandle->call( &ColorStderrSink::setThreshold, DEBUG );
  }

  shared_ptr<OpenCVDisplay> display;

  if( !inputFilename.empty() ) {
    return playbackSonarFile( inputFilename, display );
  }

  shared_ptr<GPMFRecorder> output( new GPMFRecorder );

  if( !outputFilename.empty() ) {
    output->open( outputFilename );

    if( !output->isRecording() ) {
      LOG(WARNING) << "Unable to open " << outputFilename << " for output.";
      exit(-1);
    }
  }

  LOG(INFO) << "Enabling sonar";
  std::unique_ptr<SonarClient> sonar( new SonarClient( sonarIp, output, display ) );
  sonar->start();

  while(true) {
    if( (count > 0) && (sonar->pingCount() >= count ) ) {
      LOG(INFO) << "Collected " << count << " pings, quitting...";
      break;
    }

    usleep(100000);
  }

  if( output->isRecording() ) output->close();

  sonar->stop();
  exit(0);
}


int playbackSonarFile( const std::string &filename, const std::shared_ptr<OpenCVDisplay> &display ) {
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
  while( ping ) {
    if( ping->valid()) {
      if( display ) display->showSonar( ping );
    }

    cv::waitKey(1000);

    ping = player->nextPing();
  }

  return 0;
}
