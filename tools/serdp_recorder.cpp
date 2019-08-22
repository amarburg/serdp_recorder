///
///  Thin wrapper around the SerdpRecorder class, which records multiple
///  Decklink cameras and optionally an Oculus sonar.
///

// TODO:   Reduce the DRY

#include <string>
using namespace std;

#include <signal.h>

bool keepGoing = true;

#include "libg3logger/g3logger.h"

#include "SerdpRecorder.h"

shared_ptr<serdp_recorder::SerdpRecorder> app;

void signal_handler( int sig )
{
	LOG(INFO) << "Signal handler: " << sig;

	switch( sig ) {
		case SIGINT:
				app->keepGoing( false );
				break;
		default:
				app->keepGoing( false );
				break;
	}
}

//const int CamNum = 1;


int main( int argc, char** argv )
{
	signal( SIGINT, signal_handler );
	app.reset( new serdp_recorder::SerdpRecorder() );

	return app->run( argc, argv );
}
