#include "CayenneEncoder.hpp"
#include "Sherlock.hpp"

static const char * TAG = "enc.cayenne";

#define ERROR_OOM_STR "Ran out of cayenne buffer, putting measurement back"

/**
 * Default Constructor
 */
CayenneEncoder::CayenneEncoder()
  : MeasurementEncoder(), cayenne(NULL, 0) { }

/**
 * Returns the name of the encoder
 */
const char * CayenneEncoder::getEncoderName() {
  return "CayenneLPP";
}

/**
 * Starts an encoding fragment
 */
int CayenneEncoder::encoderFragmentStart(char * buffer, size_t buffer_size) {
  cayenne.setBuffer((uint8_t*)buffer, buffer_size);
  return 0;
}

/**
 * Start encoding a measurement in the buffer
 *
 * Cayenne cannot slit a value into multiple
 *
 * @returns the number of bytes written or <0 if there was an error and specifically -E_QUEUE_FULL if the data do not fit in the buffer
 */
int CayenneEncoder::encodeMeasurementStart(char * buffer, size_t buffer_size, Measurement & m) {
  // Get the first component of the measurement path
  const char * groupName = m.path[m.path.size() - 1].name;
  uint8_t written = 0;
  uint8_t ret;

  TRACE_LOGD(TAG, "Collecting measurement for %s", m.name());

  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // CayenneLPP is not a general purpose encoding protocol, which means that
  // we cannot simply translate the measured values into a byte stream (eg.
  // like JSON, BSON or MQTT). So we have to do a per-sensor de-composition
  // and re-composition of the values.
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  ///
  /// [GPS]
  ///
  if (strcmp("gps", groupName) == 0) {

    ret = cayenne.addGPS(
        1, // We only have one GPS
        m.getValue("lat").get<float>(),
        m.getValue("lng").get<float>(),
        m.getValue("alt").get<float>()
      );

    // If we ran out of buffer, put the measurement back in the array
    // and exit the collect loop.
    if (ret == 0) return -E_QUEUE_FULL;
    written += ret;

    // Add heading -if exists-
    if (m.hasValue("cog")) {
      ret = cayenne.addAnalogInput(
        21,
        m.getValue("cog").get<float>()
      );

      if (ret == 0) return -E_QUEUE_FULL;
      written += ret;
    }

    // Add speed -if exists-
    if (m.hasValue("spd")) {
      ret = cayenne.addAnalogInput(
        22,
        m.getValue("spd").get<float>()
      );

      if (ret == 0) return -E_QUEUE_FULL;
      written += ret;
    }

  ///
  /// [IMU]
  ///
  } else if (strcmp("imu", groupName) == 0) {

    ret = cayenne.addAccelerometer(
        1, // We only have one accelerometer
        m.getValue("ax").get<float>(),
        m.getValue("ay").get<float>(),
        m.getValue("az").get<float>()
      );

    if (ret == 0) return -E_QUEUE_FULL;
    written += ret;

    ret = cayenne.addGyrometer(
        1, // We only have one gyrometer
        m.getValue("gx").get<float>(),
        m.getValue("gy").get<float>(),
        m.getValue("gz").get<float>()
      );

    if (ret == 0) return -E_QUEUE_FULL;
    written += ret;

  ///
  /// [Fuel Gauge]
  ///
  } else if (strcmp("fg", groupName) == 0) {

    // The channel is stored as a second path component
    int chan = m.path[1].id;

    ret = cayenne.addFuelGauge(
        chan,
        m.getValue("soc").get<uint16_t>(),
        m.getValue("cap").get<uint16_t>(),
        m.getValue("voltage").get<uint16_t>(),
        m.getValue("current").get<uint16_t>(),
        m.getValue("temp").get<uint16_t>(),
        m.getValue("soh").get<uint16_t>()
      );

    // If we ran out of buffer, put the measurement back in the array
    // and exit the collect loop.
    if (ret == 0) return -E_QUEUE_FULL;
    written += ret;

  ///
  /// [Internal Battery]
  ///
  } else if (strcmp("ibat", groupName) == 0) {

    ret = cayenne.addAnalogInput(
      1,
      m.getValue("voltage").get<float>()
    );

    // If we ran out of buffer, put the measurement back in the array
    // and exit the collect loop.
    if (ret == 0) return -E_QUEUE_FULL;
    written += ret;

    ret = cayenne.addAnalogInput(
      2,
      m.getValue("soc").get<float>()
    );

    // If we ran out of buffer, put the measurement back in the array
    // and exit the collect loop.
    if (ret == 0) return -E_QUEUE_FULL;
    written += ret;

  } else {
    TRACE_LOGE(TAG, "Unknown sensor measurement: %s", m.name());
  }

  return written;
}
