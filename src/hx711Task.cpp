#include "hx711Task.h"
#include "hx711.h"
#include "main.h"
#include <Arduino.h>
#include <i2c.h>

#define HX_SDA GPIO_NUM_26
#define HX_SCK GPIO_NUM_27
// scale_calibration, side A and B
float scale_calibration_A = 2030;
float scale_calibration_B = 2030;

int32_t scale(uint8_t count, hx711_t &dev);

void hx711Task(void *pvParam) {
  hx711_t dev = {
      .dout = HX_SDA,
      .pd_sck = HX_SCK,
      .gain = HX711_GAIN_A_128,
  };
  // initialize device
  while (true) {
    esp_err_t r = hx711_init(&dev);
    if (r == ESP_OK)
      break;
    printf("Could not initialize HX711: %d (%s)\n", r, esp_err_to_name(r));
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
  uint8_t count = 1;
  while (true) {
    int32_t result = scale(count, dev);
    // printf("Raw data: %d\n", data);
    printf("%d\n", result);

    vTaskDelay(100 / portTICK_PERIOD_MS);
    // printf("hx711Task - loop\n");
  }
}

int compare(const void *a, const void *b) { return *(int *)a - *(int *)b; }

int32_t scale(uint8_t count, hx711_t &dev) {
  int32_t scales[] = {count};
  int32_t data;
  for (uint8_t c = 0; c < count; c++) {
    esp_err_t r = hx711_wait(&dev, 500);
    if (r != ESP_OK) {
      printf("Device not found: %d (%s)\n", r, esp_err_to_name(r));
      continue;
    }

    r = hx711_read_data(&dev, &data);
    scales[c] = data;
    if (r != ESP_OK) {
      printf("Could not read data: %d (%s)\n", r, esp_err_to_name(r));
      continue;
    }
  }
  int i = 0;
  int j = 0;
  // for (i = 0; i < count; ++i)
  //   printf("%d ", scales[i]);
  // printf("\n");
  // for (i = 0; i < count; ++i) {
  //   for (j = i + 1; j < count; ++j) {
  //     if (scales[i] == scales[j]) {
  //       // printf("can't find median\n");
  //       return 0;
  //     }
  //   }
  // }

  // qsort(scales, count, sizeof(int), compare);

  // printf("Median is %d\n", scales[count / 2]);
  return data;
}
