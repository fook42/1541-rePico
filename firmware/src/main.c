#include <stdio.h>
#include "pico/stdlib.h"
// #include "hardware/spi.h"
#include "hardware/i2c.h"
// #include "hardware/dma.h"
// #include "hardware/pio.h"
// #include "hardware/interp.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"

#include "quadrature_encoder.pio.h"

#include "pinout.h"

#define _EXTERN_

#include "main.h"
#include "version.h"
#include "display.h"
#include "lcd.h"
#include "oled.h"
#include "i2c.h"
#include "gcr.h"
#include "menu.h"
#include "mymenu.h"
#include "gui_constants.h"

#include "hw_config.h"
#include "f_util.h"
#include "ff.h"

#define START_MESSAGE_TIME 1500

FATFS       fs;
FRESULT     fr;
DIR         dir_object;
FILINFO     dir_entry;
FILINFO     file_entry;
FILINFO     fb_dir_entry[LCD_LINE_COUNT];

uint8_t stepper_signal_puffer[0x80]; // Ringpuffer für Stepper Signale (128 Bytes)
volatile uint8_t stepper_signal_r_pos = 0;
volatile uint8_t stepper_signal_w_pos = 0;
volatile uint8_t stepper_signal_time = 0;
volatile uint8_t stepper_signal = 0;

uint8_t akt_half_track = 100;

int new_value, delta, old_value = 0;
int last_value = -1, last_delta = -1;
PIO pio = pio0;
int pio_prg_offset;
const uint sm = 0;
uint8_t key3_irq_value = NO_KEY;

// -----------

void set_gui_mode(const uint8_t gui_mode);
uint8_t get_key_from_buffer(void);
void check_menu_events(const uint16_t menu_event);
void update_gui();
void show_start_message(void);

void filebrowser_update(uint8_t key_code);
void filebrowser_refresh();
uint16_t seek_to_dir_entry(uint16_t entry_num);
uint16_t get_dir_entry_count();


bool repeating_timer_callback(__unused struct repeating_timer *t) {
//    printf("Repeat at %lld\n", time_us_64());
    return true;
}



void gpio_callback(uint gpio, uint32_t events)
{
    if ((GPIO_STP0==gpio) || (GPIO_STP1==gpio))
    {
        // general gpio-ISR .. triggered for STP0 or STP1 change.. no need to detect the cause
        stepper_signal_puffer[stepper_signal_w_pos & 0x7F] = ((bool_to_bit(gpio_get(GPIO_STP0))) | (bool_to_bit(gpio_get(GPIO_STP1))<<1));
        stepper_signal_w_pos++;
    } else if (GPIO_BT3==gpio)
    {
        key3_irq_value=(events==GPIO_IRQ_EDGE_FALL)?KEY2_DOWN:KEY2_UP;
    }
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


    // --- Input ----

    gpio_init(GPIO_BT3);
    gpio_set_dir(GPIO_BT3, GPIO_IN);
    gpio_set_pulls(GPIO_BT3, true, false);
    gpio_set_irq_enabled(GPIO_BT3, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);


    // Base pin to connect the A phase of the encoder.
    // The B phase must be connected to the next pin
    const uint PIN_AB = GPIO_BT2; // GPIO26+27

    // we don't really need to keep the offset, as this program must be loaded
    // at offset 0
    pio_prg_offset = pio_add_program(pio, &quadrature_encoder_program);
    quadrature_encoder_program_init(pio, sm, PIN_AB, 500);

    // // gpio_put_masked(0xFF<<GPIO_PAPORT,value<<GPIO_PAPORT);

    struct repeating_timer timer;
    bool success;
    success = add_repeating_timer_us(timer0_values[0], repeating_timer_callback, NULL, &timer);

    if (success)
    {
        success = cancel_repeating_timer(&timer);
    }



    if (display_init())
    {
        display_clear();
        display_home();

    }
    
    show_start_message();

    fr = f_mount(&fs, "", 1);
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
        display_string("retrying ... ");
        display_data(retry_count+'0');
        retry_count--;
        sleep_ms(2000);
        FRESULT fr = f_mount(&fs, "", 1);
        display_clear();
    }

    display_setcursor(0, 0);
    display_string("f_mount okay");

    fb_dir_entry_count = get_dir_entry_count(); // open card, count entries on root level

    sleep_ms(3000);

    display_clear();
    display_home();

    uint8_t dsp_zeile = 0;

    // setup menus
    menu_init(&main_menu,     main_menu_entrys,     count_of(main_menu_entrys),     LCD_COLS, LCD_ROWS);
    menu_init(&image_menu,    image_menu_entrys,    count_of(image_menu_entrys),    LCD_COLS, LCD_ROWS);
    menu_init(&settings_menu, settings_menu_entrys, count_of(settings_menu_entrys), LCD_COLS, LCD_ROWS);
    menu_init(&info_menu,     info_menu_entrys,     count_of(info_menu_entrys),     LCD_COLS, LCD_ROWS);

    menu_set_root(&main_menu);
    // ----
    set_gui_mode(GUI_MENU_MODE);



    while (true) {
        update_gui();

        sleep_ms(100);
    }
}

