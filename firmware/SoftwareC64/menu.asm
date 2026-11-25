        ;!to "menu.prg",cbm

        !ifdef DTV {
        !cpu c64dtv2
        } else {
        !cpu 6510
        } 

;   Typ [8bit, 1 =file, 2=dir], 
;   Filename[chars],stopbyte[8bit, =0] .. jeweils pro eintrag hintereinander
    TYPE_NONE = 0
    TYPE_DIR = 1
    TYPE_D64 = 2
    TYPE_G64 = 3
    TYPE_PRG = 4
    TYPE_UNKNOWN = 255

!ifdef PLUS4 {
    C16_64K = 1
    C16 = 1
}

!ifdef VIC20 {
    MAX_X = 22	    ;screen width
    MAX_LINE = 21

    VIDEORAM = $1000
    COLORRAM = $9400
    CLR_SCREEN = $E55F
    BORDER_COLOR = $02 
    SEARCH_COLOR_XOR = 4
    LOAD = $ffd5
    BA = $ba
    ptr1 = $fb
    ptr2 = $fd
    ptr3 = $f7
    D020 = $900f
    
    RASTER_LINE_READ = $9004
    NUM_PAGES = >($2000-directory_start)

} else  { ; VIC20

    MAX_X = 40	    ;screen width
    MAX_LINE = 23

    
!ifdef C16 {
    VIDEORAM = $0c00
    COLORRAM = $0800
    RASTER_LINE = $ff0b
    RASTER_LINE_READ = $ff1d
    VIC_IRQ_ENA = $ff0a
    VIC_IRQ_STATUS = $ff09
    EA31 = $ce0e
    CLR_SCREEN = 55435
    D020 = $ff19
    D021 = $ff15
    BORDER_COLOR = $31
    SELECTOR_COLOR = $16
    SEARCH_COLOR_XOR = $40

    LOAD = $ffd5
    BA = $0134
    ptr1 = $03
    ptr2 = $05
    ptr3 = $9f
    !ifdef C16_64K {
    NUM_PAGES = >($8000-directory_start)
    } else {
    NUM_PAGES = >($4000-directory_start)
    }
    

    TED_JOY    = $ff08        ; TED joystick/keyboard register
    KB_LATCH   = $fd30        ; Keyboard latch

} else { ; C64

    VIDEORAM = $0400
    COLORRAM = $d800
    RASTER_LINE = $d012
    RASTER_LINE_READ = $d012
    VIC_IRQ_ENA = $d01a
    VIC_IRQ_STATUS = $d019
    EA31 = $ea31
    CLR_SCREEN = $e51b
    D020 = $d020
    D021 = $d021
    BORDER_COLOR = $0b
    SEARCH_COLOR_XOR = 3
    SELECTOR_COLOR = $06
    BA = $ba
    !ifdef DTV {
    LOAD = $014b
    } else {
    LOAD = FASTLOAD_EXOS
    }
    ptr1 = $fb
    ptr2 = $fd
    ptr3 = $f7
    NUM_PAGES = >($d000-directory_start)
} ; C16
} ; VIC20

    STACK_START = $0135

tmp1 = $02
tmp2 = $ff
directory_end = $ae
x_pos = $92

current_pos = $c1
current_endpos = $c3

current_cursor_position = $9e
!ifdef VIC20 {
current_cursor_last_position = $9f
}

current_index_number = $ac

DRIVECODE_START = $0146

TOGGLE_DISKWRITE_FOR_DISKSWITCH = 2

;CLEANUP_BLOCK_WRITTEN = 1
DRIVE_RESET = 1;
DRIVE_INITIALIZE = 1


!ifdef VIC20 {
LINE_SIZES = stck_code1 - MAX_LINE
} else {
LINE_SIZES = $033c  ;"MAX_LINE"-bytes to line lengths
}

!macro dec_word .x {
        lda .x
        bne +
        dec .x+1
+       dec .x
        }

!macro inc_word .x {
        inc .x
        bne +
        inc .x+1
+
}

        !ifdef VIC20 {
        *= $1201
        !by $0b,$12,$00,$00,$9e,$34,$36,$32,$31,$00,$00,$00
        } else {
        !ifndef C16 {
        * = $0808
        !ifdef DTV {
        ldx #$01    ;set kernal bank to default
        stx $d03f
        dex
        stx $d100
        stx $d03f
        } else {
        lda $02A6
        bne +
        lda #$c1
        sta exos_ntsc_pal_switch
+       }
        } else {
        !ifdef C16_64K {
        * = $1008
        } else {
        * = $1001
        !by $0b,$10,$00,$00,$9e,$34,$31,$30,$39,$00,$00,$00
        }
        }
        }
        !ifdef VIC20 {
        ;first, set how much memory will can use
        lda $0284   ;basic end
        sec
        sbc #>directory_start
        sta number_of_pages
        }
        
        jsr fastload_init
        !ifdef VIC20 {
        } else { ; VIC20
        !ifdef C16 {
        lda #$08
        sta BA
        } else {
        dec $01
        }
        } ; VIC20
restart:
        jsr load_menu_file
        !ifdef VIC20 {
        lda #$02
        sta current_cursor_last_position
        }        
        jsr CLR_SCREEN
        ldy #MAX_X
-       lda bottomline-1,y
        ora #$80
        sta VIDEORAM+(MAX_LINE+1)*MAX_X-1,y
        lda #BORDER_COLOR
        sta COLORRAM-1,y
        sta COLORRAM+(MAX_LINE+1)*MAX_X-1,y
        dey
        bne -
        !ifdef VIC20 {
        lda #BORDER_COLOR+8+1*16
        sta $900f
        } else { ; VIC20
        sta D020
        sty D021
        }
        !ifdef C16 {
        lda #$39
        } else {
        lda #$1b
        sta $d011
        }
        !ifdef VIC20 {
        lda #$c2
        sta $9005
        } else {   ; VIC20
        sta RASTER_LINE
        !ifdef C16 {
        lda #$d5
        sta $ff13
        } else {
        lda #$17
        sta $d018
        lda #$7f
        sta $dc0d
        lda $dc0d
        }
        lda #<irq
        sta $0314
        lda #>irq
        sta $0315
        !ifdef C16 {
        lda #$02
        } else {
        lda #$81
        sta VIC_IRQ_ENA
        }
        !ifdef C16 {
        sta VIC_IRQ_STATUS
        } else {
        inc VIC_IRQ_STATUS
        }
        }  ; VIC20

