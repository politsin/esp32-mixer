#include "mcp23017Task.h"
#include "main.h"
#include <Arduino.h>
#include <i2c.h>

typedef enum {
  STOP,
  START,
  REVERCE
} pump_mode_t;
void mcp23x17_mode_output();
void mcp23x17_set_pump(uint16_t pump, pump_mode_t mode);

void mcp23017Task(void *pvParam) {
  mcp23x17_mode_output();
  bool isOn = 0;
  pump_mode_t pumps[8] = {STOP};
  while (true) {
    // for (int i = 0; i < 8; i++) {
    //   // mcp23x17_set_level(i, isOn);
    //   printf("pump %d = %d\n", i, pumps[i]);
    //   // mcp23x17_set_pump(i, START);
    //   // mcp23x17_set_pump(i, STOP);
    //   // mcp23x17_set_pump(i, REVERCE);
    //   // mcp23x17_set_pump(i, STOP);
    //   uint8_t mask = 0b00000000;
    //   master_write_cmd(ADDR_MCP23017, REG_GPIOA, mask);
    //   vTaskDelay(2000 / portTICK_PERIOD_MS);
    //   uint8_t setup = ~(0b00000001 << i);
    //   master_write_cmd(ADDR_MCP23017, REG_GPIOA, setup);
    //   vTaskDelay(2000 / portTICK_PERIOD_MS);
    //   uint8_t maskb = 0b00000000;
    //   master_write_cmd(ADDR_MCP23017, REG_GPIOB, mask);
    //   vTaskDelay(2000 / portTICK_PERIOD_MS);
    //   uint8_t setupb = ~(0b00000001 << i);
    //   master_write_cmd(ADDR_MCP23017, REG_GPIOB, setupb);
    //   vTaskDelay(2000 / portTICK_PERIOD_MS);
    // }
    master_write_cmd(ADDR_MCP23017, REG_GPIOA, 0x00);
    master_write_cmd(ADDR_MCP23017, REG_GPIOB, 0xff);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    master_write_cmd(ADDR_MCP23017, REG_GPIOA, 0x00);
    master_write_cmd(ADDR_MCP23017, REG_GPIOB, 0x00);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    master_write_cmd(ADDR_MCP23017, REG_GPIOA, 0xff);
    master_write_cmd(ADDR_MCP23017, REG_GPIOB, 0x00);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    master_write_cmd(ADDR_MCP23017, REG_GPIOA, 0x00);
    master_write_cmd(ADDR_MCP23017, REG_GPIOB, 0x00);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    isOn = !isOn;
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void mcp23x17_set_pump(uint16_t pump, pump_mode_t mode) {
  printf("set pin %d %d\n", pump, mode);
  // uint16_t command;
  // command = (command & ~BV(pin)) | (REG_GPIOA ? BV(pin) : 0);
  // esp_err_t err = master_write_cmd(ADDR_MCP23017, REG_IODIRA, command);
  // if (err != ESP_OK) {
  //   printf("errrrrrrrrrrrrrrrrrr\n");
  // }
  if (false) {

  }
}

void mcp23x17_mode_output() {
  esp_err_t err;
  // Config.
  err = master_write_cmd(ADDR_MCP23017, REG_IOCON, MCP_INIT);
  // Input mode.
  err = master_write_cmd(ADDR_MCP23017, REG_IODIRA, 0);
  err = master_write_cmd(ADDR_MCP23017, REG_IODIRB, 0);
  // Reset.
  err = master_write_cmd(ADDR_MCP23017, REG_GPIOA, 0x00);
  err = master_write_cmd(ADDR_MCP23017, REG_GPIOB, 0x00);
  for (uint8_t i = 0; i < 8; i++) {
    /* code */
  }

  if (err != ESP_OK) {
    printf("errrrrrrrrrrrrrrrrrr\n");
  }
  else {
    printf("okokoko\n");
  }
}