////////////////////////////////////////////////////////////////////7
uint8_t get_key_from_buffer(void)
{
    uint8_t val;
    new_value = quadrature_encoder_get_count(pio, sm);
    if ((new_value - old_value)>0) {
        ++old_value;
        val = KEY1_DOWN;
    } else if ((old_value - new_value)>0) {
        --old_value;
        val = KEY0_DOWN;
    } else {
        val = key3_irq_value;
        key3_irq_value = NO_KEY;
    }
    return val;
}

void update_gui()
{
    static uint8_t old_half_track = 0;
    static uint8_t old_motor_status = 0;
    static uint16_t wait_counter0 = 0;
    uint8_t new_motor_status;
    uint8_t key_code = get_key_from_buffer();
    char byte_str[16];

    switch (current_gui_mode)
    {
    case GUI_INFO_MODE:

        if(KEY2_UP == key_code)
        {
            set_gui_mode(GUI_MENU_MODE);
        } else if(KEY2_TIMEOUT2 == key_code)
        {
            // exit_main = 0;
        }

        if(old_half_track != akt_half_track)
        {
            display_setcursor(disp_trackno_p);
            sprintf (byte_str,"%02d",akt_half_track >> 1);
            display_string(byte_str);
        }
        old_half_track = akt_half_track;

//        new_motor_status = get_motor_status();
        if(old_motor_status != new_motor_status)
        {
            old_motor_status = new_motor_status;
            display_setcursor(disp_motortxt_p);
            if(new_motor_status)
                display_string(disp_motor_on_s);
            else
                display_string(disp_motor_off_s);
        }

        /*
        if(is_image_mount)
        {
            //// Filename Scrolling

            ++wait_counter0;

            if((gui_current_line_offset > 0) && (wait_counter0 == 30000))
            {
                wait_counter0 = 0;

                if(0 == gui_line_scroll_end_begin_wait)
                {
                    // Es darf gescrollt werden
                    if(!gui_line_scroll_direction)
                    {
                        ++gui_line_scroll_pos;
                        if(gui_line_scroll_pos >= gui_current_line_offset)
                        {
                            gui_line_scroll_end_begin_wait = 6;
                            gui_line_scroll_direction = 1;
                        }
                    }
                    else
                    {
                        --gui_line_scroll_pos;
                        if(gui_line_scroll_pos == 0)
                        {
                            gui_line_scroll_end_begin_wait = 6;
                            gui_line_scroll_direction = 0;
                        }
                    }

                    display_setcursor(disp_scrollfilename_p);
                    display_print(image_filename,gui_line_scroll_pos,LCD_LINE_SIZE);
                } else {
                    --gui_line_scroll_end_begin_wait;
                }
            }
        }
        */
        break;

    case GUI_MENU_MODE:
        check_menu_events(menu_update(key_code));
        break;

    case GUI_FILE_BROWSER:
        filebrowser_update(key_code);
        break;

    default:
        break;
    }
}

