#ifndef KUDZUKERNEL_PERIPHERAL_BQ34Z100G1_H
#define KUDZUKERNEL_PERIPHERAL_BQ34Z100G1_H
#include "Interfaces/I2CInterface.hpp"
#include "esp_system.h"

/**
 * The BQ34Z100-G1 device is an Impedance Track fuel gauge for Li-Ion,
 * PbA, NiMH, and NiCd batteries.
 */

/**
 * The device registers
 */
#define BQ34Z100G1_CTRL_CONTROL_STATUS      0x0000
#define BQ34Z100G1_CTRL_DEVICE_TYPE         0x0001
#define BQ34Z100G1_CTRL_FW_VERSION          0x0002
#define BQ34Z100G1_CTRL_FW_RESET            0x0041

#define BQ34Z100G1_REG16_CNTL               0x00  /**> Control */
#define BQ34Z100G1_REG8_SOC                 0x02  /**> StateOfCharge (%) */
#define BQ34Z100G1_REG8_ME                  0x03  /**> MaxError (%) */
#define BQ34Z100G1_REG16_RM                 0x04  /**> RemainingCapacity (mAh) */
#define BQ34Z100G1_REG16_FCC                0x06  /**> FullChargeCapacity (mAh) */
#define BQ34Z100G1_REG16_VOLT               0x08  /**> Voltage (mV) */
#define BQ34Z100G1_REG16_AI                 0x0A  /**> AverageCurrent (mA) */
#define BQ34Z100G1_REG16_TEMP               0x0C  /**> Temperature (0.1ºK) */
#define BQ34Z100G1_REG16_FLAGS              0x0E  /**> Flags */
#define BQ34Z100G1_REG16_I                  0x10  /**> Current (mA) */
#define BQ34Z100G1_REG16_FLAGSB             0x12  /**> FlagsB */
#define BQ34Z100G1_REG16_ATTE               0x18  /**> AverageTimeToEmpty (Minutes) */
#define BQ34Z100G1_REG16_ATTF               0x1A  /**> AverageTimeToFull (Minutes) */
#define BQ34Z100G1_REG16_PCHG               0x1C  /**> PassedCharge (mAh) */
#define BQ34Z100G1_REG16_DoD0T              0x1E  /**> DoD0Time (Minutes) */
#define BQ34Z100G1_REG16_AE                 0x24  /**> AvailableEnergy (10 mW/h) */
#define BQ34Z100G1_REG16_AP                 0x26  /**> AvailablePower (10 mW) */
#define BQ34Z100G1_REG16_SERNUM             0x28  /**> Serial Number */
#define BQ34Z100G1_REG16_INTTEMP            0x2A  /**> Internal_Temperature (0.1°K) */
#define BQ34Z100G1_REG16_CC                 0x2C  /**> CycleCount (Counts) */
#define BQ34Z100G1_REG16_SOH                0x2E  /**> StateOfHealth */
#define BQ34Z100G1_REG16_CHGV               0x30  /**> ChargeVoltage (mV) */
#define BQ34Z100G1_REG16_CHGI               0x32  /**> ChargeCurrent (mA) */
#define BQ34Z100G1_REG8_DFCLS               0x3E  /**> DataFlashClass */
#define BQ34Z100G1_REG8_DFBLK               0x3F  /**> DataFlashBlock */
#define BQ34Z100G1_REGx_DF                  0x40  /**> Authenticate / BlockData */
#define BQ34Z100G1_REG8_DFDCKS              0x60  /**> BlockDataCheckSum */
#define BQ34Z100G1_REG8_DFDCNTL             0x61  /**> BlockDataControl */

#define BQ34Z100G1_FLASH_CONFIG_SAFETY      0x02  /**> Configuration :: Safety */
#define BQ34Z100G1_FLASH_CONFIG_CIC         0x20  /**> Configuration :: Charge Inhibit Configuration */
#define BQ34Z100G1_FLASH_CONFIG_CHG         0x22  /**> Configuration :: Charge */
#define BQ34Z100G1_FLASH_CONFIG_CT          0x24  /**> Configuration :: Charge Termination */
#define BQ34Z100G1_FLASH_CONFIG_DATA        0x30  /**> Configuration :: Data */
#define BQ34Z100G1_FLASH_CONFIG_DCHG        0x31  /**> Configuration :: Discharge */
#define BQ34Z100G1_FLASH_CONFIG_MFR         0x38  /**> Configuration :: Manufacturer Data */
#define BQ34Z100G1_FLASH_CONFIG_REG         0x40  /**> Configuration :: Registers */
#define BQ34Z100G1_FLASH_CALIB_DATA         0x68  /**> Calibration :: Data */

