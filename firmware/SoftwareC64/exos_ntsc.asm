; ---------------------------------------------------------------------------
; exos_ntsc.asm - disassembly of exos_ntsc.prg, reassembles byte-identical
;                 with ACME (acme -o exos_ntsc.prg exos_ntsc.asm)
;
; --- $080d: relocator / block-copy routine -----------------------------
relocate:
        lda #$00
        sta $57
        sta $59
        lda #>$e000
        sta $58
        lda #>$a000
        sta $5a        
        lda #<packed_data
        sta $5b
        lda #>packed_data
        sta $5c
        ldy #$00
copy8k:
        lda ($57),y
        sta ($57),y
        lda ($59),y
        sta ($59),y
        iny
        bne copy8k
        inc $5a
        inc $58
        bne copy8k

blocktab:
        ldy #$00
        lda ($5b),y
        sta $59
        iny
        lda ($5b),y
        beq relocate_done
        sta $5a
        iny
        lda ($5b),y
        tay
        sta $fc
        clc
        lda $5b
        adc #$03
        sta $5b
        lda $5c
        adc #$00
        sta $5c
copyblk:
        dey
        lda ($5b),y
        sta ($59),y
        tya
        bne copyblk
        clc
        lda $5b
        adc $fc
        sta $5b
        lda $5c
        adc #$00
        sta $5c
        bcc blocktab
relocate_done:
        rts

