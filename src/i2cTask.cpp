#include <Arduino.h>
using std::string;
// i2c
#include "driver/i2c.h"
#include "esp_types.h"

#include <i2c.h>
void i2cConnect(uint8_t addr);
void i2cTask(void *pvParam) {
  while (true) {
    uint8_t addr;
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    printf("00:         ");
    for (addr = 3; addr < 0x78; addr++) {
      (addr % 16 == 0) ? printf("\n%.2x:", addr) : false;
      if (i2cConnectX(addr)) {
        printf(" %.2x", addr);
      } else {
        printf(" --");
      };
    }
    printf("\n");
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}
