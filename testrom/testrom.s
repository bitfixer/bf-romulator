.segment    "CODE"

    lda     #$AA
    sta     $E800
    sta     $E801
    sta     $E802

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
    lda     #$00    ; start of rom space
    sta     $00FB
    lda     #$B0
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

