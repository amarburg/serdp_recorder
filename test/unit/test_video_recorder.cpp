
#include <gtest/gtest.h>

#include <string>
#include <iostream>
using namespace std;

#include "serdp_recorder/VideoRecorder.h"
using serdp_recorder::VideoRecorder;

TEST(TestVideoRecorder, makeFilename) {

  VideoRecorder recorder("/tmp/foo.mov", 640, 480, 30 );

}