packed_data:
; --- $086d: packed relocation-block table (not code) --------------------
; format per block: dest_word (lo/hi+$c0), length (0 = 256), then <length> data bytes
; table ends with a $00 dest_hi byte
;block_01  ; dest=$e1da len=1 (@ $086d)
;        !word $e1da
;        !byte $01
;        !byte $08
;block_02  ; dest=$e1dc len=1 (@ $0871)
;        !word $e1dc
;        !byte $01
;        !byte $01
;block_03  ; dest=$e47c len=25 (@ $0875)
;        !word $e47c
;        !byte $19
;        !byte $20,$43,$36,$34,$20,$49,$4d,$50,$52,$4f,$56,$45,$44,$20,$42,$59
;        !byte $20,$45,$58,$4f,$53,$20,$56,$33,$20
;block_04  ; dest=$e49b len=3 (@ $0891)
;        !word $e49b
;        !byte $03
;        !byte $20,$42,$59
;block_05  ; dest=$e49f len=8 (@ $0897)
;        !word $e49f
;        !byte $08
;        !byte $4a,$2e,$53,$43,$48,$45,$4d,$4d
;block_06  ; dest=$e4a8 len=1 (@ $08a2)
;        !word $e4a8
;        !byte $01
;        !byte $4c
;block_07  ; dest=$e535 len=1 (@ $08a6)
;        !word $e535
;        !byte $01
;        !byte $0f
;block_08  ; dest=$e5e8 len=2 (@ $08aa)
;        !word $e5e8
;        !byte $02
;        !byte $c9,$fb			;eliminate $fbc9 also ?
;block_09  ; dest=$e5ee len=3 (@ $08af)
;        !word $e5ee
;        !byte $03
;        !byte $20,$eb,$f0		;eliminate $f0eb also ?
;block_10  ; dest=$e5f4 len=2 (@ $08b5)
;        !word $e5f4
;        !byte $02
;        !byte $ff,$f0			;eliminate $f0ff also ?
;block_11  ; dest=$e935 len=3 (@ $08ba)
;        !word $e935
;        !byte $03
;        !byte $20,$b8,$fc		;eliminate $fcb8 also ?
;block_12  ; dest=$e94c len=1 (@ $08c0)
;        !word $e94c
;        !byte $01
;        !byte $20
;block_13  ; dest=$ea15 len=1 (@ $08c4)
;        !word $ea15
;        !byte $01
;        !byte $01
;block_14  ; dest=$eb1d len=1 (@ $08c8)
;        !word $eb1d
;        !byte $01
;        !byte $03
;block_15  ; dest=$ec06 len=4 (@ $08cc)
;        !word $ec06
;        !byte $04
;        !byte $e3,$e0,$e1,$e2
;block_16  ; dest=$ec7b len=4 (@ $08d3)
;        !word $ec7b
;        !byte $04
;        !byte $e7,$e4,$e5,$e6
;block_17  ; dest=$ecd9 len=2 (@ $08da)
;        !word $ecd9
;        !byte $02
;        !byte $00,$00
;block_18  ; dest=$eebb len=41 (@ $08df)
;        !word $eebb
;        !byte $29
;        !byte $78,$84,$01,$a6,$5c,$b1,$5d,$91,$5f,$c8,$d0,$f9,$e6,$5e,$e6,$60
;        !byte $ca,$d0,$f2,$a9,$37,$85,$01,$58,$60,$78,$84,$01,$a6,$5c,$b1,$5d
;        !byte $48,$b1,$5f,$91,$5d,$68,$91,$5f,$c8
;block_19  ; dest=$eee5 len=116 (@ $090b)
;        !word $eee5
;        !byte $74
;        !byte $f3,$e6,$5e,$e6,$60,$ca,$d0,$ec,$a9,$37,$85,$01,$58,$60,$c9,$e4
;        !byte $90,$14,$c9,$e8,$b0,$3d,$38,$e9,$e4,$0a,$0a,$18,$69,$a0,$85,$60
;        !byte $20,$2b,$ef,$4c,$a2,$fc,$c9,$e0,$90,$29,$20,$2b,$ef,$aa,$bd,$47
;        !byte $ee,$e0,$e2,$b0,$04,$85,$60,$d0,$06,$85,$5e,$a9,$04,$85,$60,$4c
;        !byte $94,$fc,$a8,$ac,$a8,$ac,$a0,$00,$84,$5d,$84,$5f,$a2,$04,$86,$5e
;        !byte $86,$5c,$60,$c9,$16,$d0,$09,$20,$72,$ef,$20,$a2,$fc,$4c,$6a,$ef
;        !byte $c9,$17,$d0,$06,$20,$72,$ef,$4c,$94,$fc,$c9,$0c,$18,$d0,$3c,$20
;        !byte $72,$ef,$a5,$5d
;block_20  ; dest=$ef5a len=90 (@ $0982)
;        !word $ef5a
;        !byte $5a
;        !byte $5f,$85,$5f,$86,$5d,$a5,$5e,$a6,$60,$85,$60,$86,$5e,$20,$94,$fc
;        !byte $a9,$0f,$8d,$77,$02,$e6,$c6,$60,$a9,$00,$85,$5f,$a5,$2b,$a6,$2c
;        !byte $85,$5d,$86,$5e,$a2,$ff,$e8,$bd,$aa,$ef,$20,$d2,$ff,$d0,$f7,$20
;        !byte $42,$f1,$f0,$fb,$c9,$30,$90,$60,$c9,$34,$b0,$5c,$20,$d2,$ff,$aa
;        !byte $a9,$0d,$20,$d2,$ff,$bd,$ba,$ef,$85,$5c,$bd,$be,$ef,$85,$60,$60
;        !byte $0d,$3c,$12,$30,$92,$3e,$20,$32,$30,$4b
;block_21  ; dest=$efb5 len=151 (@ $09df)
;        !word $efb5
;        !byte $97
;        !byte $42,$30,$2d,$46,$46,$0d,$3c,$12,$31,$92,$3e,$20,$34,$4b,$20,$42
;        !byte $30,$2d,$42,$46,$0d,$3c,$12,$32,$92,$3e,$20,$34,$4b,$20,$43,$30
;        !byte $2d,$43,$46,$0d,$3c,$12,$33,$92,$3e,$20,$31,$32,$4b,$20,$44,$30
;        !byte $2d,$46,$46,$0d,$00,$50,$10,$10,$30,$b0,$b0,$c0,$d0,$c9,$18,$d0
;        !byte $06,$20,$34,$f0,$4c,$c3,$a6,$c9,$1a,$d0,$41,$20,$34,$f0,$a2,$32
;        !byte $c6,$60,$a0,$fd,$b1,$5f,$f0,$03,$88,$d0,$f9,$c8,$84,$5e,$18,$a5
;        !byte $5f,$65,$5e,$85,$5f,$a5,$60,$69,$00,$85,$60,$c5,$2c,$f0,$06,$ca
;        !byte $d0,$de,$4c,$c3,$a6,$a5,$2b,$85,$5f,$a5,$2c,$85,$60,$d0,$f3,$c6
;        !byte $60,$a0,$ff,$b1,$5f,$08,$e6,$60,$28,$d0,$ea,$60,$c9,$06,$d0,$03
;        !byte $4c,$32,$f7,$aa,$68,$68,$8a
;block_22  ; dest=$f04d len=85 (@ $0a79)
;        !word $f04d
;        !byte $55
;        !byte $20,$20,$20,$8a,$53,$d9,$34,$30,$39,$36,$2a,$31,$32,$9d,$00,$9d
;        !byte $85,$93,$4c,$c9,$00,$0d,$86,$52,$55,$4e,$3a,$00,$0d,$8c,$43,$4c
;        !byte $cf,$37,$3a,$4f,$d0,$37,$2c,$38,$2c,$31,$35,$2c,$22,$22,$00,$14
;        !byte $87,$4c,$4f,$41,$44,$22,$22,$00,$14,$8b,$53,$41,$56,$45,$22,$22
;        !byte $00,$14,$89,$53,$d9,$33,$32,$37,$36,$38,$00,$0d,$88,$93,$4c,$cf
;        !byte $22,$24,$22,$2c,$38
;block_23  ; dest=$f0a3 len=1 (@ $0ad1)
;        !word $f0a3
;        !byte $01
;        !byte $0d
;block_24  ; dest=$f0d9 len=23 (@ $0ad5)
;        !word $f0d9
;        !byte $17
;        !byte $4c,$cf,$22,$3a,$2a,$22,$2c,$38,$2c,$31,$3a,$58,$20,$08,$f9,$4c
;        !byte $28,$f5,$a2,$00,$bd,$d8,$f0
;block_25  ; dest=$f0f1 len=21 (@ $0aef)
;        !word $f0f1
;        !byte $15
;        !byte $d2,$ff,$e8,$e0,$0c,$d0,$f5,$a2,$06,$78,$60,$ea,$ff,$ff,$ff,$0d
;        !byte $52,$55,$4e,$3a,$0d
;block_26  ; dest=$f387 len=1 (@ $0b07)
;        !word $f387
;        !byte $01
;        !byte $08			;skip tape devices
;block_27  ; dest=$f389 len=2 (@ $0b0b)
;        !word $f389
;        !byte $02
;        !byte $90,$f3
;block_28  ; dest=$f409 len=61 (@ $0b10)
;        !word $f409
;        !byte $3d
;        !byte $60,$20,$20,$f4,$d0,$03,$a9,$ff,$60,$4c,$02,$fd,$ea,$20,$20,$f4
;        !byte $d0,$f7,$68,$68,$4c,$66,$fe,$a2,$00,$8e,$03,$dc,$ca,$8e,$02,$dc
;        !byte $a9,$7f,$8d,$00,$dc,$ad,$01,$dc,$c9,$fb,$60,$a5,$00,$29,$3f,$85
;        !byte $00,$6c,$00,$a0,$a5,$00,$29,$3f,$85,$00,$6c,$02,$a0
;block_29  ; dest=$f4b7 len=1 (@ $0b50)
;        !word $f4b7
;        !byte $01
;        !byte $f7			;skip tape

