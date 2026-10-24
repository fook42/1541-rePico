/////////////////////////////////////////////////
// 1541-rePico
/////////////////////////////////////////////////
// author: F00K42
// last changed: 2026/07/24
// repo: https://github.com/fook42/1541-rePico
/////////////////////////////////////////////////


#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include <pico/multicore.h>


#include "pinout.h"

#define _EXTERN_

#include "version.h"
#include "display.h"
#include "lcd.h"
#include "oled.h"
#include "i2c.h"
#include "gcr.h"
#include "menu.h"
#include "mymenu.h"
#include "gui_constants.h"
#include "globals.h"
#include "rw_routines.h"
#include "menu_image.h"

#include "hw_config.h"
#include "f_util.h"
#include "ff.h"

#include "main.h"
#include "drive.h"
#include "files.h"
#include "gui.h"

#define START_MESSAGE_TIME 1500

MENU_STRUCT main_menu;
MENU_STRUCT image_menu;
MENU_STRUCT settings_menu;
MENU_STRUCT info_menu;

MENU_ENTRY main_menu_entrys[] = {{"Disk Menu", M_IMAGE,ENTRY_MENU,   0,&image_menu},
                                 {"Settings",  M_SETTINGS,ENTRY_MENU,0,&settings_menu},
                                 {"Info",      M_INFO,ENTRY_MENU,    0,&info_menu}};

MENU_ENTRY image_menu_entrys[] = {{"..",           M_BACK_IMAGE,ENTRY_TO_PARENT},
                                  {"Load Selector",M_MENU_IMAGE},
                                  {"Load Image",   M_LOAD_IMAGE},
                                  {"Save Image",   M_SAVE_IMAGE},
                                  {"Unload Image", M_UNLOAD_IMAGE},
                                  {"ReInsert Disk",M_RELOAD_DISK},
                                  {"WriteProt.",   M_WP_IMAGE, ENTRY_ONOFF, 1},
                                  {"New Image",    M_NEW_IMAGE}};

MENU_ENTRY settings_menu_entrys[] = {{"..",      M_BACK_SETTINGS,ENTRY_TO_PARENT},
                                     {"Restart", M_RESTART}};

MENU_ENTRY info_menu_entrys[] = {{"..",           M_BACK_INFO,ENTRY_TO_PARENT},
                                 {"Version",      M_VERSION_INFO},
                                 {"SD Card Info", M_SDCARD_INFO}};

volatile uint8_t irq_key_value = NO_KEY;

volatile bool input_block = false;

uint64_t key2_down_time=0;
uint8_t num_max_tracks;
uint16_t selected_image_nr = 0xFFFF;

FATFS       fs;             // filesystem handle - only created once
DIR         dir_object;
FIL         fd;             // file descriptor for every open file
FILINFO     fb_dir_entry[LCD_LINE_COUNT];

alarm_id_t input_debounce_alarm = 0;
volatile uint16_t akt_track_pos = 0;
uint8_t selected_track;     // this holds the selected tracknumber
volatile uint8_t akt_half_track;     // this will be the one to be transfered
uint8_t old_half_track;
struct repeating_timer bytetimer;
char image_filename[256]; //Maximal 256 Zeichen
char current_path[512];
uint8_t current_gui_mode;
uint8_t gui_current_line_offset = 0;         // >0 dann ist der Name länger als die maximale Anzeigelaenge
uint8_t gui_line_scroll_pos = 0;             // Kann zwischen 0 und fb_current_line_offset liegen
uint8_t gui_line_scroll_direction = 0;       // Richtung des Scrollings
uint8_t gui_line_scroll_end_begin_wait = 0;
uint16_t fb_dir_entry_count = 0;         // Anzahl der Einträge im aktuellen Direktory
uint8_t fb_cursor_pos = 0;               // Position des Cursors auf dem LCD Display
uint8_t fb_window_pos = 0;               // Position des Anzeigebereichs innerhablb der Menüeinträge
uint8_t fb_current_line_offset = 0;         // >0 dann ist der Name länger als die maximale Anzeigelaenge
uint8_t fb_line_scroll_pos = 0;             // Kann zwischen 0 und fb_current_line_offset liegen
uint8_t fb_line_scroll_direction = 0;       // Richtung des Scrollings
uint8_t fb_line_scroll_end_begin_wait = 10;
uint8_t akt_image_type = UNDEF_IMAGE;     // 0=kein Image, 1=G64, 2=D64, 3=Selector
bool is_image_mount = false;
bool floppy_wp = true;  // Hier wird der aktuelle WriteProtection Zustand gespeichert
uint8_t stepper_signal_puffer[256]; // Ringpuffer für Stepper Signale (256 Bytes)
volatile uint8_t stepper_signal_r_pos = 0;
volatile uint8_t stepper_signal_w_pos = 0;
alarm_id_t stepper_alarm = 0;
volatile bool track_is_written   = false;
volatile bool send_byte_ready    = true;
volatile uint8_t  track_write_nr = 0;
volatile uint16_t track_write_pos = 0;

// ---------------------------------------------------------------

int64_t input_debounce_callback(alarm_id_t id, void *user_data)
{
    (void) id;
    (void) user_data;

    input_block = false;
    return 0;
}

