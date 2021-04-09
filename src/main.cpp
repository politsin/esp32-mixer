#include <Arduino.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include <i2c.h>
/**
 * I2C:
 * D16 - Clock
 * D17 - Data
 */
using std::string;
// Tasks.
#include "hx711Task.h"
#include "mcp23017Task.h"

// Log
#include "esp_err.h"
#include "esp_log.h"
#define TAG "LOG"

TaskHandle_t mcp23017;
TaskHandle_t hx711;
void setup() {
  i2cInitX();
  ESP_LOGI(TAG, "setup");
  printf("setup %d\n", 1);
  xTaskCreate(hx711Task, "hx711", 1024 * 16, NULL, 1, &hx711);
  xTaskCreate(mcp23017Task, "mcp23017", 1024 * 32, NULL, 1, &mcp23017);
}

void loop() {
  uint16_t delay = 15.5; // sec.
  vTaskDelay(delay * 1000 / portTICK_PERIOD_MS);
  // printf("Loop %ds \n", delay);
}