block_30  ; dest=$f4c9 len=2 (@ $0b54)
        !word $f4c9
        !byte $02
        !byte $2c,$f7
block_31  ; dest=$f4fa len=2 (@ $0b59)
        !word $f4fa
        !byte $02
        !byte $4a,$f7
block_32  ; dest=$f550 len=6 (@ $0b5e)
        !word $f550
        !byte $06
        !byte $ea,$ea,$ea,$a9,$02,$60
block_33  ; dest=$f5db len=2 (@ $0b67)
        !word $f5db
        !byte $02
        !byte $9a,$fb
block_34  ; dest=$f5f9 len=1 (@ $0b6c)
        !word $f5f9
        !byte $01
        !byte $f7
block_35  ; dest=$f72c len=62 (@ $0b70)
        !word $f72c
        !byte $3e
        !byte $20,$32,$f7,$4c,$d5,$f3,$a5,$ba,$20,$0c,$ed,$a9,$ff,$20,$b9,$ed
        !byte $a0,$06,$b9,$e3,$f8,$20,$dd,$ed,$88,$10,$f7,$4c,$fe,$ed,$24,$00
        !byte $50,$03,$4c,$e1,$ff,$a0,$00,$b1,$bb,$c9,$24,$d0,$09,$a5,$9d,$c9
        !byte $80,$d0,$ef,$4c,$1e,$fb,$68,$68,$a9,$ea,$85,$5d,$a9,$f8
