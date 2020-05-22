#include <cstring>
#include <cmath>

#include "Peripherals/BQ34Z100G1.hpp"
#include "Sherlock.hpp"

static const char * TAG = "dev.BQ34Z100G1";

/**
 * Convert a float value into the byte representation the BQ34Z100G1 understands
 */
static void floatToBytes(float X, uint8_t * buf) {
  memset(buf, 0, 4);
  uint32_t mantissa;
  int exp;

  // Handle zero as special case
  if (X == 0) return;

  // Handle sign
  if (X < 0) {
    buf[1] |= 0x80;
    X = -X;
  }

  // Calculate exponent
  X = frexp( X, &exp );
  // Scale value to compute mantissa
  mantissa = X * pow(2, 24);
  // Compute byte values
  buf[0] = exp + 128;
  buf[1] |= (mantissa >> 16) & 0x7F;
  buf[2] |= (mantissa >> 8) & 0xFF;
  buf[3] |= mantissa & 0xFF;
}

/**
 * Convert a byte value from the BQ34Z100G1 into a float value we can handle
 */
static void bytesToFloat(const uint8_t *buf, float *X) {
  int sign = (buf[1] & 0x80) ? -1 : 1;
  int exp = buf[0] - 128 - 24;
  int mantissa = 0x800000 | ((int)buf[1] << 16) | ((int)buf[2] << 8) | (int)buf[3];

  *X = sign * pow(2, exp) * mantissa;
}

/**
 * Call-out (write 0-data) to a control register
 */
static esp_err_t _call_control(I2CInterface &iface, uint16_t reg) {
  uint8_t buf[3];
  TRACE_LOGD(TAG, "Calling control register 0x%04x", reg);

  // Select the control register to use
  buf[0] = BQ34Z100G1_REG16_CNTL;
  buf[1] = reg & 0xFF;
  buf[2] = (reg >> 8) & 0xFF;

  return iface.i2cWrite(
    BQ34Z100G1_ADDR,
    &buf[0],
    3
  );
}

/**
 * Read a control register
 */
static esp_err_t _read_control(I2CInterface &iface, uint16_t reg, uint16_t *value) {
  uint8_t buf[3];
  esp_err_t err;
  TRACE_LOGD(TAG, "Reading control register 0x%04x", reg);

  // Select the control register to use
  buf[0] = BQ34Z100G1_REG16_CNTL;
  buf[1] = reg & 0xFF;
  buf[2] = (reg >> 8) & 0xFF;

  err = iface.i2cWrite(
    BQ34Z100G1_ADDR,
    &buf[0],
    3
  );
  if (err != ESP_OK) return err;

  // Read from the same address, the value of the register
  buf[0] = BQ34Z100G1_REG16_CNTL;

  err = iface.i2cWriteRead(
    BQ34Z100G1_ADDR,
    &buf[0],
    1,
    &buf[1],
    2
  );
  if (err != ESP_OK) return err;

  *value = (((uint16_t)buf[2]) << 8) | buf[1];
  TRACE_LOGD(TAG, "Read = 0x%04x", *value);
  return ESP_OK;
}

/**
 * Read a 16-bit unsigned number from the given address
 */
static esp_err_t _read_mem16(I2CInterface &iface, uint8_t addr, uint16_t *value) {
  uint8_t buf[3];
  esp_err_t err;
  TRACE_LOGD(TAG, "Reading address 0x%02x", addr);

  // Select the address to read
  buf[0] = addr;

  err = iface.i2cWriteRead(
    BQ34Z100G1_ADDR,
    &buf[0],
    1,
    &buf[1],
    2
  );
  if (err != ESP_OK) return err;

  *value = (((uint16_t)buf[2]) << 8) | buf[1];
  TRACE_LOGD(TAG, "Read = 0x%04x", *value);
  return ESP_OK;
}

/**
 * Read a 8-bit unsigned number from the given address
 */
static esp_err_t _read_mem8(I2CInterface &iface, uint8_t addr, uint8_t *value) {
  uint8_t buf[2];
  esp_err_t err;
  TRACE_LOGD(TAG, "Reading address 0x%02x", addr);

  // Select the address to read
  buf[0] = addr;

  err = iface.i2cWriteRead(
    BQ34Z100G1_ADDR,
    &buf[0],
    1,
    &buf[1],
    1
  );
  if (err != ESP_OK) return err;

  *value = buf[1];
  TRACE_LOGD(TAG, "Read = 0x%02x", *value);
  return ESP_OK;
}

/**
 * Read arbitrary long segment of flash memory in the given pointer
 */
