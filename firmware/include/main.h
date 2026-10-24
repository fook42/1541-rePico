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

FRESULT mount_sdcard(const char* mount_path);
FRESULT umount_sdcard(void);
void show_fs_error(FRESULT error_code);

void check_stepper_signals(void);
void check_motor_signal(void);

void init_key_inputs(void);
uint8_t get_key_from_buffer(void);
void show_longpress(void);
void update_gui(void);
void check_menu_events(const uint16_t menu_event);
void set_gui_mode(const uint8_t gui_mode);
void filebrowser_update(uint8_t key_code);
void filebrowser_refresh(void);

void infomode_update(void);

void handle_menu_image(void);
void insert_menu_image(char* menu_path, const uint8_t max_blocks);

uint16_t get_dir_entry_count(const char* entrycount_path);
uint16_t seek_to_dir_entry(uint16_t entry_num, const char* seek_path);

void show_start_message(void);
void show_sdcard_info_message(void);

void init_stepper(void);
void stepper_inc(void);
void stepper_dec(void);
void init_motor(void);
void init_control_signals(void);
void init_soe_gatearray(void);

uint8_t open_dir_entry(FILINFO od_file_entry);

uint8_t open_disk_image(FIL* fd, FILINFO *file_entry);
void close_disk_image(FIL* fd);
void unmount_image(void);

void init_writeprot(void);
void send_disk_change(void);

void init_sound(void);

bool repeating_timer_callback(__unused struct repeating_timer *t);
void init_bytetimer(void);
void start_bytetimer(uint8_t half_track);
void stop_bytetimer(void);

int64_t steppertimer_callback(alarm_id_t id, void *user_data);
void start_stepper_timer(void);

/////////////
#define set_byte_ready()    gpio_set_dir(GPIO_BRDY,GPIO_IN)    // HiZ
#define clear_byte_ready()  {gpio_set_dir(GPIO_BRDY,GPIO_OUT);gpio_put(GPIO_BRDY,false);}   // auf Ground ziehen

#define get_soe_status()    gpio_get(GPIO_SOE)

#define get_so_status()     gpio_get(GPIO_OE)

#define set_soe_gatearray()     gpio_put(GPIO_SOE_GA,true)
#define clear_soe_gatearray()   gpio_put(GPIO_SOE_GA,false)

// WPS will be generated via inverter 74ls04 on 1541*-mainboard
// ... thus we send the inverse here (clear_wps = "1" on WPS)
#define clear_wps()          gpio_set_dir(GPIO_WPS,GPIO_IN)    // HiZ
#define set_wps()           {gpio_set_dir(GPIO_WPS,GPIO_OUT);gpio_put(GPIO_WPS,false);}   // pull low

#define get_motor_status()  gpio_get(GPIO_MTR)

#define set_sync()          gpio_put(GPIO_SYNC,true)
#define clear_sync()        gpio_put(GPIO_SYNC,false)

#define out_gcr_byte(gcr_byte)  gpio_put_masked(PAPORT_MASK,gcr_byte<<GPIO_PAPORT)
#define in_gcr_byte()       (gpio_get_all()&PAPORT_MASK)>>GPIO_PAPORT

#define enable_write_protection()   {clear_wps();floppy_wp=true;}
#define disable_write_protection()  {set_wps();floppy_wp=false;}

#if PCB_VERSION>=17
#define turn_sound_on()     gpio_put(GPIO_SND,true)
#define turn_sound_off()    gpio_put(GPIO_SND,false)
#else
#define turn_sound_on()     {}
#define turn_sound_off()    {}
#endif

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
