#ifndef KUDZUKERNEL_SENSOR_CONFIG
#define KUDZUKERNEL_SENSOR_CONFIG
#include <vector>
#include <stdint.h>
#include "Measurement.hpp"

/**
 * A data group
 */
class SensorDataGroup {
public:

  /**
   * Create a sub-group
   */
  SensorDataGroup group(const char * name);
  SensorDataGroup group(const char * name, const std::vector<const char*> tags);
  SensorDataGroup group(uint32_t id);
  SensorDataGroup group(uint32_t id, const std::vector<const char*> tags);

  /**
   * The group identifier
   */
  PathIdentifier id;

  /**
   * Tags for this group
   */
  std::vector<const char*> tags;

  /**
   * Parent class
   */
  SensorDataGroup * parent;

protected:

  /**
   Constructor
   */
  SensorDataGroup(PathIdentifier id, const std::vector<const char*> tags, SensorDataGroup * parent);

};

/**
 * A structure that carries the sensor configuration
 */
class SensorConfig: public SensorDataGroup {
public:

  /**
   * Construct a sensor configuration
   */
  SensorConfig(const char * name);
  SensorConfig(const char * name, const std::vector<const char*> tags);

  /**
   * The name of the sensor
   */
  const char * name;

  /**
   * Tags for this sensor
   */
  std::vector<const char*> tags;

};


#endif
