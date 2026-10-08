; ------------------------------------------------------------------
; vic20loader.v1.asm -- disassembled from vic20loader.v1.prg
; Load address $1400, 1408 bytes of code/data.
; ACME-style syntax ("* = $addr", "!byte", "!word", "!fill").
;
; Overview:
;   VIC-20 side installer for a custom 1541 fast-loader protocol.
;   It patches 13 parameter bytes into a drive-side code block,
;   then sends an "I" (initialize) DOS command and repeated
;   "M-W"/"M-E" (memory write/execute) DOS commands over the
;   serial bus (via KERNAL LISTEN/SECOND/CIOUT/UNLSN) to install
;   and launch that code inside the 1541's own RAM.
; ------------------------------------------------------------------

WITH_START_BYTE = 1

LISTEN  = $FFB1
SECOND  = $FF93
CIOUT   = $FFA8
UNLSN   = $FFAE
;VIC_AUX_COLOR = $900f


init_vc20_load:
L1400:  JSR L1431
        JSR L1476
        LDA #(floppy_code_len+$1f)/$20
L1408:  PHA
        JSR L1489
        PLA
        SEC
        SBC #$01
        BNE L1408
;        JSR L14DC
;        LDY #$80
;        LDX #$00
;L1419:  DEX
;        BNE L1419
;        DEY
;        BNE L1419
;        LDA #$02
;        LDY #$00
;        LDX #$00
;L1425:  DEX
;        BNE L1425
;        DEY
;        BNE L1425
;        SEC
;        SBC #$01
;        BNE L1425
        RTS

L1431:  STA $FF
        LDA #$00
        STA $FE
        LDY #$00
L1451:  LDA STCK_CODE,Y
        STA ($FE),Y
        INY
        CPY #STCK_CODE_LEN
        BNE L1451
        RTS


;L1476:  LDA $ba
;        JSR LISTEN
;        LDA #$6F
;        JSR SECOND
;        LDA #$49
;        JSR CIOUT
;        JSR UNLSN
;        RTS

L1489:  LDA $ba
        JSR LISTEN
        LDA #$6F
        JSR SECOND
        LDX #$00
L1495:  LDA L14F7,X
        JSR CIOUT
        INX
        CPX #$06
        BNE L1495
        LDX #$00
L14A2:  LDA floppy_code,X
        ;PHA
        ;AND #$07
        ;ORA #$18
        ;STA VIC_AUX_COLOR
        ;PLA
        JSR CIOUT
        INX
        CPX #$20
        BNE L14A2
        JSR UNLSN
        CLC
        LDA #$20
        ADC L14F7+3       ; advance M-W command's target address low byte
        STA L14F7+3
        LDA #$00
        ADC L14F7+4       ; ...and high byte
        STA L14F7+4
        CLC
        LDA #$20
        ADC L14A2+1       ; advance floppy_code read pointer low byte
        STA L14A2+1
        LDA #$00
        ADC L14A2+2       ; ...and high byte
        STA L14A2+2
        RTS

;L14DC:  LDA $ba
;        JSR LISTEN
;        LDA #$6F
;        JSR SECOND
;        LDX #$00
;L14E8:  LDA L14FD,X
;        JSR CIOUT
;        INX
;        CPX #$05
;        BNE L14E8
;        JSR UNLSN
;        RTS

;L14FD:  !byte $4D,$2D,$45,<L041A, >L041A

L14F7:  !byte $4D,$2D,$57,<L0300, >L0300,$20



