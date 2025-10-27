/* pinout assignment for Pico2 to baseboard */
// header
// implementation: F00K42
// last change: 25/10/2025

#include "hardware/spi.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT    spi0
#define SPI_MISO    16
#define SPI_CS      17
#define SPI_SCK     18
#define SPI_MOSI    19

// I2C defines
#define I2C_PORT    i2c0
#define I2C_SDA     20
#define I2C_SCL     21

// Via connection
#define GPIO_SOE    0
#define GPIO_OE     1
#define GPIO_SYNC   2
#define GPIO_SOE_GA 3
#define GPIO_MTR    4
#define GPIO_WPS    5
#define GPIO_STP0   6
#define GPIO_STP1   7

#define GPIO_PAPORT 8  //defines the first bit of 8
/*
PA0 8
PA1 9
PA2 10
PA3 11
PA4 12
PA5 13
PA6 14
PA7 15
*/
#define GPIO_BRDY   28

// 3 für den Drück-Dreh-Schalter
#define GPIO_BT1    27
#define GPIO_BT2    26
#define GPIO_BT3    22
