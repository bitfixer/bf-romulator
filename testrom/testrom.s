.segment    "CODE"

    lda     #$AA
    sta     $E800
    sta     $E801
    sta     $E802
    sta     $E803
    sta     $E804
    sta     $E805
    sta     $E806

; check the zero page first, will be used for later tests

zeropagecheck:
    ldx     #$00
    txa
    sta     $E800   ; use overridden io space as scratch memory

zeropagewrite:
    sta     $00,X   ; fill zero page with accumulator value
    inx
    bne     zeropagewrite

zeropageverify:
    stx     $E801
    lda     $00,X   ; load value from zero page address
    cmp     $E800
    bne     zeropagemismatch
    inx
    bne     zeropageverify

zeropageincrement:
    ldy     $E800   ; load and increment current test value
    iny
    beq     zeropagesuccess
    sty     $E800
    tya
    jmp     zeropagewrite

zeropagemismatch:
    sta     $E802
    jmp     done

zeropagesuccess:

; now do ram check
    
    lda     #$00
    sta     $00FB   ; low byte of ram address
    sta     $E803   ; store current test byte, starting at 0

    lda     #$01
    sta     $00FC   ; store address for start of ram check
    sta     $E802   ; store copy of ram address

    ldy     #$00    ; load offset into y
    tya

; check each block of 256 bytes
ramwrite:
    sta     ($FB),y ; write byte to RAM
    iny
    bne     ramwrite

; read back the ram

ramread:
    sty     $E804   ; store the current address offset
    lda     ($FB),y ; read byte from RAM
    cmp     $E803   ; compare with test byte
    bne     rammismatch
    iny
    bne     ramread

ramincrement:
; increment test value
    lda     $E803   ; load test byte
    eor     #$FF    ; flip all bits
    sta     $E803   ; save new test byte
    beq     rampagesuccess  ; done if we have tested all bits
    jmp     ramwrite

rampagesuccess:
    ; current 256 byte page success
    ; increment address

    ldx     $00FC
    inx
    cpx     #$80
    beq     ramtestdone
    stx     $00FC   ; write incremented address
    stx     $E802   ; write copy of address
    jmp     ramwrite    ; test the next ram page

rammismatch:
    ; record the value read
    sta     $E805
    lda     #$BB
    sta     $E808
    jmp     romtest
    
ramtestdone:
    lda     #$CC
    sta     $E808

romtest:
    lda     #$00    ; start of rom space
    sta     $00FB
    lda     #$90
    sta     $00FC
    sta     $00FD
    ldy     #$00

loop:
    lda     ($FB),y  ; read byte from ROM
    sta     ($FB),y  ; write byte back to ROM
    iny
    bne     loop

    ldx     $00FC
    inx
    stx     $00FC
    cpx     #$FF
    beq     done
    jmp     loop

done:
    nop
    jmp     done    ; wait here

