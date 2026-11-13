/**********************************
 * header - sound routines
 *
 * Author: F00K42
 * Last change: 2026/11/13
 ***********************************/

#ifndef _SOUND_H_
#define _SOUND_H_

#include <stdbool.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "hardware/timer.h"

#include "pinout.h"
#include "globals.h"

void init_sound(void);

#if PCB_VERSION>=17
#define turn_sound_on()     gpio_put(GPIO_SND,true)
#define turn_sound_off()    gpio_put(GPIO_SND,false)
#else
#define turn_sound_on()     {}
#define turn_sound_off()    {}
#endif

#endif