static esp_err_t _read_flash(I2CInterface &iface, uint8_t subclass, uint16_t offset, uint8_t *dst, size_t size) {
  esp_err_t err;
  uint8_t buf[4];
  uint8_t sz;
  TRACE_LOGD(TAG, "Reading flash subclass=0x%02x, offset=%d, len=%d", subclass, offset, size);

  // Enable block flash control
  buf[0] = BQ34Z100G1_REG8_DFDCNTL;
  buf[1] = 0x00;
  err = iface.i2cWrite(
    BQ34Z100G1_ADDR,
    &buf[0],
    2
  );
  if (err != ESP_OK) return err;

  // Configure the subclass
  buf[0] = BQ34Z100G1_REG8_DFCLS;
  buf[1] = subclass;
  err = iface.i2cWrite(
    BQ34Z100G1_ADDR,
    &buf[0],
    2
  );
  if (err != ESP_OK) return err;

  // The flass is accessed in 32-byte blocks.
  // Find the first and the last block requested
  uint16_t firstBlock = offset / 32;
  uint16_t lastBlock = (offset + size) / 32;
  uint16_t b = 0;
  for (uint16_t block=firstBlock; block<=lastBlock; block++) {
    TRACE_LOGD(TAG, "Selecting block %d", block);

    // Select the block we want to read
    buf[0] = BQ34Z100G1_REG8_DFBLK;
    buf[1] = block & 0xFF;
    err = iface.i2cWrite(
      BQ34Z100G1_ADDR,
      &buf[0],
      2
    );
    if (err != ESP_OK) return err;

    uint8_t startOffset = offset % 32;
    if (block > firstBlock) startOffset = 0;

    uint8_t endOffset = (offset + size) % 32;
    if (block < lastBlock) endOffset = 32;

    TRACE_LOGD(TAG, "Reading {%d - %d} -> @%d", startOffset, endOffset, b);

    sz = endOffset - startOffset;
    if (!sz) break;

    // Read bytes from the block address
    buf[0] = BQ34Z100G1_REGx_DF + startOffset;
    err = iface.i2cWriteRead(
      BQ34Z100G1_ADDR,
      &buf[0],
      1,
      &dst[b],
      sz
    );
    if (err != ESP_OK) return err;

    // Advance destination buffer pointer
    b += sz;
  }

  TRACE_LOGD(TAG, "Read %d bytes", size);
  ESP_LOG_BUFFER_HEXDUMP(TAG, dst, size, ESP_LOG_DEBUG);

  return ESP_OK;
}

/**
 * Write arbitrary long segment of flash memory in the given pointer
 */
