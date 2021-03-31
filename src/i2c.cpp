#include "esp_err.h"
#include "esp_log.h"
#include <driver/i2c.h>

#define I2_TAG "I2C"
#define SCL_GPIO GPIO_NUM_16
#define SDA_GPIO GPIO_NUM_17

void _delay_millisec(uint32_t millisec) {
  if (millisec > 0) {
    ets_delay_us(millisec * 1000);
  }
}

void i2cInitX() {
  i2c_config_t i2c_config = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = SDA_GPIO,
      .sda_pullup_en = GPIO_PULLUP_ENABLE,
      .scl_io_num = SCL_GPIO,
      .scl_pullup_en = GPIO_PULLUP_ENABLE,
  };
  i2c_config.master.clk_speed = 100000;
  i2c_param_config(I2C_NUM_0, &i2c_config);
  i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
  ESP_LOGD(I2_TAG, "LOG instance of mjd_sht3x_config_t");
  ESP_LOGI(I2_TAG, "  i2c_scl_gpio_num: %u | PULLUP", SCL_GPIO);
  ESP_LOGI(I2_TAG, "  i2c_sda_gpio_num: %u | PULLUP", SDA_GPIO);
}

bool i2cConnectX(uint8_t addr) {
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
  i2c_master_stop(cmd);
  esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd);
  if (ret != ESP_OK) {
    return false;
  }
  return true;
}

esp_err_t master_write_byte(uint8_t addr, uint8_t reg) {
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, I2C_MASTER_NACK);
  i2c_master_write_byte(cmd, reg, I2C_MASTER_NACK);
  i2c_master_stop(cmd);
  esp_err_t err = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
  if (err != ESP_OK) {
    ESP_LOGE(I2_TAG,
             "%s() ABORT Send request i2c_master_cmd_begin() err %i (%s)",
             __FUNCTION__, err, esp_err_to_name(err));
    return 1;
  }
  i2c_cmd_link_delete(cmd);
  return err;
}

esp_err_t master_write_cmd(uint8_t addr, uint8_t reg, uint8_t command) {
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, I2C_MASTER_NACK);
  i2c_master_write_byte(cmd, reg, I2C_MASTER_NACK);
  i2c_master_write_byte(cmd, command, I2C_MASTER_NACK);
  i2c_master_stop(cmd);
  esp_err_t err = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
  if (err != ESP_OK) {
    ESP_LOGE(I2_TAG, "could not write to device: %s", esp_err_to_name(err));
    return 1;
  }
  i2c_cmd_link_delete(cmd);
  _delay_millisec(1);
  return err;
}

esp_err_t master_write_cmd16(uint8_t addr, uint8_t reg, uint16_t command) {
  uint8_t write[2];
  write[0] = command >> 8; // get 8 greater bits
  write[1] = command & 0xFF; // get 8 lower bits
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, I2C_MASTER_NACK);
  i2c_master_write_byte(cmd, reg, I2C_MASTER_NACK);
  // i2c_master_write(cmd, write, 2, 1); // write it
  i2c_master_write_byte(cmd, write[0], I2C_MASTER_NACK);
  i2c_master_write_byte(cmd, write[1], I2C_MASTER_NACK);
  i2c_master_stop(cmd);
  esp_err_t err = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
  if (err != ESP_OK) {
    ESP_LOGE(I2_TAG, "could not write to device: %s", esp_err_to_name(err));
    return 1;
  }
  i2c_cmd_link_delete(cmd);
  _delay_millisec(1);
  return err;
}

esp_err_t master_write_register(uint8_t addr, uint16_t command16) {
  uint8_t reg = (uint8_t)((command16) >> 8);
  uint8_t command = (uint8_t)command16;
  return master_write_cmd(addr, reg, command);
}

uint8_t readRegister8(uint8_t addr, uint8_t reg) {
  esp_err_t err = master_write_byte(addr, reg);
  if (err != ESP_OK) {
    ESP_LOGE(I2_TAG, "Read [0x%02x] int16 failed, err = %d", reg, err);
    return 0;
  }
  uint8_t msb;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, I2C_MASTER_NACK);
  i2c_master_read_byte(cmd, &msb, I2C_MASTER_ACK);
  i2c_master_stop(cmd);
  i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd);
  uint8_t result = msb;
  return result;
}