restart_home:
        jsr do_restart_home

repaint:
        jsr print_at_current_pos
        !ifdef VIC20 {
        ldx current_cursor_position
        beq +
        dex
        !byte $89
+       inx
        stx current_cursor_last_position
        }
keyloop:
        jsr get_current_type
        sta current_type
        
        !ifdef VIC20 {
        jsr set_scroll_bar
        }
        ;ask for joystick
        !ifdef VIC20 {
            lax     $9122
            and #$7f
            sta     $9122
            lda     $9111
            lsr
            lsr
            ora     #$10
            bit     $9120
            bmi     +
            and     #$ef
+
            stx $9122
            and #$1f
            cmp #$1f
        } else {
        !ifdef C16 {
joy_c16_value = *+1
            lda #$ff
            and #$cf
            cmp #$cf
        } else {
            lda $dc00
            and #$1f
            cmp #$1f
        }
        }
            beq +

joy_scroll_speed = *+1
            ldy #10
--
            !ifdef VIC20 {
            ldx #$00
            } else {
            ldx #$fc
            }
-           cpx RASTER_LINE_READ
            bne -
            inx
            !ifdef C16 {
            inx     ;bit 0 not used for C16
            }
-           cpx RASTER_LINE_READ
            bne -
            dey
            bne --
            
            ldx #1
            stx joy_scroll_speed
            
            lsr
            bcc do_cursorup
            lsr
            bcc do_cursordown
            lsr
            bcc do_f1
            !ifdef VIC20 {
            lsr
            bcc jmp_do_space
            }
            lsr
            bcc do_f7
            !ifdef VIC20 {
            ;for vic20, fire button and right are switched
            bcs + 
            } else {
            lsr
            !ifdef C16 {
            ;for C16, fire is bit 6+7, not 4
            lsr
            lsr
            bcc jmp_do_space
            lsr
            }
            bcs +
            }
jmp_do_space:
            jmp  do_space
+
        ldx #10
        stx joy_scroll_speed
        jsr $ffe4
        beq keyloop
        cmp #17    ;cursor down
        bne no_cursor_down
do_cursordown:
        jsr call_cursor_down
        bcs repaint
        jmp keyloop

call_cursor_down:
        lda current_cursor_position
maxline_cmp_value = *+1
        cmp #MAX_LINE-1
        bcc do_cursor_down
        jsr try_scroll_down
        sec
        rts
do_cursor_down:
        inc current_cursor_position
        clc
        rts

no_cursor_down:
        cmp #145    ;cursor up
        bne no_cursor_up
do_cursorup:
        lda current_cursor_position
        beq do_scroll_up
        dec current_cursor_position                
        jmp keyloop
do_scroll_up:
        jsr try_scroll_up
        jmp repaint

no_cursor_up:
        cmp #136    ;f7
        bne no_f7
do_f7:
        lda #MAX_LINE
-       pha
        jsr try_scroll_down
        bcc +
        jsr calc_at_current_pos
+       pla
        sec
        sbc #$01
        bne -
        jmp repaint

no_f7:  cmp #133    ;f1
        bne no_f1
do_f1:
        lda #MAX_LINE
-       pha
        jsr try_scroll_up
        bcc +
        jsr calc_at_current_pos
+       pla
        sec
        sbc #$01
        bne -
        jmp repaint

no_f1:  cmp #29     ;cursor right
        bne no_crsr_right
        lda x_pos
        cmp #255-MAX_X
        bcc +
        jmp keyloop
+       inc x_pos
        jmp repaint
        
no_crsr_right:
        cmp #157    ;cursor left
        bne no_crsr_left
        lda x_pos
        beq do_keyloop2
        dec x_pos
        jmp repaint

do_keyloop2:
        jmp keyloop

no_crsr_left:
        cmp #$13    ; home
        bne no_home
        jsr home
        tay
        jmp restart_home

home:
        lda #$00
        sta current_index_number
        sta current_index_number+1
        sta current_cursor_position
        sta x_pos
        rts
        
no_home:
        cmp #134
        bne no_f3

        lda current_type
        cmp #TYPE_DIR
        beq do_keyloop
        
        !ifndef C16 {
        lda BA
        clc
        adc #$30
        sta drive_number
        }
        
        !ifdef C16 {
        !ifdef C16_64K {
        lda #$2c
        sta fastload_c16_deactivated
        }
        }
        lda #'0'
        sta load_run_text_patch
        lda #'9'
        sta load_run_text_patch+1
        bne do_space    ;unconditional	

no_f3:       
        !ifdef VIC20 {
        cmp #135
        bne no_f5
        lda #$4c
        sta enable_5k
        lda #$20
        bne do_space    ;unconditional

do_5k:
        ldx #$ff
        sei
        txa
        cld
        jsr new_fd8d
        jmp $fd32
new_fd8d:
        lda #$00
        tax
fd90    sta $00,x
        sta $0200,x
        sta $0300,x
        inx
        bne fd90
        ldx #$3c
        ldy #$03
        stx $b2
        sty $b3
        sta $c1
        sta $97
        sta $0281
        tax
        ldy #$1e
        sty $0288
        lda #$10
        sta $0282
        sta $c2
        lda #$01
        sta $c1
        jmp $fe7b

	
no_f5:
        }
        cmp #$20
        beq do_space
        !ifdef C16 {
        cmp #$1b
        } else {
        cmp #$5f
        }
        bne +
        !ifdef VIC20 {
        lda #$ff
        jsr vicload
        jmp 64802
        } else {
        !ifdef C16 {
        jmp 65526   ;reset
        } else {
        jmp $fce2
        }
        }