static esp_err_t _write_flash(I2CInterface &iface, uint8_t subclass, uint16_t offset, uint8_t *src, size_t size) {
  esp_err_t err;
  uint8_t buf[34];
  uint8_t sz;
  TRACE_LOGD(TAG, "Writing flash subclass=0x%02x, offset=%d, len=%d", subclass, offset, size);
  ESP_LOG_BUFFER_HEXDUMP(TAG, src, size, ESP_LOG_DEBUG);

  // Enable block flash control
  buf[0] = BQ34Z100G1_REG8_DFDCNTL;
  buf[1] = 0x00;
  err = iface.i2cWrite(
    BQ34Z100G1_ADDR,
    &buf[0],
    2
  );
  if (err != ESP_OK) return err;

  // Configure the subclass
  buf[0] = BQ34Z100G1_REG8_DFCLS;
  buf[1] = subclass;

  err = iface.i2cWrite(
    BQ34Z100G1_ADDR,
    &buf[0],
    2
  );
  if (err != ESP_OK) return err;

  // The flass is accessed in 32-byte blocks.
  // Find the first and the last block requested
  uint16_t firstBlock = offset / 32;
  uint16_t lastBlock = (offset + size) / 32;
  uint16_t b = 0;
  uint8_t csum = 0;
  for (uint16_t block=firstBlock; block<=lastBlock; block++) {
    TRACE_LOGD(TAG, "Selecting block %d", block);

    // Select the block we want to read
    buf[0] = BQ34Z100G1_REG8_DFBLK;
    buf[1] = block & 0xFF;
    err = iface.i2cWrite(
      BQ34Z100G1_ADDR,
      &buf[0],
      2
    );
    if (err != ESP_OK) return err;

    uint8_t startOffset = offset % 32;
    if (block > firstBlock) startOffset = 0;

    uint8_t endOffset = (offset + size) % 32;
    if (block < lastBlock) endOffset = 32;

    TRACE_LOGD(TAG, "Writing {%d - %d} <- @%d", startOffset, endOffset, b);

    sz = endOffset - startOffset;
    if (!sz) break;

    // If we are overwriting the entire flash block,
    // compute the checksum based on the data given
    if (sz == 32) {
      TRACE_LOGD(TAG, "This is a full-page write");

      csum = 0;
      for (uint8_t i=0; i<32; ++i) {
        csum += src[b + i];
      }
      csum = 0xFF - csum;

      TRACE_LOGD(TAG, "Computed csum=%02X", csum);
      ESP_LOG_BUFFER_HEXDUMP(TAG, &src[b], 32, ESP_LOG_DEBUG);
    }

    // Otherwise, we have to read the previous page and
    // compute the new checksum
    else {
      uint8_t page[32];
      TRACE_LOGD(TAG, "This is a partial-page write");

      // Read the entire page
      buf[0] = BQ34Z100G1_REGx_DF;
      err = iface.i2cWriteRead(
        BQ34Z100G1_ADDR,
        &buf[0],
        1,
        &page[0],
        32
      );
      TRACE_LOGD(TAG, "ret=%d", err);
      if (err != ESP_OK) return err;

      TRACE_LOGD(TAG, "Read page");
      ESP_LOG_BUFFER_HEXDUMP(TAG, page, 32, ESP_LOG_DEBUG);

      // Compute the new csum, using the page bytes to
      // fill-in the gaps.
      csum = 0;
      for (uint8_t i=0; i<32; ++i) {
        if ((i>=startOffset) && (i<endOffset)) {
          TRACE_LOGD(TAG, "++ %02x <- src[%d]", src[i - startOffset + b], i - startOffset + b);
          csum += src[i - startOffset + b];
        } else {
          TRACE_LOGD(TAG, "++ %02x <- page[%d]", page[i], i);
          csum += page[i];
        }
      }
      csum = 0xFF - csum;

      TRACE_LOGD(TAG, "Computed csum=%02X", csum);
      ESP_LOG_BUFFER_HEXDUMP(TAG, &src[b], sz, ESP_LOG_DEBUG);
    }

    // Write bytes to the block address
    buf[0] = BQ34Z100G1_REGx_DF + startOffset;
    memcpy(&buf[1], &src[b], sz);

    TRACE_LOGD(TAG, "Writing %d bytes", sz);
    ESP_LOG_BUFFER_HEXDUMP(TAG, &buf[1], sz, ESP_LOG_DEBUG);
    err = iface.i2cWrite(
      BQ34Z100G1_ADDR,
      &buf[0],
      1 + sz
    );
    TRACE_LOGD(TAG, "ret=%d", err);
    if (err != ESP_OK) return err;

    // The data will be written when the correct checksum is sent
    buf[0] = BQ34Z100G1_REG8_DFDCKS;
    buf[1] = csum;

    TRACE_LOGD(TAG, "Writing checksum %02x", csum);
    err = iface.i2cWrite(
      BQ34Z100G1_ADDR,
      &buf[0],
      2
    );
    TRACE_LOGD(TAG, "ret=%d", err);
    if (err != ESP_OK) return err;

    // Upon successful write, BQ34Z100G1 needs some time to update flash
    vTaskDelay(150 / portTICK_PERIOD_MS);

    // Advance destination buffer pointer
    b += sz;
  }

  return ESP_OK;
}

/**
 * @brief      Compute the checksum of a flash page
 *
 * @param      data  The contents of the flash page
 * @param[in]  len   The size of the page (multiples of 32)
 *
 * @return     Returns the checksum value
 */
static uint8_t _block_csum(uint8_t* data, uint8_t len) {
  uint8_t sum = 0;
  for (uint8_t i=0; i<len; ++i) {
    sum += data[i];
  }
  return 0xFF - sum;
}

esp_err_t bq34z100g1_unseal(I2CInterface &iface, uint32_t key) {
  esp_err_t err;
  uint16_t ctrl;
  TRACE_LOGD(TAG, "Unsealing device with key %02x", key);

  err = _read_control(iface, BQ34Z100G1_CTRL_CONTROL_STATUS, &ctrl);
  if (err != ESP_OK) return err;

  // Check if the device is not sealed
  if (!(ctrl & 0x2000)) {
    TRACE_LOGD(TAG, "Device not sealed");
    return ESP_OK;
  }

  // Send the keys
  uint16_t key_hi = (key >> 16) & 0xFFFF;
  uint16_t key_lo = key & 0xFFFF;

  TRACE_LOGD(TAG, "Sending first part %04x", key_lo);
  err = _call_control(iface, key_lo);
  if (err != ESP_OK) return err;
  TRACE_LOGD(TAG, "Sending second part %04x", key_hi);
  err = _call_control(iface, key_hi);
  if (err != ESP_OK) return err;

  // Read the control register, it must be unsealed now
  err = _read_control(iface, BQ34Z100G1_CTRL_CONTROL_STATUS, &ctrl);
  if (err != ESP_OK) return err;
  if (!(ctrl & 0x2000)) {
    TRACE_LOGD(TAG, "Device unsealed");
    return ESP_OK;
  }

  TRACE_LOGW(TAG, "Unable to unseal device");
  return ESP_FAIL;
}

