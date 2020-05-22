#ifndef KUDZUKERNEL_I2C_INTERFACE_H
#define KUDZUKERNEL_I2C_INTERFACE_H
#include "esp_system.h"

class I2CInterface {
public:

  /**
   * Write the given data to the I2C device at given address
   */
  virtual esp_err_t i2cWrite(uint8_t address, uint8_t * data, uint8_t len) = 0;

  /**
   * Read the given data to the I2C device from the given address
   */
  virtual esp_err_t i2cRead(uint8_t address, uint8_t * data, uint8_t len) = 0;

  /**
   * Perform a write/restart/read cycle to the I2C device given
   */
  virtual esp_err_t i2cWriteRead(uint8_t address, uint8_t * wr_data, uint8_t wr_len, uint8_t * rd_data, uint8_t rd_len) = 0;

  /**
   * Reset the I2C bus
   */
  virtual esp_err_t i2cReset() = 0;

  /**
   * Simple I2C scan
   */
  void i2cScan() {
    uint8_t address;

    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\r\n");
    for (int i = 0; i < 128; i += 16) {
        printf("%02x: ", i);
        for (int j = 0; j < 16; j++) {
            fflush(stdout);
            address = i + j;
            esp_err_t ret = i2cRead(address, NULL, 0);
            if (ret == ESP_OK) {
                printf("%02x ", address);
            } else if (ret == ESP_ERR_TIMEOUT) {
                printf("UU ");
            } else {
                printf("-- ");
            }
        }
        printf("\r\n");
    }
  }

};

#endif
