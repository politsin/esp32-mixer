#include "mcp23017Task.h"
#include "main.h"
#include <Arduino.h>
#include <i2c.h>

#define MCP_TAG "MCP"

void mcp23x17_mode_output();
void mcp23x17_set_pump(PumpState pump);

PumpState pumps[8] = {
    PumpState{PUMP1, PUMP_STOP, 1278}, PumpState{PUMP2, PUMP_STOP, 15080},
    PumpState{PUMP3, PUMP_STOP, 5220}, PumpState{PUMP4, PUMP_STOP, 8460},
    PumpState{PUMP5, PUMP_STOP, 8010}, PumpState{PUMP6, PUMP_STOP, 14310},
    PumpState{PUMP7, PUMP_STOP, 7550}, PumpState{PUMP8, PUMP_STOP, 1220},
};
void mcp23017Task(void *pvParam) {
  mcp23x17_mode_output();
  uint8_t size = sizeof(pumps) / sizeof(pumps[0]);
  while (true) {
    for (uint8_t i = 0; i < size; i++) {
      PumpState pump = pumps[i];
      printf("pump%d = %d\n", i + 1, pump.pump);
      // Start:
      pump.mode = PUMP_START;
      mcp23x17_set_pump(pump);
      uint32_t scale = 0;
      while (pump.result < pump.task) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
        if (xTaskNotifyWait(0, 0x00, &scale, 100 / portTICK_PERIOD_MS) == pdTRUE){
          pump.result = scale;
        }
        else {
          // printf("scaleFail = %d\n", scale);
        }
        printf("p%d %d << %d\n", i + 1, pump.task, pump.result);
      }
      pump.mode = PUMP_STOP;
      mcp23x17_set_pump(pump);
      // Reverse:
      pump.mode = PUMP_REVERCE;
      mcp23x17_set_pump(pump);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      pump.mode = PUMP_STOP;
      mcp23x17_set_pump(pump);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void mcp23x17_set_pump(PumpState pump) {
  printf("set pin %d %d\n", pump.pump, pump.mode);
  uint8_t off = 0b00000000;
  uint8_t setup = 0b00000001 << pump.pump;
  if (pump.mode == PUMP_STOP) {
    master_write_cmd(ADDR_MCP23017, REG_GPIOA, off);
    master_write_cmd(ADDR_MCP23017, REG_GPIOB, off);
  }
  if (pump.mode == PUMP_START) {
    master_write_cmd(ADDR_MCP23017, REG_GPIOA, off);
    master_write_cmd(ADDR_MCP23017, REG_GPIOB, setup);
  }
  if (pump.mode == PUMP_REVERCE) {
    master_write_cmd(ADDR_MCP23017, REG_GPIOA, setup);
    master_write_cmd(ADDR_MCP23017, REG_GPIOB, off);
  }
}

void mcp23x17_mode_output() {
  esp_err_t err;
  // Config.
  err = master_write_cmd(ADDR_MCP23017, REG_IOCON, MCP_INIT);
  // Output mode.
  err = master_write_cmd(ADDR_MCP23017, REG_IODIRA, 0);
  err = master_write_cmd(ADDR_MCP23017, REG_IODIRB, 0);
  // Reset.
  err = master_write_cmd(ADDR_MCP23017, REG_GPIOA, 0x00);
  err = master_write_cmd(ADDR_MCP23017, REG_GPIOB, 0x00);
  if (err != ESP_OK) {
    ESP_LOGE(MCP_TAG, "could not setup mode_output: %s", esp_err_to_name(err));
  }
}