+

        cmp #$0d    ;return
        beq do_space
        jsr enter_search
        bcc do_keyloop
        jmp repaint
do_keyloop:
        jmp keyloop
do_space:
        sbc #$20
        sta do_run
        bne +
        lda current_type
        cmp #TYPE_DIR
        beq do_keyloop
+
        lda current_type
        cmp #TYPE_PRG+1
        bcs do_keyloop
        cmp #TYPE_DIR
        bcc do_keyloop
        lda #$00
        !ifdef VIC20 {
        } else { ; VIC20
        !ifdef C16 {
        lda #$cc
        sta RASTER_LINE
        lda #$02
        sta VIC_IRQ_STATUS
        } else {
        sta VIC_IRQ_ENA
        inc VIC_IRQ_STATUS
        }
        lda #<EA31
        sta $0314
        lda #>EA31
        sta $0315
        !ifdef C16 {
        } else {
        lda #$81
        sta $dc0d
        }
        ;lda $dc0d
        } ; VIC20
        
        lda current_cursor_position
        ;sec        ; carry is set because no "bcc do_keyloop"
        adc current_index_number
        sta index
        lda current_index_number+1
        adc #$00
        sta index+1

        jsr upload_code
        !ifdef CLEANUP_BLOCK_WRITTEN {
        jsr clear_buffer_2
        }
        jsr block_write
        jsr wait_for_flipdisk
        !ifdef VIC20 {
        ; do this later for vic20 for the fast loader
        } else {
        !ifdef DRIVE_RESET {
        jsr drive_initialize
        }
        }
current_type = *+1
        lda #$00
        cmp #TYPE_DIR
        bne +
        jmp restart
+        
        !ifdef C16 {
        sei
        sta $FF3E
        ldx #$ff
        txs
        jsr 55373   ;editor reset
        jsr 62219   ;io init
        jsr 62158   ;restore vectors
        lda #$00
        sta $fdd0 
        cli
        }
do_run = *+1
        lda #$00
        beq do_exit
        !ifdef C16 {
        ;jsr drive_reset
        ;jsr 32814   ;basic reset
        !ifdef C16_64K {
fastload_c16_deactivated:
        jsr $0700
        }
        ldx #load_run_text_len
        stx $ef
        } else {
        ldx #load_run_text_len
        stx $c6
        }
        sei
-       lda load_run_text-1,x
        !ifdef C16 {
        sta $0527-1,x
        } else { 
        sta $0277-1,x
        }
        dex
        bne -
do_exit:
        !ifdef VIC20 {
        jsr drive_initialize    ;do this late for vic20, it is not done above
        }
        ldy #$80
        ldx #$00
-       inx
        bne -
        iny
        bne -
        !ifdef VIC20 {
enable_5k:
        bit do_5k
        jmp $fd35   ;jump to basic warm start
        } else { ; VIC20
        !ifdef C16 {
c16reset:
        jsr 32962   ;init screen
        cli
        ;jmp ($0302)
        jmp 32778   ;basic warm start
        } else {
        inc $01
        jsr $fda3
        !ifdef DTV {
        jsr $ff5b
        jsr $fd15
        jsr $e422
        jmp $a474
        } else {
        jmp $fcf8   ;part of reset, jumps to ($a000)
        }
        }
        } ; VIC20

        !ifndef VIC20 {
        !align $ff,0
irq:    ldy #SELECTOR_COLOR
        !ifndef C16 {
        lda RASTER_LINE_READ
-       cmp RASTER_LINE_READ
        beq -
        }
        sty D021
        lda #<irq2
        sta $0314
        ;lda #>irq2
        ;sta $0315
current_cursor_position_this_irq = *+1
        lda #$00
        clc
        !ifdef C16 {
        adc #$12
        } else {
        adc #$42
        }
        sta RASTER_LINE

        lda current_cursor_position
        asl
        asl
        asl
        sta current_cursor_position_this_irq
        sta current_cursor_position_other_irq
        !ifdef C16 {
        lda #$02
        sta VIC_IRQ_STATUS
        } else {
        inc VIC_IRQ_STATUS
        dec $028B
        bne +
        inc $028B
+
        }
        !ifdef C16 {
        jmp c16irqcont
        } else {
        jmp $ea81
        }

irq2:
        ldy #$03
-       dey
        bne -
        sty D021
        lda #<irq
        sta $0314
        ;lda #>irq
        ;sta $0315
current_cursor_position_other_irq = *+1
        lda #$00
        clc
        !ifdef C16 {
        adc #$a
        } else {
        adc #$39
        }
        sta RASTER_LINE
        !ifdef C16 {
        lda #$02
        sta VIC_IRQ_STATUS
c16irqcont:
        cli
        
        lda #$ff
        sta KB_LATCH

        ; 2. Poll Joystick 1
        ldx #$fb              ; Select Joystick 1 (%11111011)
        stx TED_JOY
        lda TED_JOY           ; Read Joystick 1 lines

        ; 3. Poll Joystick 2
        ldx #$fd              ; Select Joystick 2 (%11111101)
        stx TED_JOY
        and TED_JOY           ; Read Joystick 2 lines

        sta joy_c16_value
        ; 4. Re-enable standard system keyboard scanning (optional, but good practice)
        lda #$00
        sta KB_LATCH

        jsr $DB11       ;check keyboard
        
        pla
        tay
        pla
        tax
        pla
        rti
        } else {
        inc VIC_IRQ_STATUS
        jmp EA31
        }

    !if >irq2 != >irq {
        !error "irq and irq in different pages"
    }
    }   ;!ifndef VIC20 

