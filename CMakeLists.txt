project( serdp_recorder )
cmake_minimum_required(VERSION 3.5)
if( NOT FIPS_IMPORT )
	get_filename_component(FIPS_ROOT_DIR "../fips" ABSOLUTE)
	include("${FIPS_ROOT_DIR}/cmake/fips.cmake")

	fips_setup(PROJECT serdp_recorder)
endif()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# find_package(Eigen3 REQUIRED )
find_package(Boost COMPONENTS filesystem system REQUIRED )  #n.b. program_options is only required for my version of Stereolabs "Recorder"

## Default OpenCV version
list(APPEND OPENCV_COMPONENTS core highgui imgproc )
find_package(OpenCV COMPONENTS ${OPENCV_COMPONENTS} REQUIRED)

## C++ flags applied to all builds
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

## This is required on OSX?
IF(APPLE)
    # Fix linking on 10.14+. See https://stackoverflow.com/questions/54068035
    LINK_DIRECTORIES(/usr/local/lib)
		fips_include_directories( /usr/local/include )
ENDIF()


## Find Blackmagic SDK
set( BLACKMAGIC_FORCE_LINUX TRUE )
find_package( BlackmagicSDK REQUIRED )

## Set global include paths
fips_include_directories(
  ${CMAKE_SOURCE_DIR}/include
  ${BLACKMAGIC_INCLUDE_DIR}
  ${OpenCV_INCLUDE_DIRS}
 )

# The local library
fips_begin_module( serdp_recorder )

   fips_src( lib )

   fips_deps( videoencoder blackmagic oculus gpmf activeobject serdp_common )

   fips_include_directories(
 	   ${CMAKE_SOURCE_DIR}/include
 	 )

	 fips_libs(  ${Boost_LIBRARIES} )

fips_end_module()


if( NOT FIPS_IMPORT )
  add_subdirectory("tools/")

	if( FIPS_UNITTESTS )
			 gtest_begin( serdp_recorder )
			     fips_src( test/unit/ )
			     fips_deps( serdp_recorder )
			gtest_end()
	endif()

  fips_finish()
endif()
