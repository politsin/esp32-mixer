#pragma once
#include <Arduino.h>

extern TaskHandle_t hx711;
void hx711Task(void *pvParam);