do_restart_home:
        ldy #<directory_start  ; is zero now
        sty ptr1
        lda #>directory_start
        sta ptr1+1
        sta current_pos+1
        jsr paint_top_line
        ;tya
        ;sec
        ;adc #<directory_start
        ;sta current_pos
        iny
        sty  current_pos
        ;lda #$00
        ;adc #>directory_start
        ;lda #>directory_start
        ;sta current_pos+1

        jsr calc_at_current_pos
        ldx tmp1
        stx maxline_cmp_value_calc_sizes
        beq +
        dex
+       stx maxline_cmp_value
        rts
get_current_type:
        lda current_pos
        sta ptr1
        lda current_pos+1
        sta ptr1+1
        ldx #$00
        beq get_current_type_loop ;unconditional
--      inx
        ldy #$00
-       iny
        lda (ptr1),y
        bne -
        tya
        sec
        adc ptr1
        sta ptr1
        bcc +
        inc ptr1+1
+
        
get_current_type_loop:
        cpx current_cursor_position
        bne --
        ldy #$00
        lda (ptr1),y
        rts


paint_top_line:
        ldy #$00
-       lda directory_start,y
        beq end_of_top_line_string
        jsr conv_to_screen
        ora #$80
        sta VIDEORAM,y
        iny
        cpy #MAX_X
        bne -
        dey
-       iny
        lda directory_start,y
        bne -
end_of_top_line_string:
        tya
        tax
-       cpx #MAX_X
        bcs end_top_line
        lda #$a0
        sta VIDEORAM,x
        inx
        bne -
end_top_line:
        rts

calc_at_current_pos:
        lda current_pos
        sta ptr1
        lda current_pos+1
        sta ptr1+1
        lda #$00
        sta tmp1
calc_big_loop:
        lda ptr1
        cmp directory_end
        lda ptr1+1
        sbc directory_end+1
        bcs end_calcing

        ldy #$ff
-       iny
        lda (ptr1),y
        bne -
        tya
        sec
        adc ptr1
        sta ptr1
        bcc +
        inc ptr1+1
+
        inc tmp1
        lda tmp1
        cmp #MAX_LINE
        bne calc_big_loop
end_calcing:
        jmp end_printing


print_at_current_pos:
        ;cal line sizes
        lda current_pos
        sta ptr1
        lda current_pos+1
        sta ptr1+1
        ldx #$ff
--      
        ldy #$00
        inx
maxline_cmp_value_calc_sizes = *+1
        cpx #00
        beq print_at_current_pos_continue
        dey
-       iny
        lda (ptr1),y
        bne -
        tya
        sta LINE_SIZES,x
        sec
        adc ptr1
        sta ptr1
        bcc --
        inc ptr1+1
        bne --

print_at_current_pos_continue:        
        ;print
        
        lda #<LINE_SIZES
        sta line_size

        lda current_pos
        sta ptr1
        lda current_pos+1
        sta ptr1+1
        lda #$00
        sta tmp1
        lda #MAX_X
        sta ptr2
        sta ptr3
        lda #>VIDEORAM
        sta ptr2+1
        lda #>COLORRAM
        sta ptr3+1
print_big_loop:
        lda ptr1
        cmp directory_end
        lda ptr1+1
        sbc directory_end+1
        bcc +
        jmp end_printing
+
        ldy #$00
        lax (ptr1),y
        inx
        +inc_word ptr1        

        lda x_pos
line_size = *+1
        cmp LINE_SIZES
        bcc do_print
-       lda (ptr1),y
        beq +
        iny
        bne -
+       sty tmp2
        ldy #$00
        lda #$20
-       sta (ptr2),y
        !ifdef VIC20 {
        lda filetypecolor,x        
        sta (ptr3),y
        lda #$20       
        }
        iny
        cpy #MAX_X
        bne -
        ldy tmp2
        jmp end_line

do_print:
        lda x_pos
        clc
        adc ptr1
        sta ptr1
        bcc +
        inc ptr1+1
+        
-       lda (ptr1),y
        beq string_ends
        jsr conv_to_screen
        sta (ptr2),y
        lda filetypecolor,x
        sta (ptr3),y
        iny
        cpy #MAX_X
        bne -
        ;find end of string
-       lda (ptr1),y
        beq end_line
        iny
        bne -
string_ends:
        sty tmp2
        lda #$20
-       sta (ptr2),y
        !ifdef VIC20 {
        lda filetypecolor,x        
        sta (ptr3),y
        lda #$20       
        }
        iny
        cpy #MAX_X
        bne -
        ldy tmp2
end_line:
        tya
        sec
        adc ptr1
        sta ptr1
        bcc +
        inc ptr1+1
        clc
+       lda ptr2
        adc #MAX_X
        sta ptr2
        sta ptr3
        bcc +
        inc ptr2+1
        inc ptr3+1
+       inc line_size
        inc tmp1
        lda tmp1
        cmp #MAX_LINE
        beq end_printing
        jmp print_big_loop
end_printing:
        lda ptr1
        sta current_endpos
        lda ptr1+1
        sta current_endpos+1
        rts

!macro current_pos_equ_ptr1_plus_1 {
        lax ptr1
        sbx #$100-$01
        stx current_pos
        lda ptr1+1
        adc #$00
        sta current_pos+1
        }

try_scroll_up:
        lda current_index_number
        ora current_index_number+1
        beq try_scroll_up_failed
        lax current_pos
        sbx #$02
        stx ptr1
        lda current_pos+1
        sbc #$00
        sta ptr1+1
        ldy #$00
        jmp try_scroll_up_do_search
-
        +dec_word ptr1
try_scroll_up_do_search:
        lda (ptr1),y
        bne -
        +current_pos_equ_ptr1_plus_1
        +dec_word current_index_number
        sec
        rts
        
try_scroll_down_failed:
try_scroll_up_failed:
        clc
        rts


try_scroll_down:
        lda current_endpos
        cmp directory_end
        lda current_endpos+1
        sbc directory_end+1
        bcs try_scroll_down_failed
        lda current_pos
        sta ptr1
        lda current_pos+1
        sta ptr1+1
        ldy #$00
        beq try_scroll_down_do_search   ;unconditional
