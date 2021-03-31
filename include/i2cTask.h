#pragma once

#include "driver/gpio.h"
#include <Arduino.h>

TaskHandle_t i2c_handle;
void i2cTask(void *pvParam);
