/* basic i2c routines for Pico2 */
// header
// implementation: F00K42
// last change: 11/10/2025

#ifndef _I2C_INCLUDE_
#define _I2C_INCLUDE_

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "pinout.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_READ    (1)
#define I2C_WRITE   (0)

void my_i2c_init(void);

bool my_i2c_test(uint8_t addr);

#endif