block_36  ; dest=$f76b len=81 (@ $0bb1)
        !word $f76b
        !byte $51
        !byte $5e,$a9,$00,$85,$5f,$a9,$04,$85,$60,$a9,$0f,$85,$b6,$a5,$ba,$20
        !byte $0c,$ed,$a9,$ff,$20,$b9,$ed,$a0,$02,$b9,$da,$f8,$20,$dd,$ed,$88
        !byte $10,$f7,$a5,$5f,$20,$dd,$ed,$a5,$60,$20,$dd,$ed,$a9,$23,$20,$dd
        !byte $ed,$a0,$00,$b1,$5d,$20,$dd,$ed,$c8,$c0,$23,$d0,$f6,$20,$fe,$ed
        !byte $18,$a5,$5d,$69,$23,$85,$5d,$a5,$5e,$69,$00,$85,$5e,$a5,$5f,$69
        !byte $23
block_37  ; dest=$f7bd len=26 (@ $0c05)
        !word $f7bd
        !byte $1a
        !byte $5f,$a5,$60,$69,$00,$85,$60,$c6,$b6,$d0,$b0,$a5,$ba,$20,$0c,$ed
        !byte $a9,$ff,$20,$b9,$ed,$a0,$00,$b9,$dd,$f8
block_38  ; dest=$f7d8 len=49 (@ $0c22)
        !word $f7d8
        !byte $31
        !byte $dd,$ed,$c8,$c0,$05,$d0,$f5,$20,$fe,$ed,$78,$ad,$00,$dd,$85,$5a
        !byte $ad,$11,$d0,$29,$ef,$8d,$11,$d0,$38,$a5,$ae,$e9,$02,$85,$58,$a5
        !byte $af,$e9,$00,$85,$59,$20,$87,$ea,$a4,$c6,$f0,$0d,$b9,$76,$02,$c9
        !byte $03
block_39  ; dest=$f80a len=20 (@ $0c56)
        !word $f80a
        !byte $14
        !byte $06,$20,$a3,$f8,$4c,$36,$f6,$2c,$00,$dd,$30,$12,$70,$06,$20,$a3
        !byte $f8,$4c,$04,$f7
block_40  ; dest=$f81f len=54 (@ $0c6d)
        !word $f81f
        !byte $36
        !byte $a3,$f8,$a9,$40,$85,$90,$4c,$a9,$f5,$70,$e7,$a9,$20,$8d,$00,$dd
        !byte $2c,$00,$dd,$50,$fb,$a9,$00,$8d,$00,$dd,$85,$b5,$20,$b1,$f8,$a9
        !byte $fe,$85,$a4,$a5,$5f,$85,$95,$0a,$85,$b4,$26,$b5,$18,$a5,$5f,$65
        !byte $59,$aa,$38,$a5,$58,$e5
block_41  ; dest=$f857 len=148 (@ $0ca6)
        !word $f857
        !byte $94
        !byte $5b,$8a,$e5,$b5,$85,$5c,$a6,$5e,$f0,$0f,$ca,$86,$a4,$8a,$18,$65
        !byte $5b,$85,$ae,$a5,$5c,$69,$00,$85,$af,$a0,$00,$a5,$95,$f0,$04,$a5
        !byte $5d,$91,$5b,$c8,$84,$b6,$20,$b1,$f8,$a4,$b6,$a2,$03,$c4,$a4,$b0
        !byte $0c,$a5,$95,$d0,$04,$c0,$01,$f0,$04,$b5,$5d,$91,$5b,$c8,$c0,$fe
        !byte $b0,$07,$ca,$10,$e8,$84,$b6,$d0,$dd,$4c,$fd,$f7,$a5,$5a,$8d,$00
        !byte $dd,$ad,$11,$d0,$09,$10,$8d,$11,$d0,$60,$2c,$00,$dd,$70,$fb,$a0
        !byte $03,$ea,$a6,$01,$ad,$00,$dd,$4a,$4a,$c1,$ea,$0d,$00,$dd,$4a,$4a	; $c1 -> $ea : PAL
        !byte $ea,$ea,$0d,$00,$dd,$4a,$4a,$ea,$ea,$0d,$00,$dd,$99,$5d,$00,$88
        !byte $10,$e2,$60,$57,$2d,$4d,$4d,$2d,$45,$00,$04,$20,$0f,$01,$1c,$07
        !byte $57,$2d,$4d,$a5
