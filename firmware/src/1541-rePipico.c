#include <stdio.h>
#include "pico/stdlib.h"
// #include "hardware/spi.h"
#include "hardware/i2c.h"
// #include "hardware/dma.h"
// #include "hardware/pio.h"
// #include "hardware/interp.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"

#include "pinout.h"

#define _EXTERN_

#include "display.h"
#include "lcd.h"
#include "oled.h"
#include "i2c.h"
#include "gcr.h"
#include "menu.h"

#include "hw_config.h"
#include "f_util.h"
#include "ff.h"

uint8_t stepper_signal_puffer[0x80]; // Ringpuffer für Stepper Signale (128 Bytes)
volatile uint8_t stepper_signal_r_pos = 0;
volatile uint8_t stepper_signal_w_pos = 0;
volatile uint8_t stepper_signal_time = 0;
volatile uint8_t stepper_signal = 0;

// -- MENU preparation --
MENU_STRUCT main_menu;
MENU_STRUCT image_menu;
MENU_STRUCT settings_menu;
MENU_STRUCT info_menu;

enum  MENU_IDS{M_BACK, M_IMAGE, M_SETTINGS, M_INFO, \
               M_BACK_IMAGE, M_INSERT_IMAGE, M_UNMOUNT_IMAGE, M_WP_IMAGE, M_NEW_IMAGE, M_SAVE_IMAGE, \
               M_BACK_SETTINGS, M_PIN_PB2, M_PIN_PB3, M_SAVE_EEPROM, M_RESTART, \
               M_BACK_INFO, M_VERSION_INFO, M_SDCARD_INFO};

/// Hauptmenü
MENU_ENTRY main_menu_entrys[] = {{"Disk Menu", M_IMAGE,ENTRY_MENU,   0,&image_menu},
                                {"Settings",  M_SETTINGS,ENTRY_MENU,0,&settings_menu},
                                {"Info",      M_INFO,ENTRY_MENU,    0,&info_menu}};
/// Image Menü
MENU_ENTRY image_menu_entrys[] = {{"Insert Image", M_INSERT_IMAGE},
                                {"Unmount Image",M_UNMOUNT_IMAGE},
                                {"WriteProt.",   M_WP_IMAGE,ENTRY_ONOFF,1},
                                {"New Image",    M_NEW_IMAGE},
                                {"Save Image",   M_SAVE_IMAGE}};
/// Setting Menü
MENU_ENTRY settings_menu_entrys[] = {{"Pin PB2",M_PIN_PB2, ENTRY_ONOFF, 0},
                                    {"Pin PB3",M_PIN_PB3, ENTRY_ONOFF, 0},
                                    {"Restart",M_RESTART}};
/// Info Menü
MENU_ENTRY info_menu_entrys[] = {{"Version",     M_VERSION_INFO},
                                {"SD Card Info",M_SDCARD_INFO}};
// -----------

void gpio_callback(uint gpio, uint32_t events)
{
    // general gpio-ISR .. triggered for STP0 or STP1 change.. no need to detect the cause
    stepper_signal_puffer[stepper_signal_w_pos & 0x7F] = ((bool_to_bit(gpio_get(GPIO_STP0))) | (bool_to_bit(gpio_get(GPIO_STP1))<<1));
    stepper_signal_w_pos++;
}


int main()
{
    stdio_init_all();

    // set interrupt on pin-change for STP0 and STP1 ...
    gpio_init(GPIO_STP0);
    gpio_init(GPIO_STP1);
    gpio_set_dir(GPIO_STP0, GPIO_IN);
    gpio_set_dir(GPIO_STP1, GPIO_IN);
    gpio_set_irq_enabled_with_callback(GPIO_STP0, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(GPIO_STP1, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);

    // // SPI initialisation. This example will use SPI at 1MHz.
    // spi_init(SPI_PORT, 1000*1000);
    // gpio_set_function(SPI_MISO, GPIO_FUNC_SPI);
    // gpio_set_function(SPI_CS,   GPIO_FUNC_SIO);
    // gpio_set_function(SPI_SCK,  GPIO_FUNC_SPI);
    // gpio_set_function(SPI_MOSI, GPIO_FUNC_SPI);
    

    // // gpio_put_masked(0xFF<<GPIO_PAPORT,value<<GPIO_PAPORT);

    // // Chip select is active-low, so we'll initialise it to a driven-high state
    // gpio_set_dir(SPI_CS, GPIO_OUT);
    // gpio_put(SPI_CS, 1);
    // // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    if (display_init())
    {
        display_clear();
        display_home();
    }

    FATFS fs;
    FRESULT fr = f_mount(&fs, "", 1);
    uint8_t retry_count = 3;

    while ((FR_OK != fr) && (retry_count > 0)) {
        display_setcursor(0, 0);
        display_string("f_mount error:");
        display_data(fr+'0');
        display_setcursor(0, 1);
        display_print(FRESULT_str(fr),0,16);
        display_setcursor(0, 2);
        display_print(FRESULT_str(fr),16,16);
        display_setcursor(0, 3);
        display_string("retrying ...");
        display_data(retry_count+'0');
        retry_count--;
        sleep_ms(2000);
        FRESULT fr = f_mount(&fs, "", 1);
        display_clear();
    }

    display_setcursor(0, 0);
    display_string("f_mount okay");

    sleep_ms(3000);

    display_clear();
    display_home();

    DIR dir_object;
    FILINFO dir_entry;

    uint8_t dsp_zeile = 0;

    fr = f_findfirst(&dir_object, &dir_entry, "", "*");

    while ((fr == FR_OK) && (dir_entry.fname[0]!=0))
    {
        display_setcursor(0, dsp_zeile);
        display_data(display_cursor_char);
        display_print(dir_entry.fname, 0, 12);
        dsp_zeile = (dsp_zeile+1)%4;
        if (0 == dsp_zeile)
        {
            sleep_ms(2000);
            display_clear();
        }
        fr = f_findnext(&dir_object, &dir_entry);
    }
    display_setcursor(0, dsp_zeile);
    display_string("--END--");

    sleep_ms(5000);

    // setup menus
    menu_init(&main_menu,     main_menu_entrys,     count_of(main_menu_entrys),     LCD_COLS, LCD_ROWS);
    menu_init(&image_menu,    image_menu_entrys,    count_of(image_menu_entrys),    LCD_COLS, LCD_ROWS);
    menu_init(&settings_menu, settings_menu_entrys, count_of(settings_menu_entrys), LCD_COLS, LCD_ROWS);
    menu_init(&info_menu,     info_menu_entrys,     count_of(info_menu_entrys),     LCD_COLS, LCD_ROWS);

    menu_set_root(&main_menu);
    // ----

    // uint8_t stepper;
    // stepper_signal_puffer[0]=212; // fake data 11010100
    // stepper_signal_w_pos++;



    while (true) {
        menu_refresh();

        //        printf("Hello, world!\n");
        // if (stepper_signal_r_pos != stepper_signal_w_pos)
        // {
        //     display_clear();
        //     display_setcursor(0, 0);
        //     stepper = stepper_signal_puffer[stepper_signal_r_pos & 0x7F] | stepper_signal_puffer[(stepper_signal_r_pos-1) & 0x7F]<<2;
        //     stepper_signal_r_pos++;

        //     for (int i=7; i>=0; --i)
        //     {
        //         display_data((stepper&(1<<i))?'1':'0');
        //     }
        // }
        sleep_ms(1000);
    }
}
