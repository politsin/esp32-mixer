#include "hx711Task.h"
#include "hx711.h"
#include "main.h"
#include "mcp23017Task.h"
#include <Arduino.h>
#include <i2c.h>

#define HX_SDA GPIO_NUM_26
#define HX_SCK GPIO_NUM_27
// scale_calibration, side A and B
const uint8_t NUM_READ = 16;
float scale_calibration_A = 2030;
float scale_calibration_B = 2030;

int32_t scale(uint8_t count, hx711_t &dev);
int32_t inithx711(hx711_t &dev);
int32_t runMiddleArifmOptim(int32_t newVal);

// TaskHandle_t mcp23017;
void hx711Task(void *pvParam) {
  hx711_t dev = {
      .dout = HX_SDA,
      .pd_sck = HX_SCK,
      .gain = HX711_GAIN_A_128,
  };
  int32_t tare = inithx711(dev);
  uint8_t count = 1;
  while (true) {
    int32_t result = scale(count, dev) - tare;
    uint32_t scale = -result / 205;
    if (xTaskNotify(mcp23017, scale, eSetValueWithOverwrite) == pdPASS) {
      /* The task's notification value was updated. */
    } else {
      printf("SCALE: %d\n", scale);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

int32_t scale(uint8_t count, hx711_t &dev) {
  int32_t data;
  for (uint8_t c = 0; c < count; c++) {
    esp_err_t r = hx711_wait(&dev, 500);
    if (r != ESP_OK) {
      printf("Device not found: %d (%s)\n", r, esp_err_to_name(r));
      continue;
    }

    r = hx711_read_data(&dev, &data);
    data = runMiddleArifmOptim(data);
    if (r != ESP_OK) {
      printf("Could not read data: %d (%s)\n", r, esp_err_to_name(r));
      continue;
    }
  }
  return data;
}

int32_t inithx711(hx711_t &dev) {
  while (true) {
    esp_err_t r = hx711_init(&dev);
    if (r == ESP_OK)
      break;
    printf("Could not initialize HX711: %d (%s)\n", r, esp_err_to_name(r));
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
  uint8_t count = 32;
  int32_t tare = scale(count, dev);
  return tare;
}

// Filter: https://alexgyver.ru/lessons/filters/
int32_t runMiddleArifmOptim(int32_t newVal) {
  static int t = 0;
  static int vals[NUM_READ];
  static int average = 0;
  if (++t >= NUM_READ)
    t = 0;
  average -= vals[t];
  average += newVal;
  vals[t] = newVal;
  return ((int32_t)average / NUM_READ);
}