block_42  ; dest=$f8ec len=45 (@ $0d3d)
        !word $f8ec
        !byte $2d
        !byte $85,$06,$a5,$19,$85,$07,$a9,$00,$85,$fb,$8d,$00,$18,$a9,$4c,$8d
        !byte $00,$03,$a9,$3b,$8d,$01,$03,$a9,$04,$8d,$02,$03,$a9,$e0,$85,$00
        !byte $a5,$00,$30,$fc,$c9,$02,$90,$e5,$c9,$08,$d0,$04,$a9
block_43  ; dest=$f91a len=63 (@ $0d6d)
        !word $f91a
        !byte $3f
        !byte $d0,$02,$a9,$0a,$8d,$00,$18,$78,$4c,$22,$eb,$a5,$43,$85,$fa,$20
        !byte $9b,$05,$20,$56,$f5,$50,$fe,$b8,$ad,$01,$1c,$91,$30,$c8,$c0,$05
        !byte $d0,$f3,$a0,$00,$20,$e8,$f7,$a6,$f9,$a5,$53,$95,$cf,$a5,$54,$95
        !byte $ba,$a9,$ff,$95,$e4,$c6,$fa,$d0,$d6,$a9,$01,$85,$fc,$a6,$07
block_44  ; dest=$f95a len=100 (@ $0daf)
        !word $f95a
        !byte $64
        !byte $fb,$95,$e4,$e6,$fb,$b5,$cf,$f0,$0c,$c5,$06,$d0,$08,$e6,$fc,$b5
        !byte $ba,$aa,$4c,$6f,$04,$c9,$24,$90,$05,$a9,$0f,$4c,$98,$05,$85,$06
        !byte $b5,$ba,$85,$07,$20,$9b,$05,$a6,$f9,$a9,$ff,$d5,$e4,$f0,$f5,$20
        !byte $56,$f5,$50,$fe,$b8,$ad,$01,$1c,$91,$30,$c8,$d0,$f5,$a0,$ba,$50
        !byte $fe,$b8,$ad,$01,$1c,$99,$00,$01,$c8,$d0,$f4,$20,$e8,$f7,$a5,$53
        !byte $f0,$04,$a9,$00,$85,$54,$a6,$f9,$b5,$e4,$85,$53,$a9,$ff,$95,$e4
        !byte $a9,$00,$85,$34
block_45  ; dest=$f9bf len=59 (@ $0e16)
        !word $f9bf
        !byte $3b
        !byte $fa,$20,$d0,$f6,$a9,$41,$85,$36,$a9,$08,$8d,$00,$18,$a0,$00,$a2
        !byte $00,$ad,$00,$18,$4a,$b0,$09,$88,$d0,$f7,$ca,$d0,$f4,$4c,$96,$05
        !byte $a0,$00,$8c,$00,$18,$a4,$fa,$b1,$30,$4a,$4a,$4a,$85,$5c,$b1,$30
        !byte $29,$07,$85,$5d,$c8,$d0,$06,$a0,$ba,$a9,$01
block_46  ; dest=$f9fb len=94 (@ $0e54)
        !word $f9fb
        !byte $5e
        !byte $31,$b1,$30,$0a,$26,$5d,$0a,$26,$5d,$4a,$4a,$4a,$85,$5a,$b1,$30
        !byte $4a,$c8,$b1,$30,$2a,$2a,$2a,$2a,$2a,$29,$1f,$85,$5b,$b1,$30,$29
        !byte $0f,$85,$58,$c8,$b1,$30,$0a,$26,$58,$4a,$4a,$4a,$85,$59,$b1,$30
        !byte $0a,$0a,$0a,$29,$18,$85,$56,$c8,$d0,$06,$a0,$ba,$a9,$01,$85,$31
        !byte $b1,$30,$2a,$2a,$2a,$2a,$29,$07,$05,$56,$85,$56,$b1,$30,$29,$1f
        !byte $85,$57,$c8,$84,$fa,$a0,$08,$8c,$00,$18,$b6,$55,$bd,$c4