esp_err_t bq34z100g1_deviceType(I2CInterface &iface, uint16_t *value) {
  return _read_control(iface, BQ34Z100G1_CTRL_DEVICE_TYPE, value);
}

esp_err_t bq34z100g1_deviceStatus(I2CInterface &iface, uint16_t *value) {
  return _read_control(iface, BQ34Z100G1_CTRL_CONTROL_STATUS, value);
}

esp_err_t bq34z100g1_stateOfCharge(I2CInterface &iface, uint8_t *value) {
  return _read_mem8(iface, BQ34Z100G1_REG8_SOC, value);
}

esp_err_t bq34z100g1_remainingCapacity(I2CInterface &iface, uint16_t *value) {
  return _read_mem16(iface, BQ34Z100G1_REG16_RM, value);
}

esp_err_t bq34z100g1_voltage(I2CInterface &iface, uint16_t *value) {
  return _read_mem16(iface, BQ34Z100G1_REG16_VOLT, value);
}

esp_err_t bq34z100g1_averageCurrent(I2CInterface &iface, uint16_t *value) {
  return _read_mem16(iface, BQ34Z100G1_REG16_AI, value);
}

esp_err_t bq34z100g1_temperature(I2CInterface &iface, uint16_t *value) {
  return _read_mem16(iface, BQ34Z100G1_REG16_TEMP, value);
}

esp_err_t bq34z100g1_stateOfHealth(I2CInterface &iface, uint16_t *value) {
  return _read_mem16(iface, BQ34Z100G1_REG16_SOH, value);
}

esp_err_t bq34z100g1_deviceName(I2CInterface &iface, char* dst, size_t len) {
  if (len > 12) len = 12;

  return _read_flash(
    iface,
    BQ34Z100G1_FLASH_CONFIG_DATA,
    32,
    (uint8_t*)dst,
    len
  );
}

esp_err_t bq34z100g1_reset(I2CInterface &iface) {
  esp_err_t err = _call_control(iface, BQ34Z100G1_CTRL_FW_RESET);
  if (err != ESP_OK) return err;

  vTaskDelay(150 / portTICK_PERIOD_MS);
  return ESP_OK;
}

esp_err_t bq34z100g1_getCalibData(I2CInterface &iface, bq34z100g1_calib_data *value) {
  uint8_t buf[16];
  uint8_t *raw = (uint8_t*)value;

  // Read page
  esp_err_t err = _read_flash(
    iface,
    BQ34Z100G1_FLASH_CALIB_DATA,
    0,
    buf,
    16
  );
  if (err != ESP_OK) return err;

  // Do the appropriate nibble swapping
  bytesToFloat(&buf[0], &value->cc_gain); // F4
  bytesToFloat(&buf[4], &value->cc_delta); // F4
  raw[8]  = buf[9]; // O2
  raw[9]  = buf[8];
  raw[10] = buf[10]; // I1
  raw[11] = buf[11]; // I1
  raw[12] = buf[12]; // I1
  raw[13] = buf[13]; // Padding
  raw[14] = buf[15]; // U2
  raw[15] = buf[14];

  return ESP_OK;
}

esp_err_t bq34z100g1_setCalibData(I2CInterface &iface, bq34z100g1_calib_data *value) {
  uint8_t buf[16];
  uint8_t *raw = (uint8_t*)value;

  // Do the appropriate nibble swapping
  floatToBytes(value->cc_gain, &buf[0]); // F4
  floatToBytes(value->cc_delta, &buf[4]); // F4
  buf[8]  = raw[9]; // O2
  buf[9]  = raw[8];
  buf[10] = raw[10]; // I1
  buf[11] = raw[11]; // I1
  buf[12] = raw[12]; // I1
  buf[13] = raw[13]; // Padding
  buf[14] = raw[15]; // U2
  buf[15] = raw[14];

  // Write page
  return _write_flash(
    iface,
    BQ34Z100G1_FLASH_CALIB_DATA,
    0,
    buf,
    16
  );
}

esp_err_t bq34z100g1_getPackConfig(I2CInterface &iface, bq34z100g1_pack_config *value) {
  return _read_flash(
    iface,
    BQ34Z100G1_FLASH_CONFIG_REG,
    0,
    (uint8_t*)value,
    4
  );
}

esp_err_t bq34z100g1_setPackConfig(I2CInterface &iface, bq34z100g1_pack_config *value) {
  return _write_flash(
    iface,
    BQ34Z100G1_FLASH_CONFIG_REG,
    0,
    (uint8_t*)value,
    4
  );
}

esp_err_t bq34z100g1_setVoltageDivider(I2CInterface &iface, uint16_t value) {
  uint8_t buf[2];

  buf[0] = (value >> 8) & 0xFF;
  buf[1] = value & 0xFF;

  return _write_flash(
    iface,
    BQ34Z100G1_FLASH_CALIB_DATA,
    14,
    buf,
    2
  );
}
