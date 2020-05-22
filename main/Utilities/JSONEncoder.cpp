#include "JSONEncoder.hpp"
#include "Sherlock.hpp"

static const char * TAG = "enc.json";

#define RAN_OUT_OF_MEM "Ran out of JSON buffer, putting measurement back"

/**
 * Default Constructor
 */
JSONEncoder::JSONEncoder()
  : MeasurementEncoder(), firstValue(true) { }

/**
 * Returns the name of the encoder
 */
const char * JSONEncoder::getEncoderName() {
  return "JSON String";
}

/**
 * Starts an encoding fragment
 *
 * @returns the number of bytes written or <0 if there was an error and specifically -E_QUEUE_FULL if the data do not fit in the buffer
 */
int JSONEncoder::encoderFragmentStart(char * buffer, size_t buffer_size) {
  buffer[0] = '{';
  firstValue = true;
  return 1;
}

/**
 * Ends an encoding fragment
 *
 * @returns the number of bytes written or <0 if there was an error and specifically -E_QUEUE_FULL if the data do not fit in the buffer
 */
int JSONEncoder::encoderFragmentEnd(char * buffer, size_t buffer_size, const char * fragment, size_t fragment_size) {
  buffer[0] = '}';
  return 1;
}

/**
 * Encode a measurement value in the buffer
 *
 * This function can be used by the encoder to actually encode the value in
 * the encoder buffer.
 *
 * @returns the number of bytes written or <0 if there was an error and specifically -E_QUEUE_FULL if the data do not fit in the buffer
 */
int JSONEncoder::encodeMeasurementValue(char * buffer, size_t buffer_size, Measurement & m, const char * name, Variant * vref) {
  const char * prefix = m.name();
  const char * vstr = vref->get();
  const char * comma = "";

  // Check if we have enough buffer to fit the measurement expression
  size_t chunk_sz = 1 + strlen(prefix) + 1 + strlen(name) + 2 +  // "name/value":
                        strlen(vstr) + (vref->isNumeric() ? 0 : 2); // "value" or value
  if (!firstValue) chunk_sz += 1; // the comma before the key/value

  TRACE_LOGD(TAG, "Adding %s/%s (%s)", prefix, name, vstr);

  // Check if this is going to overrun the buffer (note: Account tailing '}')
  if (chunk_sz + 1 > buffer_size) return -E_QUEUE_FULL;

  // Spacer
  if (firstValue) {
    firstValue = false;
  } else {
    comma = ",";
  }

  // Encode
  if (vref->isNumeric()) {
    snprintf(buffer, buffer_size, "%s\"%s/%s\":%s", comma, prefix, name, vstr);
  } else {
    snprintf(buffer, buffer_size, "%s\"%s/%s\":\"%s\"", comma, prefix, name, vstr);
  }

  return chunk_sz;
}
