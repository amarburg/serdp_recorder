
#include <gtest/gtest.h>

#include <string>
#include <iostream>
using namespace std;

#include "serdp_recorder/VideoRecorder.h"
using serdprecorder::VideoRecorder;

TEST(TestVideoRecorder, makeFilename) {

  VideoRecorder recorder;
  recorder.setOutputDir( "/tmp/" );

  {
    auto p = recorder.makeFilename();
    cout << p << endl;
  }

  {
    auto p = recorder.makeFilename();
    cout << p << endl;
  }

}