void check_menu_events(const uint16_t menu_event)
{
    const uint8_t command = (uint8_t) ((menu_event >> 8) & 0xff);
    const uint8_t value = (uint8_t) (menu_event & 0xff);

    switch(command)
    {
        case MC_EXIT_MENU:
            set_gui_mode(GUI_INFO_MODE);
            break;

        case MC_SELECT_ENTRY:
            switch(value)
            {
                /// Main Menü

                /// Image Menü
                case M_LOAD_IMAGE:
                    set_gui_mode(GUI_FILE_BROWSER);
                    break;

                case M_UNLOAD_IMAGE:
                    //unmount_image();
                    set_gui_mode(GUI_INFO_MODE);
                    break;

                case M_WP_IMAGE:
                    // if(menu_get_entry_var1(&image_menu, M_WP_IMAGE))
                    // {
                    //     set_write_protection(1);
                    // } else {
                    //     set_write_protection(0);
                    // }
                    menu_refresh();
                    break;

                /// Settings Menü
                case M_RESTART:
                    // exit_main = 0;
                    menu_refresh();
                    break;

                /// Info Menü
                case M_VERSION_INFO:
                    //show_start_message();
                    menu_refresh();
                    break;

                case M_SDCARD_INFO:
                    //show_sdcard_info_message();
                    menu_refresh();
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }
}

/////////////////////////////////////////////////////////////////////

void set_gui_mode(const uint8_t gui_mode)
{
    current_gui_mode = gui_mode;
    switch(gui_mode)
    {
    case GUI_INFO_MODE:
        display_clear();

        display_setcursor(disp_tracktxt_p);
        display_string(disp_tracktxt_s);

        display_setcursor(disp_trackno_p);
        // sprintf (byte_str,"%02d",akt_half_track >> 1);
        // display_string(byte_str);

        display_setcursor(disp_motortxt_p);
        // if(get_motor_status())
            display_string(disp_motor_on_s);
        // else
        //     display_string(disp_motor_off_s);

        display_setcursor(disp_writeprottxt_p);
        // if(floppy_wp)
            display_string(disp_writeprot_on_s);
        // else
        //     display_string(disp_writeprot_off_s);

        display_setcursor(disp_scrollfilename_p);
        // if(is_image_mount)
        // {
        //     display_print(image_filename,0,LCD_LINE_SIZE);

        //     // Für Scrollenden Filename
        //     int8_t var = (int8_t)strlen(image_filename) - LCD_LINE_SIZE;
        //     if(var < 0)
        //         gui_current_line_offset = 0;
        //     else
        //         gui_current_line_offset = var;
        //     gui_line_scroll_pos = 0;
        //     gui_line_scroll_direction = 0;
        //     gui_line_scroll_end_begin_wait = 6;
        // } else {
            display_string(disp_nofilemounted_s);
        // }

        break;
    case GUI_MENU_MODE:
        menu_refresh();
        break;
    case GUI_FILE_BROWSER:
        filebrowser_refresh();
        break;
    default:
        break;
    }
}


void show_start_message(void)
{
    display_clear();
    display_setcursor(disp_versiontxt_p);
    display_string(disp_versiontxt_s);
    display_setcursor(disp_firmwaretxt_p);
    display_string(disp_firmwaretxt_s);
    display_string(VERSION);
    sleep_ms(START_MESSAGE_TIME);

    display_clear();
}

/////////////////////////////////////////////////////////////////////

void filebrowser_update(uint8_t key_code)
{
    switch (key_code)
    {
    case KEY0_DOWN:
        if(fb_cursor_pos > 0)
        {
            --fb_cursor_pos;
            filebrowser_refresh();
        }
        else
        {
            if(fb_window_pos > 0)
            {
                --fb_window_pos;
                filebrowser_refresh();
            }
        }
        break;
    case KEY1_DOWN:
        if((fb_cursor_pos < LCD_LINE_COUNT-1) && (fb_cursor_pos < fb_dir_entry_count-1))
        {
            ++fb_cursor_pos;
            filebrowser_refresh();
        }
        else
        {
            if(fb_window_pos < fb_dir_entry_count - LCD_LINE_COUNT)
            {
                ++fb_window_pos;
                filebrowser_refresh();
            }
        }
        break;
    case KEY2_UP:
        // stop_timer0();
        // no_byte_ready_send = 1;

        // close_disk_image(fd);

        if(fb_dir_entry[fb_cursor_pos].fattrib & AM_DIR)
        {
            // Eintrag ist ein Verzeichnis
        //     change_dir(fb_dir_entry[fb_cursor_pos].long_name);
            fb_cursor_pos = 0;
            fb_window_pos = 0;
            filebrowser_refresh();
            return;
        }

        // fd = open_disk_image(fs, &fb_dir_entry[fb_cursor_pos], &akt_image_type);

        // if(akt_image_type == UNDEF_IMAGE)
        // {
        //     display_clear();
        //     display_setcursor(disp_unsupportedimg_p);
        //     display_string(disp_unsupportedimg_s);
        //     _delay_ms(1000);
        // }

        filebrowser_refresh();

        // if(!fd)
        // {
        //     is_image_mount = 0;
        //     return ;
        // }

        strcpy(image_filename, fb_dir_entry[fb_cursor_pos].fname);

        // read_disk_track(fd,akt_image_type,akt_half_track>>1,gcr_track, &gcr_track_length);
        // akt_track_pos = 0;

        // no_byte_ready_send = 0;
        // start_timer0();

        // is_image_mount = 1;
        // send_disk_change();

        // menu_set_entry_var1(&image_menu, M_WP_IMAGE, floppy_wp);

        set_gui_mode(GUI_INFO_MODE);
        break;
    case KEY2_TIMEOUT1:
        set_gui_mode(GUI_MENU_MODE);
        break;
    }

    //// Filename Scrolling
    static uint16_t wait_counter0;
    ++wait_counter0;

    if((fb_current_line_offset > 0) && (wait_counter0 == 30000))
    {
        wait_counter0 = 0;

        if(0 == fb_line_scroll_end_begin_wait)
        {
            // Es darf gescrollt werden
            if(!fb_line_scroll_direction)
            {
                ++fb_line_scroll_pos;
                if(fb_line_scroll_pos >= fb_current_line_offset)
                {
                    fb_line_scroll_end_begin_wait = 6;
                    fb_line_scroll_direction = 1;
                }
            }
            else
            {
                --fb_line_scroll_pos;
                if(fb_line_scroll_pos == 0)
                {
                    fb_line_scroll_end_begin_wait = 6;
                    fb_line_scroll_direction = 0;
                }
            }

            display_setcursor(2,fb_cursor_pos);
            display_print(fb_dir_entry[fb_cursor_pos].fname,fb_line_scroll_pos, LCD_LINE_SIZE-3 );
        }
        else
        {
            --fb_line_scroll_end_begin_wait;
        }
    }
}

/////////////////////////////////////////////////////////////////////

void filebrowser_refresh()
{
    display_clear();

    // fr = f_opendir(&dir_object, path);  /* Open the target directory */
    // if (fr == FR_OK) {
    // }
    seek_to_dir_entry(fb_window_pos);

    uint8_t i=0;

    while((i<LCD_LINE_COUNT) && ((fb_window_pos + i) < fb_dir_entry_count))
    {
        fr = f_readdir(&dir_object, &(fb_dir_entry[i]));
        if((fb_dir_entry[i].fname[0]==0) || (fr != FR_OK))
        {
            break;
        }

        display_setcursor(1,i);
        if(fb_dir_entry[i].fattrib & AM_DIR)
        {
            display_data(display_dir_char);
        } else {
            display_data(' ');
        }

        display_print(fb_dir_entry[i].fname, 0, LCD_LINE_SIZE-3);

        i++;
    }

    display_setcursor(0, fb_cursor_pos);
    display_data(display_cursor_char);


    if(fb_window_pos > 0)
    {
        display_setcursor(LCD_LINE_SIZE-1,0);
        display_data(display_more_top_char);
    }

    if((fb_window_pos + LCD_LINE_COUNT) < fb_dir_entry_count)
    {
        display_setcursor(LCD_LINE_SIZE-1, LCD_LINE_COUNT-1);
        display_data(display_more_down_char);
    }

    // Für Scrollenden Filename
    int8_t var = (int8_t)strlen(fb_dir_entry[fb_cursor_pos].fname) - (LCD_LINE_SIZE-3);
    if(var < 0)
        fb_current_line_offset = 0;
    else
        fb_current_line_offset = var;

    fb_line_scroll_pos = 0;
    fb_line_scroll_direction = 0;
    fb_line_scroll_end_begin_wait = 6;
}

/////////////////////////////////////////////////////////////////////

uint16_t get_dir_entry_count()
{
    uint16_t entry_count = 0;

//    f_closedir(&dir_object);

    char pattern[] = {"*"};
    char path[] = {""};

    dir_object.pat = pattern;           /* Save pointer to pattern string */
    fr = f_opendir(&dir_object, path);  /* Open the target directory */
    if (FR_OK == fr)
    {
        while(FR_OK == f_readdir(&dir_object, &dir_entry))
        {
            if(0 == dir_entry.fname[0])
            {
                break;
            }
            if(!(dir_entry.fattrib & (AM_SYS | AM_HID )))
            {
                entry_count++;
            }
        }
    }
    return entry_count;
}

/////////////////////////////////////////////////////////////////////

uint16_t seek_to_dir_entry(uint16_t entry_num)
{
    uint16_t entry_count = 0;

    //        fat_reset_dir(dd);
    f_closedir(&dir_object);

    char pattern[] = {"*"};
    char path[] = {""};

    dir_object.pat = pattern;           /* Save pointer to pattern string */
    fr = f_opendir(&dir_object, path);  /* Open the target directory */

    if((FR_OK == fr) || (entry_num != 0))
    {
        while(FR_OK == f_readdir(&dir_object, &dir_entry) && (entry_count < (entry_num-1)))
        {
            if(0 == dir_entry.fname[0])
            {
                break;
            }
            if(!(dir_entry.fattrib & (AM_SYS | AM_HID )))
            {
                entry_count++;
            }
        }
    }
    return entry_count;
}