block_47  ; dest=$fa5a len=85 (@ $0eb5)
        !word $fa5a
        !byte $55
        !byte $8d,$00,$18,$bd,$dc,$05,$b6,$54,$8d,$00,$18,$88,$d0,$ef,$c6,$36
        !byte $f0,$03,$4c,$f7,$04,$8c,$00,$18,$c6,$fc,$f0,$03,$4c,$94,$04,$a5
        !byte $06,$f0,$03,$4c,$9e,$fd,$a9,$08,$4c,$69,$f9,$a9,$03,$85,$31,$a9
        !byte $00,$85,$30,$20,$56,$f5,$50,$fe,$b8,$ad,$01,$1c,$c9,$52,$d0,$f3
        !byte $c8,$50,$fe,$b8,$ad,$01,$1c,$91,$30,$c8,$c0,$04,$d0,$f3,$a0,$02
        !byte $20,$2b,$f8,$a5,$54
block_48  ; dest=$fab0 len=26 (@ $0f0d)
        !word $fab0
        !byte $1a
        !byte $f9,$60,$a9,$0f,$d0,$cc,$00,$0a,$0a,$02,$00,$0a,$0a,$02,$00,$00
        !byte $08,$00,$00,$00,$08,$00,$00,$02,$08,$00
block_49  ; dest=$facb len=120 (@ $0f2a)
        !word $facb
        !byte $78
        !byte $02,$08,$00,$00,$08,$0a,$0a,$00,$00,$02,$02,$00,$00,$0a,$0a,$00
        !byte $00,$02,$02,$00,$08,$08,$08,$00,$00,$00,$00,$03,$85,$f9,$60,$a9
        !byte $0f,$d0,$98,$ea,$ea,$ea,$ea,$ea,$ea,$ea,$ea,$ea,$ea,$ea,$ea,$ea
        !byte $ea,$ea,$ea,$ea,$ea,$ea,$ea,$ea,$ea,$ea,$ea,$ea,$ea,$ea,$ea,$ea
        !byte $ea,$ea,$ea,$ea,$ea,$ea,$ea,$ea,$ea,$ea,$ea,$ea,$ea,$ea,$ea,$ea
        !byte $ea,$ea,$ea,$68,$68,$a9,$00,$85,$b4,$a9,$0d,$20,$d2,$ff,$ea,$a9
        !byte $00,$85,$90,$a0,$02,$84,$a9,$20,$13,$ee,$85,$aa,$20,$e1,$ff,$d0
        !byte $03,$4c,$33,$f6,$a4,$90,$d0,$2f
block_50  ; dest=$fb44 len=39 (@ $0fa5)
        !word $fb44
        !byte $27
        !byte $13,$ee,$a4,$90,$d0,$28,$a4,$a9,$88,$d0,$e1,$a6,$aa,$20,$cd,$bd
        !byte $a9,$20,$20,$d2,$ff,$20,$13,$ee,$a6,$90,$d0,$12,$aa,$f0,$06,$20
        !byte $7a,$fb,$4c,$59,$fb,$a9,$0d
block_51  ; dest=$fb6c len=2 (@ $0fcf)
        !word $fb6c
        !byte $02
        !byte $d2,$ff
block_52  ; dest=$fb6f len=28 (@ $0fd4)
        !word $fb6f
        !byte $1c
        !byte $02,$d0,$be,$20,$42,$f6,$a6,$2d,$a4,$2e,$60,$20,$d2,$ff,$c9,$22
        !byte $d0,$f8,$a5,$b4,$49,$ff,$85,$b4,$d0,$f0,$a9,$3a
block_53  ; dest=$fb8c len=2 (@ $0ff3)
        !word $fb8c
        !byte $02
        !byte $d2,$ff
block_54  ; dest=$fb97 len=87 (@ $0ff8)
        !word $fb97
        !byte $57
        !byte $00,$00,$60,$c0,$59,$d0,$03,$4c,$2b,$f1,$a0,$49,$20,$2b,$f1,$24
        !byte $9d,$10,$1a,$a5,$01,$29,$07,$c9,$07,$d0,$12,$a2,$fc,$bd,$c9,$fa
        !byte $20,$d2,$ff,$e8,$d0,$f7,$a5,$af,$a6,$ae,$20,$cd,$bd,$60,$20,$54
        !byte $4f,$20,$20,$b4,$e5,$a6,$00,$30,$2f,$a6,$d8,$d0,$2b,$a6,$d4,$d0
        !byte $27,$a6,$9d,$f0,$23,$c9,$85,$90,$20,$c9,$8d,$b0,$1c,$85,$3c,$a2
        !byte $ff,$e8,$bd,$4e,$f0,$c5,$3c
