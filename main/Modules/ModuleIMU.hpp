#ifndef KUDZUKERNEL_ModuleIMU_H
#define KUDZUKERNEL_ModuleIMU_H
#include <Module.hpp>
#include "SPIbus.hpp"
#include "MPU.hpp"

/**
 * Forward declaration of the module singleton
 */
class _ModuleIMU;
extern _ModuleIMU ModuleIMU;

///////////////////////////////////////////
// Declaration of events that can be used
///////////////////////////////////////////

/**
 * An enum that defines the events your modules exchanges
 */
enum ModuleIMUEvents {
  EVENT_IMU_CHECK_STARTUP,
  EVENT_IMU_RETRY_CONNECTION,
  EVENT_IMU_INITIALIZE,
  EVENT_IMU_READOUT,
};

///////////////////////////////////////////
// Declaration of the module
///////////////////////////////////////////

/**
 * WiFi module is responsible for activating or de-activating the WiFi on the device
 */
class _ModuleIMU: public Module {
public:

  _ModuleIMU();

  /**
   * Return the module configuration
   */
  virtual const ModuleConfig& getModuleConfig();

protected:

  ///////////////////////////////
  // Module life-cycle methods
  ///////////////////////////////

  /**
   * (Optional) Implement this method to initialize your module
   */
  virtual void setup();

  ///////////////////////////////
  // Event handlers
  ///////////////////////////////

  /**
   * Declare one or more event handlers
   *
   * These event handlers can either listen for events from this module
   */
  DECLARE_EVENT_HANDLER(all_events);
  DECLARE_EVENT_HANDLER(sensorhub_events);

  /**
   * (Optional) Implement this method to handle activation of your module
   */
  virtual void activate();

  /**
   * (Optional) Implement this method to handle de-activation of your module
   */
  virtual void deactivate();


private:
  spi_device_handle_t   mpu_spi_handle;
  MPU_t                 MPU;

};


#endif
