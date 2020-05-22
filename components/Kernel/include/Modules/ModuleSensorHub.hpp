#ifndef KUDZUKERNEL_ModuleSensorHub_H
#define KUDZUKERNEL_ModuleSensorHub_H
#include <deque>
#include <Module.hpp>
#include "Utilities/SensorConfig.hpp"
#include "Interfaces/MeasurementEncoder.hpp"

/**
 * The size of the egress chunk size. If the data accumulated are bigger than
 * this size, they will be flushed to the uplink module.
 *
 * A good rule of thumb is to set this to the smallest transmittable unit of
 * the uplink protocols. In our case the value 50 is used for LoRa.
 */
#define MODULE_SENSORHUB_EGRESS_CHUNK_SIZE   50

/**
 * Forward declaration of the module singleton
 */
class _ModuleSensorHub;
extern _ModuleSensorHub ModuleSensorHub;

///////////////////////////////////////////
// Declaration of events that can be used
///////////////////////////////////////////

/**
 * An enum that defines the events your modules exchanges
 */
enum ModuleSensorHubEvents {

  /**
   * Broadcasted when all sensors are requested to provide their samples
   */
  EVENT_SENSORHUB_SAMPLE,

  /**
   * Broadcasted when a measurement is collected
   */
  EVENT_SENSORHUB_MEASUREMENT,

  /**
   * Broadcasted when a requested flush operation has completed
   */
  EVENT_SENSORHUB_FLUSH_COMPLETED,


  EVNET_SENSORHUB_CONTINUE = 0x100

};

///////////////////////////////////////////
// Declaration of the module
///////////////////////////////////////////

/**
 * WiFi module is responsible for activating or de-activating the WiFi on the device
 */
class _ModuleSensorHub: public Module {
public:

  _ModuleSensorHub();

  /**
   * Return the module configuration
   */
  virtual const ModuleConfig& getModuleConfig();

  /**
   * Request all sensor to provide a sample
   */
  void sampleAllSensors();

  /**
   * Add a sensor measurement
   */
  int addMeasurement( const SensorDataGroup & grp,  std::vector<MeasurementValue> values, measurement_release_fn rfn = measurement_release_fn() );

  /**
   * Change the egress encoder
   */
  void addEncoder( MeasurementEncoder * encoder );

  /**
   * Erase all __v0006
   */
  void reset();

  /**
   * Flush the __v0006 to the sender
   */
  void flush();

  /**
   * Get all the collected __v0006
   */
  const std::deque<Measurement*> getMeasurements();

protected:
  virtual void setup();
  virtual void activate();
  virtual void deactivate();
  DECLARE_EVENT_HANDLER(all_events);
  DECLARE_EVENT_HANDLER(sender_events);
  virtual std::vector<ValueDefinition> configOptions();
  virtual void configDidSave();
  virtual void nvsReset(void* nvs);
private:
  void __v0001();
  void __v0004();
  bool __v000a;
  bool __v000b;
  StaticQueue<Measurement, 24> __v0006;
  int __v0005;
  std::vector<const char*> __v0007;
  std::vector<MeasurementEncoder *> __v0003;
  Measurement   __v0002;
  uint8_t       __v0009;
  uint8_t       __v0008;
};


#endif