-       
        +inc_word ptr1
try_scroll_down_do_search:
        lda (ptr1),y
        bne -
        +current_pos_equ_ptr1_plus_1
        +inc_word current_index_number
        sec
ret_vic20:
        rts


        !ifdef VIC20 {
set_scroll_bar:
        ldx current_cursor_last_position
        cpx current_cursor_position
        beq ret_vic20
        jsr get_curser_ptr1
        ldy #22-1
-       lda (ptr1),y
        and #$7f
        sta (ptr1),y
        dey
        bpl -
        
        ldx current_cursor_position
        stx current_cursor_last_position
        jsr get_curser_ptr1
        ldy #22-1
-       lda (ptr1),y
        ora #$80
        sta (ptr1),y
        dey
        bpl -         
        rts
        
get_curser_ptr1:
        lda screen_low_bytes,x
        sta ptr1
        lda screen_high_bytes,x
        sta ptr1+1
        rts


screen_low_bytes:
        !for i,1,22 {
        !byte <(VIDEORAM+22*i)
        }
 
screen_high_bytes:
        !for i,1,22 {
        !byte >(VIDEORAM+22*i)
        }
        

        }


conv_to_screen:
        cmp #$5f
        bne +
        lda #$64
        bne store_code
+
        cmp #$60
        bcc store_code
        sbc #$20
store_code:
        rts

clear_search_buffer:
        ldx #search_buffer_len-1
-       lda #$20
        sta search_buffer,x
        dex
        bne -
        jmp do_search_cursor

enter_search:
        cmp #$03
        beq store_code
        tax
        lda D020
        pha
        txa
        pha
        ldx #menu_line_lenght-1
-       lda menu_line,x
        sta VIDEORAM,x
        dex
        bpl -
        pla
        cmp #$14
        beq print_search_char
        jmp +
next_search_char:
        jsr $ffe4
        beq next_search_char
        bmi next_search_char
        cmp #$13
        beq clear_search_buffer
        cmp #$0d
        beq do_search
        cmp #$14        ;backspace
        bne +
        ldx len_search
        beq next_search_char
        lda #$20
        sta search_buffer,x
        dex
        jmp do_search_cursor     
+       cmp #$20
        bcc next_search_char
        ldx len_search
        cpx #search_buffer_len-1
        beq next_search_char
        jsr conv_to_screen
        sta search_buffer,x
        inx
do_search_cursor:
        lda #$a0
        sta search_buffer,x
        stx len_search
print_search_char:
        ldx #search_buffer_len-1
-       lda search_buffer,x
        eor #$80
        sta VIDEORAM+menu_line_lenght,x
        dex
        bpl -

        jmp next_search_char

do_search:
        lda len_search
        bne +
        jmp nothing_to_search        
+
        sei
        !ifdef DTV {
        sac #$99
        lda #$03
        sac #$00
        }
        lda #$2c
        sta found_jump
        lda search_buffer
        sta search_cmp_value
        
        jsr get_current_type    ;get current pos in ptr1
        lda ptr1
        sta end_ptr_low
        lda ptr1+1
        sta end_ptr_high
search_next_entry:
        ldy #$01
search_next_entry_char:
        lda (ptr1),y
        beq found_end_of_entry
        jsr conv_to_screen
search_cmp_value = *+1
        cmp #00
        bne first_char_does_not_fit
        ;make a deeper check

        tya
        tax
        clc
        adc ptr1
        sta ptr2
        lda ptr1+1
        adc #$00
        sta ptr2+1

        ldy len_search
        dey
-       lda (ptr2),y
        jsr conv_to_screen
        cmp search_buffer,y
        bne search_does_not_fit
        dey
        bpl -
        ;found !
found_jump:
        bit found_entry

search_does_not_fit:
        txa
        tay

first_char_does_not_fit:
        iny
        bne search_next_entry_char

found_end_of_entry:
        lda D020
        eor #SEARCH_COLOR_XOR
        sta D020
        lda #$4c
        sta found_jump
        tya
        sec
        adc ptr1
        sta ptr1
        bcc +
        inc ptr1+1
+       lda ptr1
        pha
        lda ptr1+1
        pha
        jsr call_cursor_down
        pla
        sta ptr1+1
        pla
        sta ptr1
        lda ptr1
        cmp directory_end
        lda ptr1+1
        sbc directory_end+1
        bcc no_wrap_around
        ;find first entry after directory name
        ldy #$ff
-       iny
        lda directory_start,y
        bne -
        iny
        tya
        pha
        jsr home
        jsr do_restart_home
        pla
        sta ptr1
        lda #>directory_start
        sta ptr1+1
no_wrap_around:
        !ifdef VIC20 {
        lda #%11110111  ; Select Column 0 (pull Bit 0 low)
        sta $9120       ; VIA #2 Port B (Keyboard Column Select)
        lda $9121       ; VIA #2 Port A (Keyboard Row Read)
        lsr             ; Mask Bit 1 (Row 3 = RUN/STOP)
        bcc nothing_to_search; Active-low: 0 means the key is pressed
        } else {
        !ifdef C16 {
        lda #$7f        ; Bit 7 auf 0 setzen, um Tastaturzeile 7 auszuwählen
        sta KB_LATCH    ; In Tastatur-Latch schreiben
        sta TED_JOY
        lda TED_JOY     ; TED-Tastaturport auslesen
        bpl nothing_to_search
        } else {
        lda $dc01
        bpl nothing_to_search
        }
        }
        lda ptr1
end_ptr_low = *+1
        cmp #$00
        bne do_search_next_entry
+       lda ptr1+1
end_ptr_high = *+1
        cmp #$00
        beq nothing_to_search
do_search_next_entry:
        jmp search_next_entry

nothing_to_search:
        !ifdef C16 {
        ; 4. Re-enable standard system keyboard scanning (optional, but good practice)
        lda #$00
        sta KB_LATCH
        }

        clc
        !byte $24
        ;nothing found