int16_t readRegister16(uint8_t addr, uint8_t reg) {
  esp_err_t err = master_write_byte(addr, reg);
  if (err != ESP_OK) {
    ESP_LOGE(I2_TAG, "Read [0x%02x] int16 failed, err = %d", reg, err);
    return 0;
  }
  uint8_t msb;
  uint8_t lsb;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, I2C_MASTER_NACK);
  i2c_master_read_byte(cmd, &msb, I2C_MASTER_NACK);
  i2c_master_read_byte(cmd, &lsb, I2C_MASTER_ACK);
  i2c_master_stop(cmd);
  err = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
  if (err != ESP_OK) {
    ESP_LOGE(I2_TAG, "Read [0x%02x] int16 failed, err = %d", reg, err);
    return 0;
  }
  i2c_cmd_link_delete(cmd);
  int16_t result = ((msb << 8) | lsb);
  return result;
}

uint16_t readRegisterU16(uint8_t addr, uint8_t reg) {
  return (uint16_t)readRegister16(addr, reg);
}

uint32_t readRegister24(uint8_t addr, uint8_t reg) {
  esp_err_t err = master_write_byte(addr, reg);
  if (err != ESP_OK) {
    ESP_LOGE(I2_TAG, "Read [0x%02x] int16 failed, err = %d", reg, err);
    return 0;
  }

  uint8_t msb;
  uint8_t lsb;
  uint8_t xlsb;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, I2C_MASTER_NACK);
  i2c_master_read_byte(cmd, &msb, I2C_MASTER_NACK);
  i2c_master_read_byte(cmd, &lsb, I2C_MASTER_NACK);
  i2c_master_read_byte(cmd, &xlsb, I2C_MASTER_ACK);
  i2c_master_stop(cmd);
  err = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
  if (err != ESP_OK) {
    ESP_LOGE(I2_TAG, "Read [0x%02x] int24 failed, err = %d", reg, err);
    return 0;
  }
  printf("Register24 %02x: %02x %02x %02x\n", reg, msb, lsb, xlsb);
  i2c_cmd_link_delete(cmd);
  uint16_t result = ((msb << 16) | (lsb << 8) | xlsb);
  return result;
}
/*********************************************************************************
 * _check_crc()
 *
 * @return ESP_OK ESP_ERR_INVALID_CRC
 *
 * @doc (rx_buf+3, rx_buf[5]);
 *
 * @datasheet 4.12 Checksum Calculation
 *    The 8-bit CRC checksum transmitted after each data word is generated by a
 *CRC algorithm. Its properties are displayed in Table 20. The CRC covers the
 *contents of the two previously transmitted data bytes. To calculate the
 *checksum only these two previously transmitted data bytes are used. Property
 *Value
 *      -------         --------
 *      Name            CRC-8
 *      Width           8 bit
 *      Protected data  read and/or write data
 *      Polynomial      0x31 (x8 + x5 + x4 + 1)
 *      Initialization  0xFF
 *      Reflect input   false
 *      Reflect output  false
 *      Final XOR       0x00
 *
 * Polynomial for CRC is 0x31 and not 0x131 (an 8-bit checksum can't use the 9th
 *bit in the polynomial)
 *
 *  Example CRC (0xBEEF) = 0x92
 *
 *********************************************************************************/
esp_err_t _compute_crc(const uint8_t *param_data, int param_len,
                       uint8_t *param_computed_value) {
  ESP_LOGD(I2_TAG, "%s()", __FUNCTION__);

  esp_err_t f_retval = ESP_OK;

  // calculates 8-Bit checksum with given polynomial
  const uint8_t POLYNOMIAL = 0x31;
  uint8_t crc = 0xFF; // @important initial value 0xFF
  for (uint8_t idx = 0; idx < param_len; idx++) {
    crc ^= (param_data[idx]);
    for (uint8_t bit = 8; bit > 0; --bit) {
      if (crc & 0x80) {
        crc = (crc << 1) ^ POLYNOMIAL;
      } else {
        crc = (crc << 1);
      }
    }
  }

  *param_computed_value = crc;

  return f_retval;
}

esp_err_t _check_crc(const uint8_t *param_data, int param_len,
                     uint8_t param_expected_value) {
  ESP_LOGD(I2_TAG, "%s()", __FUNCTION__);

  esp_err_t f_retval = ESP_OK;
  uint8_t crc = 0;
  _compute_crc(param_data, param_len, &crc);
  if (crc != param_expected_value) {
    f_retval = ESP_ERR_INVALID_CRC;
  }

  return f_retval;
}