STCK_CODE:
        !pseudopc $0400 {
vicload:
        !ifdef WITH_START_BYTE {
        ; carry clear -> load at address in file
        bcc no_addr
        }
L1600:  STX L16A0+1       ; init screen-write pointer low byte
L1603:  STY L16A0+2       ; init screen-write pointer high byte
        !ifdef WITH_START_BYTE {
        tax
no_addr:ror
        sta $0a
        txa 
        }
        LDX #$80
L1608:  DEX
        BNE L1608
        LDX #$08
L160D:  LDY #$DC
        LSR
        BCC L1614
        LDY #$FE
L1614:  STY $912C
        PHP
        PHA
        TXA
        LSR
        LDA #$FE
        BCC L1621
        LDA #$7E
L1621:  STA $911F
        PLA
        PLP
L1626:  JSR L1631
L1629:  JSR L1631
        DEX
        BNE L160D
        BCC L1632
!ifdef WITH_START_BYTE {
        ;ldx L16A0+1
        ;ldy L16A0+2
}
L1631:  RTS

        !ifdef WITH_START_BYTE {
L1632_pre:
        lda #$89
        sta dex_skip
        }
        
L1632:  LDA #$20
L1634:  EOR L1644
L1637:  STA L1644         ; flips opcode byte: toggles BEQ/BNE below
        LDA #$FE
        STA $911F
        LDA #$01
L1641:  BIT $911F
L1644:  BEQ L1641
        LDA #$7E
        STA $911F
        LDX #$0A
L164D:  DEX
        BNE L164D
        LDX #$FE
L1652:  LDA #$FF
        LDY #$FE
        AND $911F
        STY $911F
        SEC
        ROL
        SEC
        ROL
        CMP ($00,X)
        LDY #$7E
        AND $911F
        STY $911F
        SEC
        ROL
        SEC
        ROL
        CMP ($00,X)
        LDY #$FE
        AND $911F
        STY $911F
        SEC
        ROL
        SEC
        ROL
        CMP ($00,X)
        LDY #$7E
        AND $911F
        STY $911F
        CMP ($00,X)
        CMP ($00,X)
        CPX #$FE
lap_jump:
        !ifdef WITH_START_BYTE {
        BCC l_next1
        } else {
        BCC L16A0
        }
        CPX #$FE
        BNE L1695
L1692:  STA L16A4+1       ; patches CPX #imm compare value at L16A4
L1695:  STA L16B5+1       ; patches LDA #imm value at L16B5
        CMP ($00,X)
        CMP ($00,X)
        INX
L169D:  JMP L1652
L16A0:  STA $0400,X       ; L16A0+1/+2 double as the screen-write pointer
        INX
L16A4:  CPX #$00
        BNE L1652
        CLC
        !ifdef WITH_START_BYTE {
dex_skip:
        dex
        dex
        }
        TXA
L16AA:  ADC L16A0+1
L16AD:  STA L16A0+1
        BCC L16B5
L16B2:  INC L16A0+2
L16B5:  LDA #$00
        BNE L16BC
L16B9:  
        !ifdef WITH_START_BYTE {
        JMP L1632_pre
        } else {
        JMP L1632
        }

        !ifdef WITH_START_BYTE {
l_next1:
        bit $0a
        bmi no_load_at_addr1
        sta L16A0+1
        sta $2b
no_load_at_addr1:
        lda #l_next2- (lap_jump+2)
        sta lap_jump+1
        bne L1652

l_next2:
        bit $0a
        bmi no_load_at_addr2
        sta L16A0+2
        sta $2c
no_load_at_addr2:
        lda #L16A0- (lap_jump+2)
        sta lap_jump+1
        lda #$24
        sta lpatch_beq_away
        jmp L1652
        }

L16BC:  
        !ifdef WITH_START_BYTE {
        lda #$ca
        sta dex_skip
        lda #l_next1- (lap_jump+2)
        sta lap_jump+1
        }
        RTS

        }


STCK_CODE_LEN = * - STCK_CODE

