#ifndef KUDZUKERNEL_MODULEI2CEXPANDER_H
#define KUDZUKERNEL_MODULEI2CEXPANDER_H
#include "Module.hpp"
#include "Pinout.hpp"
#include "Interfaces/I2CInterface.hpp"

#define CPPUTILS
// #define TEST

////////////////////////////////////////////////////////////////////////////////////////
enum ModuleI2CExpanderEvents {
  EVENT_I2CEXPANDER_READY,
};

/**
 * WiFi module is responsible for activating or de-activating the WiFi on the device
 */
class _ModuleI2CExpander: public Module, public I2CInterface {
public:

  /**
   * Constructor
   */
  _ModuleI2CExpander();

  /**
   * Return the module configuration
   */
  virtual const ModuleConfig& getModuleConfig();

  /**
   * @brief      Select the I2C expander channel to use
   *
   * @param[in]  channel  The channel to use (1~4) or 0 to disable all
   */
  void setChannel(uint8_t channel);

  /**
   * Write the given data to the I2C device at given address
   */
  virtual esp_err_t i2cWrite(uint8_t address, uint8_t * data, uint8_t len);

  /**
   * Read the given data to the I2C device from the given address
   */
  virtual esp_err_t i2cRead(uint8_t address, uint8_t * data, uint8_t len);

  /**
   * Perform a write/restart/read cycle to the I2C device given
   */
  virtual esp_err_t i2cWriteRead(uint8_t address, uint8_t * wr_data, uint8_t wr_len, uint8_t * rd_data, uint8_t rd_len);

  /**
   * Reset the I2C bus
   */
  virtual esp_err_t i2cReset();

protected:
  virtual void activate();
  virtual void deactivate();
private:
  uint8_t __v0001;
};

extern _ModuleI2CExpander ModuleI2CExpander;

#endif
