        !to "selector.prg",cbm
        !cpu 6510

        * = $02a8
        !bin "plus4boot.bin",,2
start:        
        lda $e000
        ;cmp #$20
        ;beq plus4
        cmp #$0f
        beq vic20
        ;c64
        lda $02a6       ;pal/ntsc flag
        bne is_pal
        lda #'N'
        sta filename64
is_pal:        
        ldx #<filename64
        bne cont
vic20:
        ldx #<filenamevic
;        bne cont
;c128:   ldx #<filename128
;        bne cont
;plus4:  
;        lda #$2c
;        sta plus4_disable
;        ldx #<filenameplus4
cont:
        lda #1
        ldy #>filename64
        jsr $ffbd     ; call SETNAM
        inx
        ldy #$fc
-       lda filename64 & $ff00,x
        sta $0300-$fc,y
        inx
        iny
        bne -
        tya
-       jsr $ffd5

        ldx #4
;        stx $ef
        stx $c6
-       lda keys-1,x
;        sta $0527-1,x
plus4_disable:
        sta $0277-1,x
        dex
        bne -
        jmp ($0302)

keys:
        !byte $0d
        !byte 'R'
        !byte 'u'
        !byte $0d
        
filename64:
        !text "P"
        !byte $8b,$e3,$83,$a4

;filename128:
;        !text "1"
;        !byte $3f,$4d,$c6,$4d

filenamevic:
        !text "V"
        !byte $3a,$c4,$93,$c4

;filenameplus4:
;        !text "+"
;        !byte $86,$86,$12,$87


        * = $0300
        !word start
        !word start
