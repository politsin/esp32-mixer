#pragma once
#include <Arduino.h>

extern TaskHandle_t mcp23017;
void mcp23017Task(void *pvParam);

/**
 * GPIO mode
 */
typedef enum {
  MCP23X17_GPIO_OUTPUT = 0,
  MCP23X17_GPIO_INPUT
} mcp23x17_gpio_mode_t;


#define I2C_FREQ_HZ 1000000  // Max 1MHz for esp-idf, but device supports up to 1.7Mhz

// TODO: check this staff.
// BANK   = 0 : sequential register addresses
// MIRROR = 0 : use configureInterrupt
// SEQOP  = 1 : sequential operation disabled, address pointer not increment
// DISSLW = 0 : slew rate enabled
// HAEN   = 0 : hardware address pin is always enabled on 23017
// ODR    = 0 : open drain output
// INTPOL = 0 : interrupt active low
// writeRegister(MCP23017Register::IOCON, 0b00100000);
#define MCP_INIT 0b00100000

#define REG_IODIRA   0x00
#define REG_IODIRB   0x01
#define REG_IPOLA    0x02
#define REG_IPOLB    0x03
#define REG_GPINTENA 0x04
#define REG_GPINTENB 0x05
#define REG_DEFVALA  0x06
#define REG_DEFVALB  0x07
#define REG_INTCONA  0x08
#define REG_INTCONB  0x09
#define REG_IOCON    0x0A
#define REG_GPPUA    0x0C
#define REG_GPPUB    0x0D
#define REG_INTFA    0x0E
#define REG_INTFB    0x0F
#define REG_INTCAPA  0x10
#define REG_INTCAPB  0x11
#define REG_GPIOA    0x12
#define REG_GPIOB    0x13
#define REG_OLATA    0x14
#define REG_OLATB    0x15

#define BIT_IOCON_INTPOL 1
#define BIT_IOCON_ODR    2
#define BIT_IOCON_HAEN   3
#define BIT_IOCON_DISSLW 4
#define BIT_IOCON_SEQOP  5
#define BIT_IOCON_MIRROR 6
#define BIT_IOCON_BANK   7
