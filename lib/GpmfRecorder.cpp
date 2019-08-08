
#include <ctime>
#include <memory>

#include "libg3logger/g3logger.h"

#include "serdp_recorder/VideoRecorder.h"

#include "gpmf-write/GPMF_writer.h"

using namespace libvideoencoder;

namespace serdprecorder {

  using namespace std;

  #define GPMF_DEVICE_ID_OCULUS_SONAR  0xAA000001
  static char SonarName[] = "OculusSonar";

  //=================================================================

  const size_t BufferSize = 10 * 1024 * 1024;

  GPMFRecorder::GPMFRecorder( )
    : Recorder(),
      _scratch( new uint32_t[BufferSize] ),
      _out(),
      _gpmfHandle( GPMFWriteServiceInit() ),
      _sonarHandle( GPMFWriteStreamOpen(_gpmfHandle, GPMF_CHANNEL_TIMED, GPMF_DEVICE_ID_OCULUS_SONAR, SonarName, NULL, BufferSize) )
    {
      CHECK( _gpmfHandle ) << "Unable to initialize GPMF Write Service";
      CHECK( _sonarHandle ) << "Unable to initialize GPMF stream for sonar";

      initGPMF();
    }

  GPMFRecorder::~GPMFRecorder()
  {
    GPMFWriteStreamClose(_sonarHandle);
    GPMFWriteServiceClose(_gpmfHandle);
  }

  bool GPMFRecorder::open( const std::string &filename ) {

    _out.open( filename, ios_base::binary | ios_base::out );

    if( !_out.is_open() ) return false;

    flushGPMF();

    return true;
  }

  void GPMFRecorder::close() {
    _out.close();
  }

  void GPMFRecorder::initGPMF()
  {
    GPMFWriteSetScratchBuffer( _gpmfHandle, _scratch.get(), BufferSize );

    const char name[]="Oculus MB1200d";
    GPMFWriteStreamStore(_sonarHandle, GPMF_KEY_STREAM_NAME, GPMF_TYPE_STRING_ASCII,
                          strlen(name), 1, (void *)name, GPMF_FLAGS_STICKY);
  }

  void GPMFRecorder::flushGPMF()
  {
    // Flush any stale data before starting video capture.
    uint32_t *payload, payload_size;
    GPMFWriteGetPayload(_gpmfHandle, GPMF_CHANNEL_TIMED, _scratch.get(), BufferSize, &payload, &payload_size);
  }

  size_t GPMFRecorder::writeSonar( const std::shared_ptr<liboculus::SimplePingResult> &ping, uint32_t **buffer, size_t bufferSize )
  {
    const uint64_t timestamp = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    {
      // Mark as big endian so it doesn't try to byte-swap the data.
      LOG(INFO) << "Writing " << (ping->buffer()->size() >> 2) << " dwords of sonar";
      auto err = GPMFWriteStreamStoreStamped(_sonarHandle, STR2FOURCC("OCUS"), GPMF_TYPE_UNSIGNED_LONG,
                                          4, (ping->buffer()->size() >> 2), ping->buffer()->ptr(),
                                          GPMF_FLAGS_BIG_ENDIAN | GPMF_FLAGS_STORE_ALL_TIMESTAMPS, timestamp);
      if( err != GPMF_ERROR_OK ) {
        LOG(WARNING) << "Error writing to GPMF store";
        return 0;
      }
    }

    // Estimate buffer size
    size_t estSize = GPMFWriteEstimateBufferSize( _gpmfHandle, GPMF_CHANNEL_TIMED, 0 ) + 8192;

    if( estSize > bufferSize ) {
      *buffer = (uint32_t *)av_malloc( estSize );
      bufferSize = estSize;
    }

    uint32_t *payload, payloadSize;
    GPMFWriteGetPayload(_gpmfHandle, GPMF_CHANNEL_TIMED, *buffer, bufferSize, &payload, &payloadSize);

    CHECK( payload == *buffer );

    LOG(DEBUG) << "Estimated GPMF size (in bytes) " << estSize << " ; actual payload size (in bytes) " << payloadSize;

    return payloadSize;
  }


  bool GPMFRecorder::addSonar( const std::shared_ptr<liboculus::SimplePingResult> &ping ) {
    if( !_out.is_open() ) return false;

    uint32_t *buffer = nullptr;
    auto bufferSize = writeSonar( ping, &buffer, 0 );
    if( bufferSize == 0 ) return false;

    _out.write( (const char *)buffer, bufferSize );

    if( buffer != nullptr ) av_free(buffer);

    return true;
  }


}