void gpio_callback(uint gpio, uint32_t events)
{
    if ((GPIO_STP0==gpio) || (GPIO_STP1==gpio))
    {
        static uint64_t last_int;
        // general gpio-ISR .. triggered for STP0 or STP1 change.. no need to detect the cause
        if ((time_us_64()-last_int) > STEP_MIN_TIME)
        {
            stepper_signal_puffer[stepper_signal_w_pos] = ((bool_to_bit(gpio_get(GPIO_STP0))<<1) | (bool_to_bit(gpio_get(GPIO_STP1))));
            stepper_signal_w_pos++;
        }
        last_int = time_us_64();
    } else {
        if ((NO_KEY == irq_key_value) && (false == input_block))
        {
            if (GPIO_BT3==gpio)
            {
                input_block = true;
                if (events==GPIO_IRQ_EDGE_FALL)
                {
                    irq_key_value = KEY2_DOWN;
                    input_debounce_alarm = add_alarm_in_ms(BUTTON_DEBOUNCE_TIME, input_debounce_callback, NULL, false);
                    key2_down_time = time_us_64();
                } else {
                    uint64_t now_time = time_us_64();
                    if (key2_down_time > now_time)
                    {
                        key2_down_time -= (now_time+1);
                        now_time = ((uint64_t)-1);
                    }

                    if ((now_time-key2_down_time) > TIMEOUT2_KEY2)
                    { irq_key_value = KEY2_TIMEOUT2; }
                    else if ((now_time-key2_down_time) > TIMEOUT1_KEY2)
                    { irq_key_value = KEY2_TIMEOUT1; }
                    else
                    { irq_key_value = KEY2_UP; }

                    key2_down_time = now_time;
                    input_debounce_alarm = add_alarm_in_ms(BUTTON_DEBOUNCE_TIME, input_debounce_callback, NULL, false);
                }
            } else if (GPIO_BT1==gpio)
            {
                input_block = true;
                if (gpio_get(GPIO_BT2))
                {
                    irq_key_value = KEY1_DOWN;
                } else {
                    irq_key_value = KEY0_DOWN;
                }
                input_debounce_alarm = add_alarm_in_ms(ROTARY_DEBOUNCE_TIME, input_debounce_callback, NULL, false);
            }
        }
    }
}

// ---------------------------------------------------------------
void start_gui_core(void){

    multicore_lockout_victim_init();
    while (true) {
        update_gui();     
    }

}

// ---------------------------------------------------------------

int main()
{
    stdio_init_all();

    sleep_ms(500);

    printf("1541repico boot\n");

    if (display_init())
    {
        display_home();
    }

    show_start_message();

    // init input-keys
    init_key_inputs();

    // Stepper Initialisieren
    init_stepper();

    // Motor Initialisieren
    init_motor();

    // Steursignale BYTE_READY, SYNC und SOE Initialisieren
    init_control_signals();

    init_soe_gatearray();
    clear_soe_gatearray();

    init_bytetimer();

    init_writeprot();
    disable_write_protection();

#if PCB_VERSION>=17
    // Enable Sound-Output-Pin
    init_sound();
#endif

    // setup menus
    menu_init(&main_menu,     main_menu_entrys,     count_of(main_menu_entrys),     LCD_LINE_SIZE, LCD_LINE_COUNT);
    menu_init(&image_menu,    image_menu_entrys,    count_of(image_menu_entrys),    LCD_LINE_SIZE, LCD_LINE_COUNT);
    menu_init(&settings_menu, settings_menu_entrys, count_of(settings_menu_entrys), LCD_LINE_SIZE, LCD_LINE_COUNT);
    menu_init(&info_menu,     info_menu_entrys,     count_of(info_menu_entrys),     LCD_LINE_SIZE, LCD_LINE_COUNT);

    menu_set_root(&main_menu);
    // ----

    sleep_ms(START_MESSAGE_TIME);

    display_clear();
    display_home();

    set_gui_mode(GUI_SELECTOR);

    // set root of sd-card as current path
    current_path[0]='/';
    current_path[1]=0;

    multicore_launch_core1( start_gui_core );
    while (true) {
        check_stepper_signals();
        
    }
}
/////////////////////////////////////////////////////////////////////
void init_key_inputs(void)
{
    gpio_init(GPIO_BT1);
    gpio_init(GPIO_BT2);
    gpio_init(GPIO_BT3);
    gpio_set_dir(GPIO_BT1, GPIO_IN);
    gpio_set_dir(GPIO_BT2, GPIO_IN);
    gpio_set_dir(GPIO_BT3, GPIO_IN);
    gpio_set_pulls(GPIO_BT1, true, false);
    gpio_set_pulls(GPIO_BT2, true, false);
    gpio_set_pulls(GPIO_BT3, true, false);
    gpio_set_irq_enabled_with_callback(GPIO_BT1, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(GPIO_BT3, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
}

uint8_t get_key_from_buffer(void)
{
    uint8_t val;

    val = irq_key_value;    // get last detected key
    irq_key_value = NO_KEY; // reset last state - ready for more

    return val;
}

