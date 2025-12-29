/////
#define LCD_LINE_COUNT  (LCD_ROWS)
#define LCD_LINE_SIZE   (LCD_COLS)


const uint16_t d64_track_offset[41] = {0,0x0000,0x0015,0x002A,0x003F,0x0054,0x0069,0x007E,0x0093,
                                         0x00A8,0x00BD,0x00D2,0x00E7,0x00FC,0x0111,0x0126,0x013B,
                                         0x0150,0x0165,0x0178,0x018B,0x019E,0x01B1,0x01C4,0x01D7,
                                         0x01EA,0x01FC,0x020E,0x0220,0x0232,0x0244,0x0256,0x0267,
                                         0x0278,0x0289,0x029A,0x02AB,0x02BC,0x02CD,0x02DE,0x02EF};

const uint8_t d64_sector_count[4] = {21,        //Spuren 1-17
                                     19,        //Spuren 18-24
                                     18,        //Spuren 25-30
                                     17};       //Spuren 31-40

const uint8_t d64_track_zone[41] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                    //Spuren 1-17
                               1,1,1,1,1,1,1,                                               //Spuren 18-24
                               2,2,2,2,2,2,                                                 //Spuren 25-30
                               3,3,3,3,3,3,3,3,3,3};                                        //Spuren 31-40

// Originale Bitraten
//Zone 0: 8000000/26 = 307692 Hz    (ByteReady 38461.5 Hz)
//Zone 1: 8000000/28 = 285714 Hz    (ByteReady 35714.25 Hz)
//Zone 2: 8000000/30 = 266667 Hz    (ByteReady 33333.375 Hz)
//Zone 3: 8000000/32 = 250000 Hz    (ByteReady 31250 Hz)

//Höhere Werte verlangsammen die Bitrate
const uint64_t timer0_values[4] = {26,28,30,32};

//const uint8_t timer0_orca0[4] = {64,69,74,79};            // Diese Werte erzeugen den genausten Bittakt aber nicht 100% (Bei 20MHz)
//const uint8_t timer0_orca0[4] = {77,83,89,95};              // Diese Werte erzeugen den genausten Bittakt aber nicht 100% (Bei 24MHz)

const uint8_t d64_sector_gap[4] = {12, 21, 16, 13}; // von GPZ Code übermommen imggen
#define HEADER_GAP_BYTES (9)


char image_filename[32]; //Maximal 32 Zeichen

uint8_t current_gui_mode;

uint8_t gui_current_line_offset;         // >0 dann ist der Name länger als die maximale Anzeigelaenge
uint8_t gui_line_scroll_pos;             // Kann zwischen 0 und fb_current_line_offset liegen
uint8_t gui_line_scroll_direction;       // Richtung des Scrollings
uint8_t gui_line_scroll_end_begin_wait;

// Alles für den FilebrowserSS
uint16_t fb_dir_entry_count = 0;        // Anzahl der Einträge im aktuellen Direktory
uint8_t fb_cursor_pos = 0;              // Position des Cursors auf dem LCD Display
uint8_t fb_window_pos = 0;              // Position des Anzeigebereichs innerhablb der Menüeinträge

uint8_t fb_current_line_offset = 0;         // >0 dann ist der Name länger als die maximale Anzeigelaenge
uint8_t fb_line_scroll_pos = 0;             // Kann zwischen 0 und fb_current_line_offset liegen
uint8_t fb_line_scroll_direction = 0;       // Richtung des Scrollings
uint8_t fb_line_scroll_end_begin_wait = 10;

