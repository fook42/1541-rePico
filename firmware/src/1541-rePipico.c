#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/interp.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"

#include "pinout.h"

#define _EXTERN_

#include "display.h"
#include "lcd.h"
#include "oled.h"
#include "i2c.h"
#include "gcr.h"


#define STP0_PIN 6
#define STP1_PIN 7

uint8_t stepper_signal_puffer[0x80]; // Ringpuffer für Stepper Signale (128 Bytes)
volatile uint8_t stepper_signal_r_pos = 0;
volatile uint8_t stepper_signal_w_pos = 0;
volatile uint8_t stepper_signal_time = 0;
volatile uint8_t stepper_signal = 0;



// Data will be copied from src to dst
const char src[] = "Hello, world! (from DMA)";
char dst[count_of(src)];

#include "blink.pio.h"

void blink_SPI_forever(PIO pio, uint sm, uint offset, uint pin, uint freq) {
    blink_program_init(pio, sm, offset, pin);
    pio_sm_set_enabled(pio, sm, true);

    printf("Blinking pin %d at %d Hz\n", pin, freq);

    // PIO counter program takes 3 more cycles in total than we pass as
    // input (wait for n + 1; mov; jmp)
    pio->txf[sm] = (125000000 / (2 * freq)) - 3;
}


int64_t alarm_callback(alarm_id_t id, void *user_data) {
    // Put your timeout handler code in here
    return 0;
}


void gpio_callback(uint gpio, uint32_t events)
{
    // general gpio-ISR .. triggered for STP0 or STP1 change.. no need to detect the cause
    stepper_signal_puffer[stepper_signal_w_pos & 0x7F] = ((bool_to_bit(gpio_get(STP0_PIN))) | (bool_to_bit(gpio_get(STP1_PIN))<<1));
    stepper_signal_w_pos++;
}


int main()
{
    stdio_init_all();

    // set interrupt on pin-change for STP0 and STP1 ...
    gpio_init(STP0_PIN);
    gpio_init(STP1_PIN);
    gpio_set_dir(STP0_PIN, GPIO_IN);
    gpio_set_dir(STP1_PIN, GPIO_IN);
    gpio_set_irq_enabled_with_callback(STP0_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(STP1_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(SPI_MISO, GPIO_FUNC_SPI);
    gpio_set_function(SPI_CS,   GPIO_FUNC_SIO);
    gpio_set_function(SPI_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(SPI_MOSI, GPIO_FUNC_SPI);
    

    // gpio_put_masked(0xFF<<GPIO_PAPORT,value<<GPIO_PAPORT);

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(SPI_CS, GPIO_OUT);
    gpio_put(SPI_CS, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    if (display_init())
    {
        display_clear();
        display_home();
//        display_string("Pico2\001\002\003\004\005\000xx");
    }

    // Get a free channel, panic() if there are none
    int chan = dma_claim_unused_channel(true);
    
    // 8 bit transfers. Both read and write address increment after each
    // transfer (each pointing to a location in src or dst respectively).
    // No DREQ is selected, so the DMA transfers as fast as it can.
    
    dma_channel_config c = dma_channel_get_default_config(chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, true);
    
    dma_channel_configure(
        chan,          // Channel to be configured
        &c,            // The configuration we just created
        dst,           // The initial write address
        src,           // The initial read address
        count_of(src), // Number of transfers; in this case each is 1 byte.
        true           // Start immediately.
    );
    
    // We could choose to go and do something else whilst the DMA is doing its
    // thing. In this case the processor has nothing else to do, so we just
    // wait for the DMA to finish.
    dma_channel_wait_for_finish_blocking(chan);
    
    // The DMA has now copied our text from the transmit buffer (src) to the
    // receive buffer (dst), so we can print it out from there.
    puts(dst);

    // PIO Blinking examplec
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &blink_program);
    printf("Loaded program at %d\n", offset);
    
    #ifdef PICO_DEFAULT_LED_PIN
    blink_SPI_forever(pio, 0, offset, PICO_DEFAULT_LED_PIN, 1);
    #else
    blink_SPI_forever(pio, 0, offset, 6, 3);
    #endif
    // For more pio examples see https://github.com/raspberrypi/pico-examples/tree/master/pio

    // Interpolator example code
    interp_config cfg = interp_default_config();
    // Now use the various interpolator library functions for your use case
    // e.g. interp_config_clamp(&cfg, true);
    //      interp_config_shift(&cfg, 2);
    // Then set the config 
    interp_set_config(interp0, 0, &cfg);
    // For examples of interpolator use see https://github.com/raspberrypi/pico-examples/tree/master/interp

    // Timer example code - This example fires off the callback after 2000ms
    add_alarm_in_ms(2000, alarm_callback, NULL, false);
    // For more examples of timer use see https://github.com/raspberrypi/pico-examples/tree/master/timer

    printf("System Clock Frequency is %d Hz\n", clock_get_hz(clk_sys));
    printf("USB Clock Frequency is %d Hz\n", clock_get_hz(clk_usb));
    // For more examples of clocks use see https://github.com/raspberrypi/pico-examples/tree/master/clocks

    uint8_t stepper;
    uint8_t dsp_zeile = 0;
    stepper_signal_puffer[0]=212; // fake data 11010100
    stepper_signal_w_pos++;
    while (true) {
//        printf("Hello, world!\n");
        if (stepper_signal_r_pos != stepper_signal_w_pos)
        {
            stepper = stepper_signal_puffer[stepper_signal_r_pos & 0x7F] | stepper_signal_puffer[(stepper_signal_r_pos-1) & 0x7F]<<2;
            stepper_signal_r_pos++;

            display_clear();
            display_setcursor(0, dsp_zeile);
            for (int i=7; i>=0; --i)
            {
                display_data((stepper&(1<<i))?'1':'0');
            }
            dsp_zeile = 1-dsp_zeile;
        }
        sleep_ms(1000);
    }
}