floppy_code:
        !pseudopc $0300 {
        
        READ_BUFFER = $0700
        
L0300:

        LDA #$07
        STA $31
        JSR $F50A
        LDY #$00
L0309:  BVC L0309
        CLV
        LDA $1C01
        STA READ_BUFFER,Y
        INY
        BNE L0309
        LDY #$BA
L0317:  BVC L0317
        CLV
        LDA $1C01
        STA $0100,Y
        INY
        BNE L0317
        JSR $F8E0
        !ifdef WITH_START_BYTE {
        lxa #$00
        } else {
        LDA #$00
        TAX
        }
L0329:  EOR READ_BUFFER,X
        INX
        BNE L0329
        CMP $3A
        BNE L0300
        LDA READ_BUFFER+1
        STA $07
        LDX #$FE
        LDY #$00
        LDA READ_BUFFER
        STA $06
        BNE L0348
        LDX $07
        DEX
        LDY #$FF
L0348:  STX READ_BUFFER
        STY READ_BUFFER+1
        JSR L0378
L0351:  JSR L0393         ; self-modified: target patched to L0393/L0381/L048A below
        JSR L0378
        LDA #<L0393
        STA L0351+1
        LDA #>L0393
        STA L0351+2       ; -> L0393 (restore default)
        JMP $F505
L0364:  JSR L0378
        JSR L036D
        JMP L0364
L036D:  LDX #$00
        LDY #$00
L0371:  DEY
        BNE L0371
        DEX
        BNE L0371
        RTS
L0378:  LDA #$08
        EOR $1C00
        STA $1C00
        RTS
L0381:  
        !ifdef WITH_START_BYTE {
        ldx #$00
lpatch_beq_away:
        beq L0395
        }
        LDX READ_BUFFER
        LDY READ_BUFFER+1
        DEX
        DEX
        STX READ_BUFFER+2
        STY READ_BUFFER+3
        LDX #$02
        BNE L0395

L0393:  LDX #$00
L0395:  LDY READ_BUFFER
        INY
        INY
        STY L03E6+1       ; patches CPX #imm compare value at L03E6
L039D:  LDA $1800
        BPL L039D
L03A2:  LDA #$00
        EOR #$0A
        STA L03A2+1        ; one-byte variable reusing this LDA's operand
        STA $1800
L03AC:  BIT $1800
        BMI L03AC
L03B1:  LDY READ_BUFFER,X
        LDA L04B2,Y
        TAY
        LSR
        LSR
        LSR
        LSR
L03BC:  BIT $1800
        BMI L03BC
        STA $1800
        ASL
        ORA #$10
L03C7:  BIT $1800
        BPL L03C7
        STA $1800
        TYA
        AND #$0F
L03D2:  BIT $1800
        BMI L03D2
        STA $1800
        ASL
        ORA #$10
L03DD:  BIT $1800
        BPL L03DD
        STA $1800
        INX
L03E6:  CPX #$00
        BNE L03B1
L03EA:  BIT $1800
        BMI L03EA
        LDA L03A2+1
        STA $1800
        RTS
L03F6:  LDA #$00
        LDX #$04
L03FA:  LDY #$10
        STY $1800
L03FF:  BIT $1800
        BPL L03FF
        LSR $1800
        ROR
        LDY #$00
        STY $1800
L040D:  BIT $1800
        BMI L040D
        LSR $1800
        ROR
        DEX
        BNE L03FA
        RTS
        
        ;floppy code entry
L041A:
        SEI
        LDA #$0A
        STA $1800
        JSR L0469
        LDA #$00
        STA $1800
        LDX #$00
L042A:  DEX
        BNE L042A
L042D:  LDA #$F7
        AND $1C00
        STA $1C00
        JSR L03F6
        LDX L03A2+1
        STX $1800
        ASL
        TAX
        BCC L0445
        JMP $EB22   ; ($fffc)
L0445:  LDA L05B2,X
        STA $06
        LDA L05B2+1,X
        STA $07
        LDA #<L0381
        STA L0351+1
        LDA #>L0381
        STA L0351+2       ; -> L0381
        !ifdef WITH_START_BYTE {
        lda #$f0
        sta lpatch_beq_away
        }
L0459:  LDA #$E0
        STA $00
        CLI
L045E:  LDA $00
        BMI L045E
        SEI
        LDA $06
        BNE L0459
        BEQ L042D
L0469:  LDA #$12
        STA $06
        LDA #$01
        STA $07
L0471:  LDA #$E0
        STA $00
        LDA #<L048A
        STA L0351+1
        LDA #>L048A
        STA L0351+2       ; -> L048A
        CLI
L0480:  LDA $00
        BMI L0480
        SEI
        LDA $06
        BNE L0471
        RTS
L048A:  LDX #$00
L048C:  LDA READ_BUFFER+2,X
        AND #$3F
        CMP #$02
        BNE L04AA
L0495:  LDA #$00
        BMI L04AA
        INC L0495+1        ; one-byte variable reusing this LDA's operand
        ASL
        TAY
        LDA READ_BUFFER+3,X
        STA L05B2,Y
        LDA READ_BUFFER+4,X
        STA L05B2+1,Y
L04AA:  
        !ifdef WITH_START_BYTE {
        txa
        sbx #$e0
        } else {
        CLC
        TXA
        ADC #$20
        TAX
        }
        
        BNE L048C
        RTS

L04B2:  !byte $FF,$FB,$FE,$FA,$F7,$F3,$F6,$F2,$FD,$F9,$FC,$F8,$F5,$F1,$F4,$F0
        !byte $BF,$BB,$BE,$BA,$B7,$B3,$B6,$B2,$BD,$B9,$BC,$B8,$B5,$B1,$B4,$B0
        !byte $EF,$EB,$EE,$EA,$E7,$E3,$E6,$E2,$ED,$E9,$EC,$E8,$E5,$E1,$E4,$E0
        !byte $AF,$AB,$AE,$AA,$A7,$A3,$A6,$A2,$AD,$A9,$AC,$A8,$A5,$A1,$A4,$A0
        !byte $7F,$7B,$7E,$7A,$77,$73,$76,$72,$7D,$79,$7C,$78,$75,$71,$74,$70
        !byte $3F,$3B,$3E,$3A,$37,$33,$36,$32,$3D,$39,$3C,$38,$35,$31,$34,$30
        !byte $6F,$6B,$6E,$6A,$67,$63,$66,$62,$6D,$69,$6C,$68,$65,$61,$64,$60
        !byte $2F,$2B,$2E,$2A,$27,$23,$26,$22,$2D,$29,$2C,$28,$25,$21,$24,$20
        !byte $DF,$DB,$DE,$DA,$D7,$D3,$D6,$D2,$DD,$D9,$DC,$D8,$D5,$D1,$D4,$D0
        !byte $9F,$9B,$9E,$9A,$97,$93,$96,$92,$9D,$99,$9C,$98,$95,$91,$94,$90
        !byte $CF,$CB,$CE,$CA,$C7,$C3,$C6,$C2,$CD,$C9,$CC,$C8,$C5,$C1,$C4,$C0
        !byte $8F,$8B,$8E,$8A,$87,$83,$86,$82,$8D,$89,$8C,$88,$85,$81,$84,$80
        !byte $5F,$5B,$5E,$5A,$57,$53,$56,$52,$5D,$59,$5C,$58,$55,$51,$54,$50
        !byte $1F,$1B,$1E,$1A,$17,$13,$16,$12,$1D,$19,$1C,$18,$15,$11,$14,$10
        !byte $4F,$4B,$4E,$4A,$47,$43,$46,$42,$4D,$49,$4C,$48,$45,$41,$44,$40
        !byte $0F,$0B,$0E,$0A,$07,$03,$06,$02,$0D,$09,$0C,$08,$05,$01,$04,$00

L05B2:  !fill 17,$00
        }
floppy_code_len = * - floppy_code
