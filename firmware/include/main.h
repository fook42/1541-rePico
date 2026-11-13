/**********************************
 * header - main routines, defines, variables
 *
 * Author: F00K42
 * Last change: 2026/07/24
***********************************/

#include "hw_config.h"
#include "f_util.h"
#include "ff.h"
#include "lcd.h"
#include "globals.h"
#include "mymenu.h"

// functions
int64_t input_debounce_callback(alarm_id_t id, void *user_data);

void init_key_inputs(void);
uint8_t get_key_from_buffer(void);

/////////////
#define set_byte_ready()    gpio_set_dir(GPIO_BRDY,GPIO_IN)     // HiZ
#define clear_byte_ready()  {gpio_set_dir(GPIO_BRDY,GPIO_OUT);gpio_put(GPIO_BRDY,false);}   // pull low

#define get_soe_status()    gpio_get(GPIO_SOE)

#define get_so_status()     gpio_get(GPIO_OE)

#define set_soe_gatearray()     gpio_put(GPIO_SOE_GA,true)
#define clear_soe_gatearray()   gpio_put(GPIO_SOE_GA,false)

// WPS will be generated via inverter 74ls04 on 1541*-mainboard
// ... thus we send the inverse here (clear_wps = "1" on WPS)
#define clear_wps()          gpio_set_dir(GPIO_WPS,GPIO_IN)     // HiZ
#define set_wps()           {gpio_set_dir(GPIO_WPS,GPIO_OUT);gpio_put(GPIO_WPS,false);}     // pull low

#define get_motor_status()  gpio_get(GPIO_MTR)

#define set_sync()          gpio_put(GPIO_SYNC,true)
#define clear_sync()        gpio_put(GPIO_SYNC,false)

#define out_gcr_byte(gcr_byte)  gpio_put_masked(PAPORT_MASK,gcr_byte<<GPIO_PAPORT)
#define in_gcr_byte()       (gpio_get_all()&PAPORT_MASK)>>GPIO_PAPORT

#define enable_write_protection()   {clear_wps();floppy_wp=true;}
#define disable_write_protection()  {set_wps();floppy_wp=false;}


// Filesystem-variables:
extern FATFS       fs;             // filesystem handle - only created once
extern DIR         dir_object;
extern FIL         fd;             // file descriptor for every open file
extern FILINFO     fb_dir_entry[LCD_LINE_COUNT];
//
//
#define ROTARY_DEBOUNCE_TIME    (200)
#define BUTTON_DEBOUNCE_TIME    (100)

extern alarm_id_t input_debounce_alarm;
// timer_t key_longpress_timer;

extern volatile uint16_t akt_track_pos;

extern uint8_t num_max_tracks;
extern uint8_t selected_track;     // this holds the selected tracknumber
extern volatile uint8_t akt_half_track;     // this will be the one to be transfered
extern uint8_t old_half_track;

// timer definition

extern struct repeating_timer bytetimer;


extern char image_filename[256]; //Maximal 256 Zeichen
extern char current_path[512];

extern volatile uint8_t irq_key_value;
extern volatile bool input_block;

extern uint64_t key2_down_time;
extern uint8_t current_gui_mode;

extern uint8_t gui_current_line_offset;         // >0 dann ist der Name länger als die maximale Anzeigelaenge
extern uint8_t gui_line_scroll_pos;             // Kann zwischen 0 und fb_current_line_offset liegen
extern uint8_t gui_line_scroll_direction;       // Richtung des Scrollings
extern uint8_t gui_line_scroll_end_begin_wait;

// Alles für den FilebrowserSS
extern uint16_t fb_dir_entry_count;         // Anzahl der Einträge im aktuellen Direktory
extern uint8_t fb_cursor_pos;               // Position des Cursors auf dem LCD Display
extern uint8_t fb_window_pos;               // Position des Anzeigebereichs innerhablb der Menüeinträge

extern uint8_t fb_current_line_offset;         // >0 dann ist der Name länger als die maximale Anzeigelaenge
extern uint8_t fb_line_scroll_pos;             // Kann zwischen 0 und fb_current_line_offset liegen
extern uint8_t fb_line_scroll_direction;       // Richtung des Scrollings
extern uint8_t fb_line_scroll_end_begin_wait;


// floppydisk emulation
extern uint8_t akt_image_type;     // 0=kein Image, 1=G64, 2=D64, 3=Selector
extern bool is_image_mount;

extern bool floppy_wp;  // Hier wird der aktuelle WriteProtection Zustand gespeichert
                        // false=Nicht Schreibgeschützt , true=Schreibgeschützt

#define STEP_MIN_TIME (2000)    // us between 2 step changes.

extern uint8_t stepper_signal_puffer[256]; // Ringpuffer für Stepper Signale (256 Bytes)
extern volatile uint8_t stepper_signal_r_pos;
extern volatile uint8_t stepper_signal_w_pos;

extern alarm_id_t stepper_alarm;

extern volatile bool track_is_written;
extern volatile bool send_byte_ready;

extern volatile uint8_t  track_write_nr;
extern volatile uint16_t track_write_pos;