found_entry:
        sec
        pla
        sta D020
        php
        jsr paint_top_line
        !ifdef VIC20 {
        } else {
        !ifdef C16 {
        lda #$81
        sta VIC_IRQ_STATUS
        } else {
        !ifdef DTV {
        lda VIC_IRQ_STATUS
        sta VIC_IRQ_STATUS
        } else {
        inc VIC_IRQ_STATUS
        }
        }
        }
        plp
        !ifdef DTV {
        sac #$99
        lda #$00
        sac #$00
        }
        cli
        rts


menu_line:
        !ct scr {
        !byte 'S'+$80
        !byte 'e'+$80
        !byte 'a'+$80
        !byte 'r'+$80
        !byte 'c'+$80
        !byte 'h'+$80
        !byte ' '+$80
        !byte ':'+$80
        !byte ' '+$80
        }
menu_line_lenght = * - menu_line

load_menu_file:
        !ifdef VIC20 {
        jsr L1476   ;initialize
        jsr L14DC   ;execute fastloader
        ldx #<directory_start
        ldy #>directory_start
        lda #$06    ; sixth file !!!!
        sec
        jsr vicload
        stx directory_end
        sty directory_end+1
        lda #$ff
        jsr vicload
        } else { ; VIC20
        lda #fname_end-fname
        ldx #<fname
        ldy #>fname
        jsr $ffbd     ; call SETNAM
        lda #$01
        ldx BA
        ldy #$00      ; $00 means: load to new address
        jsr $ffba     ; call SETLFS

        ;ldx #<directory_start
        ldy #>directory_start
        ;lda #$00      ; $00 means: load to memory (not verify)
        lxa #$00
        stx $c3
        sty $c4
        jsr LOAD      ; call LOAD
        bcs .error    ; if carry set, a load error has happened
        !ifdef C16 {
        stx directory_end
        sty directory_end+1
        }
        } ; VIC20
        lda #$00
        sta current_index_number
        sta current_index_number+1
        sta current_cursor_position
        sta x_pos
        rts

print_load:
        ldx #loadtextlen-1
--      lda loadtext,x
        jsr $ffd2
        !ifdef C16 {
        lda #$60
        } else {
        lda #$40
        }
        ldy #$00
-       iny
        bne -
        sec
        sbc #$01
        bne -
        dex
        bpl --
        lxa #$00    ;sys should return with 0 value
        tay
        rts        

.error
        ; Accumulator contains BASIC error code

        ; most likely errors:
        ; A = $05 (DEVICE NOT PRESENT)
        ; A = $04 (FILE NOT FOUND)
        ; A = $1D (LOAD ERROR)
        ; A = $00 (BREAK, RUN/STOP has been pressed during loading)

        ;... error handling ...
-       inc D020
        jmp -
    
block_write:
        ; open the channel file

        LDA #cname_end-cname
        LDX #<cname
        LDY #>cname
        JSR $FFBD     ; call SETNAM

        LDA #$02      ; file number 2
        ldx BA
        tay           ; secondary address 2
        JSR $FFBA     ; call SETLFS

        JSR $FFC0     ; call OPEN
        BCS .error    ; if carry set, the file could not be opened

        ; open the command channel

        LDA #bpcmd_end-bpcmd
        LDX #<bpcmd
        LDY #>bpcmd
        JSR $FFBD     ; call SETNAM
        LDA #$0F      ; file number 15
        ldx BA
        tay           ; secondary address 15
        JSR $FFBA     ; call SETLFS

        jsr $FFC0     ; call OPEN (open command channel and send B-P command)
        BCS error    ; if carry set, the file could not be opened

        ; check drive error channel here to test for
        ; FILE NOT FOUND error etc.

        LDX #$02      ; filenumber 2
        JSR $FFC9     ; call CHKOUT (file 2 now used as output)

        LDY #$00
.loop   LDA sector_address,Y   ; read byte from memory
        JSR $FFD2     ; call CHROUT (write byte to channel buffer)
        INY
        cpy #sector_len
        BNE .loop     ; next byte, end when 256 bytes are read

        LDX #$0F      ; filenumber 15
        JSR $FFC9     ; call CHKOUT (file 15 now used as output)

        LDY #$00
.loop2  LDA bwcmd,Y   ; read byte from command string
        JSR $FFD2     ; call CHROUT (write byte to command channel)
        INY
        CPY #bwcmd_end-bwcmd
        BNE .loop2    ; next byte, end when 256 bytes are read
.close

        JSR $FFCC     ; call CLRCHN

        LDA #$0F      ; filenumber 15
        JSR $FFC3     ; call CLOSE

        LDA #$02      ; filenumber 2
        JSR $FFC3     ; call CLOSE

        JMP $FFCC     ; call CLRCHN

error:
        ; Akkumulator contains BASIC error code

        ; most likely errors:
        ; A = $05 (DEVICE NOT PRESENT)

        ;... error handling for open errors ...
        JMP .close    ; even if OPEN failed, the file has to be closed

drive_initialize:
        !ifdef DRIVE_INITIALIZE {
        !ifndef C16 {
        lda do_run
        bpl drive_reset
        }
        lda #drive_init_end-drive_init
        ldx #<drive_init
        ldy #>drive_init
        bne execute_code_cont ;unconditional 
	} else {
	lda do_run
	beq drive_reset
	rts		;for EXOS, no initialize
	}
	!ifdef DRIVE_RESET {
drive_reset:
        lda #$60
        sta reset_activated
        ldx #<drive_fffc
        ;ldy #>drive_fffc
        bne execute_code ;unconditional
        }
                

wait_for_flipdisk:
        ldx #<wait_for_diskchange
        ;ldy #>wait_for_diskchange
        !ifdef CLEANUP_BLOCK_WRITTEN {
        bne execute_code ;unconditional

clear_buffer_2:
        ldx #<clear_buffer2
        ;ldy #>clear_buffer2
        } 

