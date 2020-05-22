#ifndef KUDZUKERNEL_MEASUREMENT_ENCODER
#define KUDZUKERNEL_MEASUREMENT_ENCODER
#include "Utilities/Measurement.hpp"
#include "Utilities/StaticContainers.hpp"

/**
 * A helper class used by the sensor hub to encode the measurement array
 * into a byte stream to be sent to the remote end.
 *
 * Since there is a strict limitation on the number of bytes that can be
 * encoded in a single message, the transmission might be broken down into
 * multiple individual segments.
 *
 * The API of the encoder accommodates such cases, while maintaining the simple,
 * measurement-based interface for the encoded values.
 *
 * A typical transmission looks like this:
 *
 * 1) [encoderInit] - Called once per transmission bulk
 *     The encoder should initialize it's local state and prepare for a
 *     transmission. It can optionally put some heading bytes in the
 *     transmission buffer. It can also keep a copy of the given buffer pointer
 *     since it's going to remain the same throughout the tx process.
 *
 * 2) [encoderChunkStart] - Called when a chunk is started
 *     The encoder should append any heading information to the buffer,
 *     indicating the beginning of a chunk.
 *
 * 3) [encodeMeasurementStart]
 *
 */
class MeasurementEncoder {
public:

  /**
   * Returns the name of the encoder
   */
  virtual const char * getEncoderName() = 0;

  /**
   * Reset any local state related to the encoding
   *
   * This function is called before the first `encodeSensorStart` function is
   * called, and
   */
  virtual void encoderReset();

  /**
   * Initialize the encoding buffer
   *
   * This is the very first function called, when the first measurement is about
   * to be encoded to the destination buffer.
   *
   * @returns the number of bytes written or <0 if there was an error and specifically -E_QUEUE_FULL if the data do not fit in the buffer
   */
  virtual int encoderInit(char * buffer, size_t buffer_size);

  /**
   * Starts an encoding fragment
   *
   * @returns the number of bytes written or <0 if there was an error and specifically -E_QUEUE_FULL if the data do not fit in the buffer
   */
  virtual int encoderFragmentStart(char * buffer, size_t buffer_size);

  /**
   * Ends an encoding fragment
   *
   * @returns the number of bytes written or <0 if there was an error and specifically -E_QUEUE_FULL if the data do not fit in the buffer
   */
  virtual int encoderFragmentEnd(char * buffer, size_t buffer_size, const char * fragment, size_t fragment_size);

  /**
   * Start encoding a measurement in the buffer
   *
   * This function can be used by the encoder to append  group information in
   * the encoder buffer.
   *
   * @returns the number of bytes written or <0 if there was an error and specifically -E_QUEUE_FULL if the data do not fit in the buffer
   */
  virtual int encodeMeasurementStart(char * buffer, size_t buffer_size, Measurement & measurement);

  /**
   * Encode a measurement value in the buffer
   *
   * This function can be used by the encoder to actually encode the value in
   * the encoder buffer.
   *
   * @returns the number of bytes written or <0 if there was an error and specifically -E_QUEUE_FULL if the data do not fit in the buffer
   */
  virtual int encodeMeasurementValue(char * buffer, size_t buffer_size, Measurement & measurement, const char * name, Variant * value);

  /**
   * Complete encoding a measurement in the buffer
   *
   * This function can be used by the encoder to complete the group information
   * in the encoder buffer.
   *
   * @returns the number of bytes written or <0 if there was an error and specifically -E_QUEUE_FULL if the data do not fit in the buffer
   */
  virtual int encodeMeasurementEnd(char * buffer, size_t buffer_size, Measurement & measurement, const char * chunk, size_t chun_size);

  /**
   * Perform final transformations to the buffer right before flushing
   *
   * @returns the new size of the buffer or <0 if there was an error
   */
  virtual int encoderFlush(char * buffer, size_t buffer_size, size_t used_size);

  /**
   * Finalize the encoder
   *
   * This function is called when the encoder is no longer used and it can release
   * any allocated resources.
   *
   * @returns the number of bytes written or <0 if there was an error and specifically -E_QUEUE_FULL if the data do not fit in the buffer
   */
  virtual void encoderFinalize();

};


#endif
