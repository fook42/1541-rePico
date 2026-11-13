/**********************************
 * sound routines
 *
 * Author: F00K42
 * Last change: 2026/11/13
 ***********************************/

#include "sound.h"

void init_sound(void)
{
    gpio_init(GPIO_SND);
    gpio_set_dir(GPIO_SND, GPIO_OUT);
    turn_sound_off();
}