execute_code:
        ;in x/y code (x low)
        stx me_dest
        ;sty me_dest+1
        lda #me_comm_end-me_comm
        ldx #<me_comm
        ldy #>me_comm
        bne execute_code_cont

upload_code:
        ; open the command channel

        LDA #mw_end-mw_comm
        LDX #<mw_comm
        LDY #>mw_comm
execute_code_cont:
        JSR $FFBD     ; call SETNAM
        LDA #$0F      ; file number 15
        ldx BA
        tay           ; secondary address 15
        JSR $FFBA     ; call SETLFS

        jsr $FFC0     ; call OPEN (open command channel and send B-P command)
        BCS error     ; if carry set, the file could not be opened
reset_activated:
        LDA #$0F      ; filenumber 15
        jsr $FFC3     ; call CLOSE
        JMP $FFCC     ; call CLRCHN

me_comm:
         !text "M-E"
me_dest:
         !byte <DRIVECODE_START
         !byte >DRIVECODE_START
me_comm_end:

cname:  !text "#"
cname_end:

bpcmd:  !text "B-P:2 0"
bpcmd_end:

bwcmd:  !text "U2:2 0 18 2"
        !BYTE $0D     ; carriage return, required to start command
bwcmd_end:

        !ifdef DRIVE_INITIALIZE {
drive_init:
        !text "I"
drive_init_end:
        }

fname:  !text "DATAFILE"
fname_end:
       
load_run_text:
        !byte 'S'
        !byte 'y'
        !byte '3'
load_run_text_patch:
        !byte '1'
        !byte '2'
        !byte $0d
        !byte $0d
        !byte 'R'
        !byte 'u'
        !byte $0d
load_run_text_len = *-load_run_text

loadtext:
        !byte 145
        !byte 145

        !ifdef C16 {
        !byte $22
        !byte '*'
        !byte $22
        !byte 'l'
        !byte 'D'

        } else {
        !byte '1'
        !byte ','
drive_number:
        !byte '8'
        !byte ','
        !byte $22
        !byte '*'
        !byte $22
        !byte 'o'
        !byte 'L'
        }
loadtextlen = * - loadtext

        
bottomline:
        !ifdef VIC20 {
        !scr "<SPC>=mnt <RET/F3>=run"
        } else { ; VIC20
        !ifdef C16 {
        !scr "<SPC>=mnt RET=run F2=load F1=up HLP=down"
        } else {
        !scr "<SPC>=mnt RET=run F3=load  F1=up F7=down"
        }
        } ; VIC20

mw_comm:
         !text "M-W"
mw_dest: !word DRIVECODE_START
         !byte diskdrive_code_len
diskdrive_code:
        !pseudopc DRIVECODE_START {
        !ifdef CLEANUP_BLOCK_WRITTEN {
clear_buffer2:
        lxa #$00
-       sta $0600,x
        inx
        bne -
        rts
        }
wait_for_diskchange:
        !if TOGGLE_DISKWRITE_FOR_DISKSWITCH > 1 {
        ldx #TOGGLE_DISKWRITE_FOR_DISKSWITCH
        }
--
-       lda $1c00
        and #$10
        bne -
-       lda $1c00
        and #$10
        beq -
        !if TOGGLE_DISKWRITE_FOR_DISKSWITCH > 1 {
        dex
        bne --
        }
        rts
        
drive_fffc:
        jmp ($fffc)
        }
diskdrive_code_len = * - diskdrive_code
mw_end:

filetypecolor:
    !ifdef VIC20 {
        !byte $02   ; unknown
        !byte $02   ; none
        !byte $05   ; dir
        !byte $06   ; d64
        !byte $06   ; g64
        !byte $00   ; prg

L1476:  LDA $ba
        JSR LISTEN
        LDA #$6F
        JSR SECOND
        LDA #$49
        JSR CIOUT
        jmp UNLSN


L14DC:  LDA $ba
        JSR LISTEN
        LDA #$6F
        JSR SECOND
        LDX #$00
L14E8:  LDA L14FD,X
        JSR CIOUT
        INX
        CPX #$05
        BNE L14E8
        JSR UNLSN

        LDY #$80
        LDX #$00
L1419:  DEX
        BNE L1419
        DEY
        BNE L1419
        lda #$7d
-       cmp $911f
        bne -
        rts

L14FD:  !byte $4D,$2D,$45,<L041A, >L041A


    } else { ; VIC20
    !ifdef C16 {
        !byte $52   ; unknown
        !byte $52   ; none
        !byte $5e   ; dir
        !byte $53   ; d64
        !byte $43   ; g64
        !byte $51   ; prg
    } else {
        !byte $0a   ; unknown
        !byte $0a   ; none
        !byte $0e   ; dir
        !byte $03   ; d64
        !byte $03   ; g64
        !byte $0f   ; prg
    }
    } ; VIC20

sector_address:
index:  !word 0
number_of_pages:
        !byte NUM_PAGES ;maximum number of blocks for next "datafile"
        !ifdef CLEANUP_BLOCK_WRITTEN {
        !text "MENU"
        }

sector_len = * - sector_address

len_search:
        !byte 0


search_buffer_len = MAX_X - menu_line_lenght

search_buffer:
        !byte $a0
        !for i,2, search_buffer_len {
            !byte $20
        }

