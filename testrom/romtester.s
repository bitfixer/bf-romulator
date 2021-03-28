; rom tester
; this program just reads every byte in the specified region of the memory map,
; and writes back to the same address.
; In the ROMulator, the memory map is configured to be writethrough.
; Then, after halting the CPU and reading the contents of memory, we can check the contents of the ROMs.

screen_memory = $8000
read_address_low_byte                   =   $FB
read_address_high_byte                  =   $FC

rom_space_start                         =   $90
rom_space_end                           =   $FF

video_ram_start                         =   $80
video_ram_end                           =   $8F

ram_test_mismatch_marker                =   $BB
ram_test_complete_marker                =   $CC
done_marker                             =   $DD

flag_position                           =   $E0
flag_value                              =   $E1

vram_test_done                          =   $F0
vram_page                               =   $F1
vram_byte                               =   $F2
vram_expected                           =   $F3
vram_read                               =   $F4
done_indicator_address                  =   $FA

.segment    "CODE"

; memory addresses 0-1FF are replaced with romulator memory
; so we are not concerned about the RAM in that region being faulty.
; read from this point on in the memory map

; video ram test

    lda     #$00
    sta     read_address_low_byte

    lda     #video_ram_start
    sta     read_address_high_byte

vram_page_init:
    lda     #$03
    sta     flag_position

    lda     #$FF
    sta     flag_value

vram_page_start:
    lda     flag_value
    jsr     write_page
    ldx     flag_position
    jsr     write_alternating_page
    ldx     flag_position
    lda     flag_value
    jsr     compare_alternating_page
    ; check if done
    lda     vram_test_done
    cmp     #ram_test_mismatch_marker
    beq     clear_video_ram_page

    ; first loop flag positions
    dec     flag_position
    beq     next_flag_value
    jmp     vram_page_start

next_flag_value:
    lda     flag_value
    cmp     #$00
    beq     next_vram_page

    lda     #$03
    sta     flag_position
    lda     #$00
    sta     flag_value
    jmp     vram_page_start

next_vram_page:
    ldx     read_address_high_byte
    cpx     #video_ram_end
    beq     clear_video_ram_page
    inx
    stx     read_address_high_byte
    stx     vram_page
    jmp     vram_page_init

write_page:
    ldy     #$00
write_page_loop:
    sta     (read_address_low_byte),Y
    iny
    bne     write_page_loop
    rts

; alternating page of memory
; flip the A value every third byte
; start with position indicated in X
write_alternating_page:
    ldy     #$00
write_alternating_page_loop:
    dex
    bne     no_flip1
    ; flip A value
    eor     #$FF
no_flip1:
    ; store value in address
    sta     (read_address_low_byte),Y
    cpx     #$00
    bne     no_flip2
    eor     #$FF
    ldx     #$03
no_flip2:
    iny
    bne     write_alternating_page_loop
    rts

compare_alternating_page:
    ldy     #$00
compare_alternating_page_loop:
    dex
    bne     no_flip_compare1
    ; flip A value
    eor     #$FF
no_flip_compare1:
    ; store value in address
    cmp     (read_address_low_byte),Y
    bne     vram_test_fault
    cpx     #$00
    bne     no_flip_compare2
    eor     #$FF
    ldx     #$03
no_flip_compare2:
    iny
    bne     write_alternating_page_loop
    rts
vram_test_fault:
    sta     vram_expected
    lda     (read_address_low_byte),Y
    sta     vram_read
    lda     read_address_high_byte
    sta     vram_page
    sty     vram_byte
    lda     #ram_test_mismatch_marker
    sta     vram_test_done
    rts

clear_video_ram_page:
    lda     #$20
    ldx     #$00
video_loop:
    sta     $8000,X
    inx
    bne     video_loop

    lda     vram_page
    ldy     #$0
    jsr     printhex

    lda     vram_byte
    ldy     #$2
    jsr     printhex

; write text
;    lda     #$52
;    sta     $8004

setup:
    lda     #rom_space_start
    sta     read_address_high_byte

    ldy     #$6
    jsr     printhex

nextpage:
    ldy     #$00

readpage:
    lda     (read_address_low_byte),Y
    sta     (read_address_low_byte),Y
    iny
    bne     readpage

    ldx     read_address_high_byte
    cpx     #rom_space_end
    bcs     done
    inx
    stx     read_address_high_byte
    txa
    ldy     #$06
    jsr     printhex
    jmp     nextpage

done:
    lda     #done_marker
    sta     done_indicator_address
doneloop:
    nop
    jmp     doneloop    ; wait here

; printhex
; display a single hex character
; value is in A
; screen offset in Y
printhex:
    tax
    lsr
    lsr
    lsr
    lsr

    jsr     convert_to_hex_text
    sta     screen_memory,Y
    iny
    txa
    and     #$0F
    
    jsr     convert_to_hex_text
    sta     screen_memory,Y
    rts

convert_to_hex_text:
    cmp     #$0A
    bcs     get_letter
    adc     #$30
    rts
get_letter:
    adc     #$36
    rts