/**
 * Calibration data page structure
 */
struct __attribute__((packed)) bq34z100g1_calib_data {
  float     cc_gain;
  float     cc_delta;
  int16_t   cc_offset;
  int8_t    board_offset;
  int8_t    int_temp_offset;
  int8_t    ext_temp_offset;
  uint8_t   _reserved;
  uint16_t  voltage_divider;
};

struct __attribute__((packed)) bq34z100g1_pack_config {
  uint8_t   rsn         : 2;
  uint8_t   iwake       : 1;
  uint8_t   voltsel     : 1;
  uint8_t   _rsvd0      : 1;
  uint8_t   scaled      : 1;
  uint8_t   cal_en      : 1;
  uint8_t   rescap      : 1;

  uint8_t   temps       : 1;
  uint8_t   gndsel      : 1;
  uint8_t   qpcclear    : 1;
  uint8_t   nidv        : 1;
  uint8_t   nidt        : 1;
  uint8_t   rmfcc       : 1;
  uint8_t   sleep       : 1;
  uint8_t   rfactstep   : 1;

  uint8_t   fconv_en    : 1;
  uint8_t   dodwt       : 1;
  uint8_t   lfprelax    : 1;
  uint8_t   jeiita      : 1;
  uint8_t   _rsvd2      : 1;
  uint8_t   vcons_en    : 1;
  uint8_t   _rsvd1      : 1;
  uint8_t   chg_dod_eoc : 1;

  uint8_t   smooth      : 1;
  uint8_t   relax_smooth_ok : 1;
  uint8_t   relax_jump_ok : 1;
  uint8_t   lock_0      : 1;
  uint8_t   sleep_wake_chg : 1;
  uint8_t   ff_near_edv : 1;
  uint8_t   rsoc_hold   : 1;
  uint8_t   soh_disp    : 1;
};

/**
 * Device configuration
 */
#define BQ34Z100G1_ADDR                  0x55

/**
 * Register read functions
 */
esp_err_t bq34z100g1_deviceType         (I2CInterface &iface, uint16_t *value);
esp_err_t bq34z100g1_deviceStatus       (I2CInterface &iface, uint16_t *value);

esp_err_t bq34z100g1_unseal             (I2CInterface &iface, uint32_t key);

esp_err_t bq34z100g1_stateOfCharge      (I2CInterface &iface, uint8_t *value);
esp_err_t bq34z100g1_remainingCapacity  (I2CInterface &iface, uint16_t *value);
esp_err_t bq34z100g1_voltage            (I2CInterface &iface, uint16_t *value);
esp_err_t bq34z100g1_averageCurrent     (I2CInterface &iface, uint16_t *value);
esp_err_t bq34z100g1_temperature        (I2CInterface &iface, uint16_t *value);
esp_err_t bq34z100g1_stateOfHealth      (I2CInterface &iface, uint16_t *value);
esp_err_t bq34z100g1_deviceName         (I2CInterface &iface, char* dst, size_t len);
esp_err_t bq34z100g1_reset              (I2CInterface &iface);

esp_err_t bq34z100g1_getCalibData       (I2CInterface &iface, bq34z100g1_calib_data *value);
esp_err_t bq34z100g1_setCalibData       (I2CInterface &iface, bq34z100g1_calib_data *value);

esp_err_t bq34z100g1_getPackConfig      (I2CInterface &iface, bq34z100g1_pack_config *value);
esp_err_t bq34z100g1_setPackConfig      (I2CInterface &iface, bq34z100g1_pack_config *value);

esp_err_t bq34z100g1_setVoltageDivider  (I2CInterface &iface, uint16_t value);

#endif