directory_start = (*+$ff) & $ff00

        ;all stuff after this is erased
        ;by loading "DATAFILE"

        !ifdef VIC20 {
fastload_init:
        ldx #fastload_stack_len
-       lda fastload_stack-1,x
        sta STACK_START-1,x
        dex
        bne -

        CODE_IN_MENU = 1    ;some code is needed in the menu
                            ;and the loader itself will be overwritten
        !src "vic20loader.asm"

fastload_stack:
!pseudopc STACK_START {
        jmp print_load
do_vic_fastload:
        jsr L1476   ;initialize
        jsr L14DC   ;execute fastloader
        lda #$00    ; file #0
        clc
        jsr vicload        
        stx $2d
        sty $2e
        lda #$ff
        jsr vicload

        dec $2c
        lda #0
        ldy #$ff
        sta ($2b),y
        inc $2c
        lda $2c
        cmp #$a0    ;modul ?
        bne +
        jmp 64802   ;reset
+       cmp #$12    ;5k/8k program ?
        beq +
        lda #$10
        sta $0282
        lda #$1e
        sta $0284        
        sta $0288
        lda #$f0
        sta $9005
        jsr $e518
+       jsr $c474
        lxa #$00    ;sys should return with 0 value
        tay
        rts        
}
fastload_stack_len = * - fastload_stack 
        

        } else { ;VIC20
        !ifdef C16 {
;;;;;;;;;;;;;;;;;;; PLUS4 START ;;;;;;;;;;;;;;;;;;;;;;;
c16keyb:
        !byte 133,134,135,136,133,134,135,136
        
fastload_init:
        ldx #$07
fkey_clear_loop:
        lda #$01
        sta $055f,x
        lda c16keyb,x
        sta $0567,x
        dey
        dex
        bpl fkey_clear_loop 

        php

        ldx #fastload_stack_len
-       lda fastload_stack-1,x
        sta STACK_START-1,x
        dex
        bne -

        !ifdef C16_64K {
        sei
        sta $ff3f
        ldy #>(fastload_main_len+$ff)
        ldx #$00
-       
src = *+1
        lda fastload_main,x
dst = *+1
        sta $f300,x
        inx
        bne -
        inc src+1
        inc dst+1
        dey
        bne -
        sta $ff3e
        
        ldx #fastload_0700_len
-       lda fastload_0700-1,x
        sta $0700-1,x
        dex
        bne -
        ;for VICE tests
        ;lda #$60
        ;sta $0700
        jsr $0700
        }
        plp
        rts

        !ifdef C16_64K {
fastload_main:
        !bin "fastload_16_f300.bin",,2
fastload_main_len = *-fastload_main

fastload_0700:
        !bin "fastload_16_0700.bin",,2
fastload_0700_len = *-fastload_0700
        }

fastload_stack:
!pseudopc STACK_START {
        jmp print_load
        !ifdef C16_64K {
        lda #load_star_name_len
        ldx #<load_star_name
        ldy #>load_star_name
        jsr $ffbd     ; call SETNAM
        lda #$01
        ldx BA
        tay           ; $01 means: load to origin address
        jsr $ffba     ; call SETLFS

        lda #$00
        cli
        jsr LOAD
        bcs +         ; if carry set, a load error has happened
        stx $2d
        sty $2e
+
        jsr 62158       ;restore vectors
        lxa #$00    ;sys should return with 0 value
        tay
        rts
load_star_name:
        !text ":*"
load_star_name_len = * - load_star_name
        } else {
        jmp print_load  ; no fastloader, print "dload"
        }
}
fastload_stack_len = *-fastload_stack

;;;;;;;;;;;;;;;;;;; PLUS4 END ;;;;;;;;;;;;;;;;;;;;;;;

        } else {
        !ifdef DTV {
;;;;;;;;;;;;;;;;;;; DTV START ;;;;;;;;;;;;;;;;;;;;;;;

fastload_init:
        ldx #fastload_stack_len
-       lda fastload_stack-1,x
        sta STACK_START-1,x
        dex
        bne -
        ldy #>(fastload_main_len)
-       
fastload_main_src_ptr = *+2
        lda fastload_main,x
fastload_main_dst_ptr = *+2
        sta $e000,x
        inx
        bne -
        inc fastload_main_src_ptr
        inc fastload_main_dst_ptr
        dey
        bpl -
        rts

fastload_stack:
!pseudopc STACK_START {
        jmp print_load
        lda #load_star_name_len
        ldx #<load_star_name
        ldy #>load_star_name
        jsr $ffbd     ; call SETNAM
        lda #$01
        ldx BA
        tay           ; $01 means: load to origin address
        jsr $ffba     ; call SETLFS

        lda #$00
        !bin "fastload_0140.bin",$2c,2+11
        jmp +
        !bin "fastload_0140.bin",,2+11+$2f
load_star_name:     ; bug in fast loader : cannot handle ":*"
        !text "*"
load_star_name_len = * - load_star_name
+       ldx $ae
        ldy $af
        stx $2d
        sty $2e
        jsr $f5a9
        lxa #$00    ;sys should return with 0 value
        tay
        rts
        }
fastload_stack_len = * - fastload_stack      

fastload_main:
        !bin "fastload_e000.bin",,2
fastload_main_len = *-1
        } else {
        ;;;;;;;;;;;;;;;;;;; PAL C64 END ;;;;;;;;;;;;;;;;;;;;;;;


;;;;;;;;;;;;;;;;;;; C64 START ;;;;;;;;;;;;;;;;;;;;;;;

fastload_init:
        ldx #fastload_stack_len
-       lda fastload_stack-1,x
        sta STACK_START-1,x
        dex
        bne -
        !src "exos.asm"

fastload_stack:
!pseudopc STACK_START {
        jmp print_load
        lda #load_star_name_len
        ldx #<load_star_name
        ldy #>load_star_name
        jsr $ffbd     ; call SETNAM
        lda #$01
        ldx BA
        tay           ; $01 means: load to origin address
        jsr $ffba     ; call SETLFS
FASTLOAD_EXOS:
        lda #$0b
        sta $d011
        lda $01
        pha
        lda #$35
        sta $01
        lda #$00
        jsr $ffd5
        lda $ae
        sta $2d
        lda $af
        sta $2e
        pla
        sta $01
        cli
        lxa #$00        ;sys should return with 0 value
        tay
        rts        
load_star_name:
        !text ":*"
load_star_name_len = * - load_star_name
}
fastload_stack_len = * - fastload_stack 

;;;;;;;;;;;;;;;;;;; C64 END ;;;;;;;;;;;;;;;;;;;;;;;
     
        }
        }
        }   ;VIC20
