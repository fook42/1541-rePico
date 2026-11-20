/**********************************
 * sound routines
 *
 * Author: F00K42
 * Last change: 2026/11/20
 ***********************************/

#include "main.h"
#include "sound.h"
#include "shiftedWavData.h"
#include "shiftedWavData1.h"
#include "shiftedWavData2.h"
#include "shiftedWavData3.h"
#include <stdlib.h>

// All selectable sounds: index → pointer to the PCM data.
// 0 = song, 1 = 1kHz tone, 2 = 2kHz tone, 3 = step ramp
const uint32_t *sound_data[] = {
    shifted_wav_data,   // shiftedWavData.h
    shifted_wav_data1,  // shiftedWavData1.h
    shifted_wav_data2,  // shiftedWavData2.h
    shifted_wav_data3   // shiftedWavData3.h
};
const unsigned int sound_len[] = {
    shifted_wav_data_len,
    shifted_wav_data1_len,
    shifted_wav_data2_len,
    shifted_wav_data3_len
};
#define NUM_SOUNDS (sizeof(sound_data) / sizeof(sound_data[0]))


int  sample_dma_chan;
uint slice_num=0;
pwm_config config;
// DMA channel configuration (needed by select_sound to reconfigure the channel)
static dma_channel_config sample_dma_chan_config;

void init_sound(void) {

    // Tell the LED pin that the PWM is in charge of its value.
    gpio_set_function(GPIO_SND, GPIO_FUNC_PWM);
    // Figure out which slice we just connected to the LED pin
    slice_num = pwm_gpio_to_slice_num(GPIO_SND);
    // Mask our slice's IRQ output into the PWM block's single interrupt line,
    // and register our interrupt handler
    pwm_clear_irq(slice_num);
   // pwm_set_irq_enabled(slice_num, true);
   // irq_set_exclusive_handler(PWM_DEFAULT_IRQ_NUM(), on_pwm_wrap);
   // irq_set_enabled(PWM_DEFAULT_IRQ_NUM(), true);
    // Get some sensible defaults for the slice configuration. By default, the
    // counter is allowed to wrap over its maximum range (0 to 2**16-1)
    config = pwm_get_default_config();
    // Set divider, reduces counter clock to sysclock/this value
    pwm_config_set_clkdiv(&config, 36.f);  // around 16kHz Samplefrequency
    pwm_config_set_wrap(&config, 255);
    pwm_config_set_output_polarity(&config, false,false);
    pwm_config_set_clkdiv_mode(&config, PWM_DIV_FREE_RUNNING);
    // Load the configuration into our PWM slice, and set it stopped.
    pwm_init(slice_num, &config, false);
    pwm_set_gpio_level(GPIO_SND, 0x80);

     // Setup sample DMA channel
    sample_dma_chan  = dma_claim_unused_channel(true);
    sample_dma_chan_config = dma_channel_get_default_config(sample_dma_chan);
    // Transfer 8-bits at a time
    channel_config_set_transfer_data_size(&sample_dma_chan_config, DMA_SIZE_32);
    // Increment read address to go through audio buffer
    channel_config_set_read_increment(&sample_dma_chan_config, true);
    // Always write to the same address
    channel_config_set_write_increment(&sample_dma_chan_config, false);
    channel_config_set_dreq(&sample_dma_chan_config, pwm_get_dreq(slice_num));
    // Reconfigure the DMA channel to point at the selected sound's data.
    
    dma_channel_configure(
        sample_dma_chan,
        &sample_dma_chan_config,
        &pwm_hw->slice[slice_num].cc,
        sound_data[0],
        sound_len[0],
        true
    );
   
}
void select_sound(uint8_t sound_idx) {
    if (sound_idx >= (int)NUM_SOUNDS) {
        return;
    }
    // Reconfigure the DMA channel to point at the selected sound's data.
    // turn_sound_on() re-arms the channel, so this takes effect on next start.
    dma_channel_set_read_addr(sample_dma_chan, sound_data[sound_idx], false);
    dma_channel_set_trans_count(sample_dma_chan, sound_len[sound_idx], false);
}
void turn_sound_on(uint8_t index) {
    pwm_set_enabled(slice_num, false);
    dma_channel_abort(sample_dma_chan);   // Reset the DMA channel before starting it
    select_sound(index);
    dma_channel_start(sample_dma_chan);
    // Load the configuration into our PWM slice, and set it running.
    pwm_set_enabled(slice_num, true);
}
void turn_sound_off(void) {
    dma_channel_abort(sample_dma_chan);   // Reset the DMA channel 
    pwm_set_enabled(slice_num, false);
}
