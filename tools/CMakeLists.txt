
fips_begin_app( serdprecorder cmdline )
  fips_files( serdp_recorder.cpp SerdpRecorder.cpp )

  fips_deps( blackmagic oculus videoencoder serdp_recorder )
  fips_libs( avutil avcodec swscale avformat )


  fips_include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
  )

fips_end_app()

fips_begin_app( oculus_gpmf_recorder cmdline )
  fips_files( oculus_gpmf_recorder.cpp )

  fips_deps( oculus serdp_recorder )

  fips_include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
  )

fips_end_app()
