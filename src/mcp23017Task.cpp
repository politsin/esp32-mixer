#include "mcp23017Task.h"
#include "main.h"
#include <Arduino.h>
#include <i2c.h>

#define MCP_TAG "MCP"

void mcp23x17_mode_output();
void mcp23x17_set_pump(PumpState pump);

PumpState pumps[8] = {
    PumpState{PUMP1, PUMP_STOP, 0, 0},    PumpState{PUMP2, PUMP_STOP, 0, 700},
    PumpState{PUMP3, PUMP_STOP, 1, 1300}, PumpState{PUMP4, PUMP_STOP, 1, 0},
    PumpState{PUMP5, PUMP_STOP, 1, 0},    PumpState{PUMP6, PUMP_STOP, 0, 0},
    PumpState{PUMP7, PUMP_STOP, 0, 0},    PumpState{PUMP8, PUMP_STOP, 0, 0},
};
void mcp23017Task(void *pvParam) {
  mcp23x17_mode_output();
  uint8_t drop;
  uint8_t size = sizeof(pumps) / sizeof(pumps[0]);
  int32_t tare = 0;
  int32_t delta = 0;
  uint32_t scale = 0;
  TickType_t delay = 100 / portTICK_PERIOD_MS;
  // await for scale.
  if (xTaskNotifyWait(0, 0x00, &scale, portMAX_DELAY) == pdTRUE) {
    tare = (int32_t)scale;
  }
  for (uint8_t i = 0; i < size; i++) {
    PumpState pump = pumps[i];
    printf("pump%d = %d\n", i + 1, pump.pump);
    if (pump.task > 0) {
      // Start:
      pump.mode = PUMP_START;
      mcp23x17_set_pump(pump);
      while (pumps[i].result < pump.task) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
        if (xTaskNotifyWait(0, 0x00, &scale, delay) == pdTRUE) {
          if ((int32_t)scale > tare) {
            pumps[i].result = (int32_t)scale - tare;
          }
        }
        delta = pump.task - pumps[i].result;
        printf("p%d %d << %d [d = %d]\n", i + 1, pump.task, pumps[i].result,
               delta);
        if (delta < 500) {
          pump.mode = PUMP_STOP;
          mcp23x17_set_pump(pump);
          vTaskDelay(200 / portTICK_PERIOD_MS);
          pump.mode = PUMP_START;
          mcp23x17_set_pump(pump);
          drop = (delta < 80) ? 3 : 15;
          vTaskDelay(drop / portTICK_PERIOD_MS);
          pump.mode = PUMP_STOP;
          mcp23x17_set_pump(pump);
        }
      }
      // Reverse:
      pump.mode = PUMP_REVERCE;
      mcp23x17_set_pump(pump);
      vTaskDelay(3000 / portTICK_PERIOD_MS);
      // Stop:
      pump.mode = PUMP_STOP;
      mcp23x17_set_pump(pump);
      // Scale.
      vTaskDelay(500 / portTICK_PERIOD_MS);
      if (xTaskNotifyWait(0, 0x00, &scale, portMAX_DELAY) == pdTRUE) {
        if ((int32_t)scale > tare) {
          tare = (int32_t)scale;
          pumps[i].result = (int32_t)scale - tare;
          delta = pumps[i].result - pump.task;
          printf("p%d %d !< %d [d = %d]\n", i + 1, pump.task, pumps[i].result,
                 delta);
        }
      }
    }
  }
  while (true) {
    uint32_t stakan = 5000;
    uint32_t sum_task = 0;
    uint32_t sum_result = 0;
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    if (xTaskNotifyWait(0, 0x00, &scale, delay) == pdTRUE &&
        (int32_t)scale > 0) {
      for (uint8_t i = 0; i < size; i++) {
        PumpState pump = pumps[i];
        sum_task += pump.task;
        sum_result += pump.result;
        int32_t d = pump.result - pump.task;
        printf("p%d %d << %d [d = %d]\n", i + 1, pump.task, pump.result, d);
      }
      int32_t d2 = sum_result - sum_task;
      printf("TASK:%d \n", sum_task);
      printf("RESULT:%d, SUM:%d DELTA = %d\n", scale, sum_result, d2);
      printf("TARE-TASK:%d TARE-RESULT:%d\n", sum_task + stakan,
             sum_result + stakan);
    }
    vTaskDelay(7000 / portTICK_PERIOD_MS);
  }
}

void mcp23x17_set_pump(PumpState pump) {
  // printf("set pin %d %d\n", pump.pump, pump.mode);
  uint8_t off = 0b00000000;
  uint8_t setup = 0b00000001 << pump.pump;
  if (pump.mode == PUMP_STOP) {
    master_write_cmd(ADDR_MCP23017, REG_GPIOA, off);
    master_write_cmd(ADDR_MCP23017, REG_GPIOB, off);
  }
  if (pump.mode == PUMP_START) {
    if (!pump.reverse) {
      master_write_cmd(ADDR_MCP23017, REG_GPIOA, setup);
      master_write_cmd(ADDR_MCP23017, REG_GPIOB, off);
    } else {
      master_write_cmd(ADDR_MCP23017, REG_GPIOA, off);
      master_write_cmd(ADDR_MCP23017, REG_GPIOB, setup);
    }
  }
  if (pump.mode == PUMP_REVERCE) {
    if (!pump.reverse) {
      master_write_cmd(ADDR_MCP23017, REG_GPIOA, off);
      master_write_cmd(ADDR_MCP23017, REG_GPIOB, setup);
    } else {
      master_write_cmd(ADDR_MCP23017, REG_GPIOA, setup);
      master_write_cmd(ADDR_MCP23017, REG_GPIOB, off);
    }
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
