#ifndef YACHTSENSE_JSON_ENCODER
#define YACHTSENSE_JSON_ENCODER

#include "Interfaces/MeasurementEncoder.hpp"

/**
 * MeasurementEncoder implementation that sends the collected measurements
 * encoded as JSON
 */
class JSONEncoder: public MeasurementEncoder {
public:

  /**
   * Default Constructor
   */
  JSONEncoder();

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
   * Ends an encoding fragment
   *
   * @returns the number of bytes written or <0 if there was an error and specifically -E_QUEUE_FULL if the data do not fit in the buffer
   */
  virtual int encoderFragmentEnd(char * buffer, size_t buffer_size, const char * fragment, size_t fragment_size);

  /**
   * Encode a measurement value in the buffer
   *
   * This function can be used by the encoder to actually encode the value in
   * the encoder buffer.
   *
   * @returns the number of bytes written or <0 if there was an error and specifically -E_QUEUE_FULL if the data do not fit in the buffer
   */
  virtual int encodeMeasurementValue(char * buffer, size_t buffer_size, Measurement & measurement, const char * name, Variant * value);

private:

  bool firstValue;

};

#endif
