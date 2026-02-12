/**********************************
 * header - main routines, defines, variables
 *
 * Author: F00K42
 * Last change: 2026/02/12
***********************************/
#include "hw_config.h"
#include "f_util.h"
#include "ff.h"

//
#define INPUT_DEBOUNCE_TIME (200)
alarm_id_t input_debounce_alarm = 0;

/////
#define LCD_LINE_COUNT  (LCD_ROWS)
#define LCD_LINE_SIZE   (LCD_COLS)

// Spur auf dem der Lesekopf beim Start/Reset stehen soll
// Track 17 --> Directory (Tracks 0..41 !)
#define DIRECTORY_TRACK (17)
#define INIT_TRACK      DIRECTORY_TRACK

#define MAX_TRACKS      (42)
// functions
int64_t input_debounce_callback(alarm_id_t id, void *user_data);

void check_stepper_signals(void);
void check_motor_signal(void);

void init_key_inputs(void);
uint8_t get_key_from_buffer(void);
void update_gui(void);
void check_menu_events(const uint16_t menu_event);
void set_gui_mode(const uint8_t gui_mode);
void filebrowser_update(uint8_t key_code);
void filebrowser_refresh(void);

uint16_t get_dir_entry_count(void);
uint16_t seek_to_dir_entry(uint16_t entry_num);

void show_start_message(void);

void init_stepper(void);
void stepper_inc(void);
void stepper_dec(void);
void init_motor(void);
void init_control_signals(void);
void init_soe_gatearray(void);

void open_disk_image(FIL* fd, FILINFO *file_entry, uint8_t* image_type);
void close_disk_image(FIL* fd);
void open_g64_image(FIL* fd);
void open_d64_image(FIL* fd);
int8_t read_disk(FIL* fd, const int image_type);
int8_t write_disk(const int image_type);


void unmount_image(void);
void init_writeprot(void);
void send_disk_change(void);

bool repeating_timer_callback(__unused struct repeating_timer *t);
void init_bytetimer(void);
void start_bytetimer(uint8_t half_track);
void stop_bytetimer(void);

int64_t steppertimer_callback(alarm_id_t id, void *user_data);
void start_stepper_timer(void);

/////////////

// Filesystem-variables:
FATFS       fs;             // filesystem handle - only created once
FRESULT     fr;             // general purpos result variable
DIR         dir_object;
FIL         fd;             // file descriptor for every open file
FILINFO     dir_entry;
FILINFO     file_entry;
FILINFO     fb_dir_entry[LCD_LINE_COUNT];
//

volatile uint16_t akt_track_pos = 0;

uint8_t selected_track;             // this holds the selected tracknumber
volatile uint8_t akt_half_track;    // this will be the one to be transfered
uint8_t old_half_track;

const uint16_t d64_track_offset[MAX_TRACKS] = { 0x0000,0x0015,0x002A,0x003F,0x0054,0x0069,0x007E,0x0093,
                                                0x00A8,0x00BD,0x00D2,0x00E7,0x00FC,0x0111,0x0126,0x013B,
                                                0x0150,0x0165,0x0178,0x018B,0x019E,0x01B1,0x01C4,0x01D7,
                                                0x01EA,0x01FC,0x020E,0x0220,0x0232,0x0244,0x0256,0x0267,
                                                0x0278,0x0289,0x029A,0x02AB,0x02BC,0x02CD,0x02DE,0x02EF,
                                                0x0300,0x0311 };

const uint8_t d64_sector_count[4] = {21,    //Spuren 1-17
                                     19,    //Spuren 18-24
                                     18,    //Spuren 25-30
                                     17};   //Spuren 31-42

const uint8_t d64_track_zone[MAX_TRACKS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  //Spuren 1-17
                                            1,1,1,1,1,1,1,                      //Spuren 18-24
                                            2,2,2,2,2,2,                        //Spuren 25-30
                                            3,3,3,3,3,3,3,3,3,3,3,3};           //Spuren 31-42

#define D64_SECTOR_SIZE (256)
uint8_t d64_sector_puffer[21*D64_SECTOR_SIZE+5];


#define GCR_SYNCMARK    (0xFF)
// *** definition of GCR-structure
// see: https://ist.uwaterloo.ca/~schepers/formats/G64.TXT

#define G64_VERSION     (0x00)
#define G64_TRACKCOUNT  MAX_TRACKS
#define G64_TRACKSIZE   (7928)  // = 0x1ef8

const uint8_t g64_head[] = {'G','C','R','-','1','5','4','1', G64_VERSION, G64_TRACKCOUNT*2, (G64_TRACKSIZE&0xff), ((G64_TRACKSIZE>>8)&0xff)};
uint32_t g64_jumptable[2*G64_TRACKCOUNT];
uint32_t g64_speedtable[2*G64_TRACKCOUNT];

const uint8_t g64_speedzones[] = { 3, 2, 1, 0};

#define G64_HEADERSIZE (sizeof(g64_head)+sizeof(g64_jumptable)+sizeof(g64_speedtable)) // should be 684 bytes

uint16_t g64_tracklen[G64_TRACKCOUNT];

// enough space to store 42 tracks, each 7928 bytes byte length
uint8_t g64_tracks[G64_TRACKCOUNT][G64_TRACKSIZE];

