.segment    "CODE"

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

