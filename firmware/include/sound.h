/**********************************
 * header - sound routines
 *
 * Author: F00K42
 * Last change: 2026/11/20
 ***********************************/

#ifndef _SOUND_H_
#define _SOUND_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "hardware/dma.h"

// sound state and control functions
void init_sound(void);
void turn_sound_on(uint8_t index);
void turn_sound_off(void);
enum {
    SOUND_SONG = 0,
    SOUND_1KHZ,
    SOUND_500HZ,
    SOUND_STEP,
    NUM_SOUNDS
};

#endif
