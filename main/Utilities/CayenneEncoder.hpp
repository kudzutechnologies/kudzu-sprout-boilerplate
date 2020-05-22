#ifndef YACHTSENSE_CAYENNE_ENCODER
#define YACHTSENSE_CAYENNE_ENCODER

#include "Interfaces/MeasurementEncoder.hpp"
#include "CayenneLPP.hpp"

/**
 * MeasurementEncoder implementation that sends the collected measurements
 * encoded with the Cayenne LPP protocol
 */
class CayenneEncoder: public MeasurementEncoder {
public:

  /**
   * Default Constructor
   */
  CayenneEncoder();

  /**
   * Returns the name of the encoder
   */
  virtual const char * getEncoderName();

  /**
   * Starts an encoding fragment
   *
   * @returns the number of bytes written or <0 if there was an error and specifically -E_QUEUE_FULL if the data do not fit in the buffer
   */
  virtual int encoderFragmentStart(char * buffer, size_t buffer_size);

  /**
   * Start encoding a measurement in the buffer
   *
   * This function can be used by the encoder to append  group information in
   * the encoder buffer.
   *
   * @returns the number of bytes written or <0 if there was an error and specifically -E_QUEUE_FULL if the data do not fit in the buffer
   */
  virtual int encodeMeasurementStart(char * buffer, size_t buffer_size, Measurement & measurement);

private:

  /**
   * The cayenne encoder library
   */
  CayenneLPP cayenne;

};

#endif