block_55  ; dest=$fbef len=41 (@ $1052)
        !word $fbef
        !byte $29
        !byte $f8,$e8,$bd,$4e,$f0,$f0,$05,$20,$16,$e7,$90,$f5,$e8,$bd,$4e,$f0
        !byte $60,$20,$06,$fc,$a9,$00,$60,$c9,$0f,$d0,$34,$a5,$2b,$a4,$2c,$85
        !byte $22,$84,$23,$a0,$03,$c8,$b1,$22,$d0
block_56  ; dest=$fc19 len=6 (@ $107e)
        !word $fc19
        !byte $06
        !byte $c8,$98,$18,$65,$22,$a0
block_57  ; dest=$fc20 len=94 (@ $1087)
        !word $fc20
        !byte $5e
        !byte $91,$2b,$a5,$23,$69,$00,$c8,$91,$2b,$20,$33,$a5,$a5,$22,$69,$02
        !byte $85,$2d,$a5,$23,$69,$00,$85,$2e,$20,$63,$a6,$4c,$7b,$e3,$c9,$15
        !byte $d0,$17,$a9,$4d,$a0,$fc,$8d,$32,$03,$8c,$33,$03,$60,$a9,$36,$85
        !byte $01,$20,$ed,$f5,$a9,$37,$85,$01,$60,$c9,$01,$d0,$07,$a5,$00,$09
        !byte $40,$85,$00,$60,$c9,$0b,$d0,$29,$a9,$06,$a2,$08,$a0,$0f,$20,$ba
        !byte $ff,$a9,$00,$20,$bd,$ff,$20,$c0,$ff,$a2,$06,$20,$c6,$ff
block_58  ; dest=$fc7f len=9 (@ $10e8)
        !word $fc7f
        !byte $09
        !byte $cf,$ff,$20,$d2,$ff,$24,$90,$50,$f6
block_59  ; dest=$fc89 len=58 (@ $10f4)
        !word $fc89
        !byte $3a
        !byte $cc,$ff,$a9,$06,$20,$c3,$ff,$60,$4c,$f3,$ee,$a0,$20,$b9,$ba,$ee
        !byte $99,$09,$02,$88,$d0,$f7,$4c,$0a,$02,$a0,$20,$b9,$d3,$ee,$99,$09
        !byte $02,$88,$d0,$f7,$4c,$0a,$02,$f7,$4c,$0a,$02,$00,$00,$00,$00,$ee
        !byte $a5,$02,$ad,$8d,$02,$29,$02,$d0,$f9,$60
block_60  ; dest=$fce8 len=2 (@ $1131)
        !word $fce8
        !byte $02
        !byte $0a,$f4
;block_61  ; dest=$fcff len=3 (@ $1136)
;        !word $fcff
;        !byte $03
;        !byte $4c,$34,$f4
;block_62  ; dest=$fd68 len=36 (@ $113c)
;        !word $fd68
;        !byte $24
;        !byte $ad,$ff,$9f,$a2,$ff,$8e,$ff,$9f,$ec,$ff,$9f,$d0,$10,$e8,$8e,$ff
;        !byte $9f,$ec,$ff,$9f,$d0,$07,$8d,$ff,$9f,$a0,$a0,$d0,$07,$8d,$ff,$9f
;        !byte $a0,$80,$a2,$00
;block_63  ; dest=$fe57 len=2 (@ $1163)
;        !word $fe57
;        !byte $02
;        !byte $16,$f4
;block_64  ; dest=$fe6f len=3 (@ $1168)
;        !word $fe6f
;        !byte $03
;        !byte $4c,$3d,$f4
;block_65  ; dest=$ff00 len=1 (@ $116e)
;        !word $ff00
;        !byte $01
;        !byte $df
blocktab_end  ; @ $1172
        !byte $00,$00,$00,$00,$00