// timer definition

struct repeating_timer bytetimer;
// Originale Bitraten
//Zone 0: 8000000/26 = 307692 Hz    (ByteReady 38461.5 Hz)
//Zone 1: 8000000/28 = 285714 Hz    (ByteReady 35714.25 Hz)
//Zone 2: 8000000/30 = 266667 Hz    (ByteReady 33333.375 Hz)
//Zone 3: 8000000/32 = 250000 Hz    (ByteReady 31250 Hz)

//Höhere Werte verlangsammen die Bitrate (=us delay-values between bytes)
//const int64_t bytetimer_values[4] = {26, 28, 30, 32};
const int64_t bytetimer_values[4] = {26, 28, 30, 32};

// Zeit die nach der letzten Stepperaktivität vergehen muss, um einen neuen Track von SD Karte zu laden
// (1541 Original Rom schaltet STP1 alle 15ms)
// Default 15
#define STEPPER_DELAY_TIME (5)

const uint8_t d64_sector_gap[4] = {12, 21, 16, 13}; // von GPZ Code übernommen imggen
#define HEADER_GAP_BYTES (9)

char image_filename[256]; //Maximal 256 Zeichen

uint8_t current_gui_mode;

uint8_t gui_current_line_offset;         // >0 dann ist der Name länger als die maximale Anzeigelaenge
uint8_t gui_line_scroll_pos;             // Kann zwischen 0 und fb_current_line_offset liegen
uint8_t gui_line_scroll_direction;       // Richtung des Scrollings
uint8_t gui_line_scroll_end_begin_wait;

// Alles für den FilebrowserSS
uint16_t fb_dir_entry_count = 0;         // Anzahl der Einträge im aktuellen Direktory
uint8_t fb_cursor_pos = 0;               // Position des Cursors auf dem LCD Display
uint8_t fb_window_pos = 0;               // Position des Anzeigebereichs innerhablb der Menüeinträge

uint8_t fb_current_line_offset = 0;         // >0 dann ist der Name länger als die maximale Anzeigelaenge
uint8_t fb_line_scroll_pos = 0;             // Kann zwischen 0 und fb_current_line_offset liegen
uint8_t fb_line_scroll_direction = 0;       // Richtung des Scrollings
uint8_t fb_line_scroll_end_begin_wait = 10;

uint8_t id1 = 0;    // id1 and id2 identify a floppy-disk
uint8_t id2 = 0;    // - these need to change to .. signal a disk-change or after a format...

enum {UNDEF_IMAGE, G64_IMAGE, D64_IMAGE};

// floppydisk emulation
uint8_t akt_image_type = UNDEF_IMAGE;     // 0=kein Image, 1=G64, 2=D64
bool is_image_mount;

bool floppy_wp = true;  // Hier wird der aktuelle WriteProtection Zustand gespeichert
                        // false=Nicht Schreibgeschützt , true=Schreibgeschützt

#define set_byte_ready()    gpio_set_dir(GPIO_BRDY,GPIO_IN)    // HiZ
#define clear_byte_ready()  {gpio_set_dir(GPIO_BRDY,GPIO_OUT);gpio_put(GPIO_BRDY,false);}   // pull low

#define get_soe_status()    gpio_get(GPIO_SOE)

#define get_so_status()     gpio_get(GPIO_OE)

#define set_soe_gatearray()     gpio_put(GPIO_SOE_GA,true)
#define clear_soe_gatearray()   gpio_put(GPIO_SOE_GA,false)

// WPS will be generated via inverter 74ls04 on 1541*-mainboard
// ... thus we send the inverse here (clear_wps = "1" on WPS)
#define clear_wps()     gpio_set_dir(GPIO_WPS,GPIO_IN)    // HiZ
#define set_wps()       {gpio_set_dir(GPIO_WPS,GPIO_OUT);gpio_put(GPIO_WPS,false);}   // pull low

#define get_motor_status()  gpio_get(GPIO_MTR)

#define set_sync()          gpio_put(GPIO_SYNC,true)
#define clear_sync()        gpio_put(GPIO_SYNC,false)

#define out_gcr_byte(gcr_byte)  gpio_put_masked(PAPORT_MASK,gcr_byte<<GPIO_PAPORT)
#define in_gcr_byte()       (gpio_get_all()&PAPORT_MASK)>>GPIO_PAPORT

#define enable_write_protection()   {clear_wps();floppy_wp=true;}
#define disable_write_protection()  {set_wps();floppy_wp=false;}


uint8_t stepper_signal_puffer[256]; // Ringpuffer für Stepper Signale (256 Bytes)
volatile uint8_t stepper_signal_r_pos = 0;
volatile uint8_t stepper_signal_w_pos = 0;
volatile uint16_t stepper_signal_time = 0;  // extended to 16bit to cover bigger step-wait times
volatile uint8_t stepper_signal = 0;


alarm_id_t stepper_alarm = 0;

volatile bool diskdata_modified  = false;
volatile bool send_byte_ready    = true;


// debug
volatile uint8_t gcr_sector = 0;
volatile uint8_t gcr_track = 0;

volatile bool change_track = false;

