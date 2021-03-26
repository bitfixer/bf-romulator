test_address_start = $E800
zero_page_compare_value                 =   test_address_start
zero_page_address                       =   test_address_start + 1
zero_page_mismatch_value                =   test_address_start + 2
ram_test_address                        =   test_address_start + 3
ram_test_address_page                   =   test_address_start + 4
ram_test_compare_value                  =   test_address_start + 5
ram_test_mismatch_value                 =   test_address_start + 6
ram_test_mismatch_indicator_address     =   test_address_start + 7
ram_test_complete_indicator_address     =   test_address_start + 8
done_indicator_address                  =   test_address_start + 9
test_address_end                        =   test_address_start + 9

test_address_count                      =   test_address_end - test_address_start + 1

read_address_low_byte                   =   $FB
read_address_high_byte                  =   $FC

ram_space_start                         =   $01
ram_space_end                           =   $02

rom_space_start                         =   $90
rom_space_end                           =   $FF

ram_test_mismatch_marker                =   $BB
ram_test_complete_marker                =   $CC
done_marker                             =   $DD

.segment    "CODE"

; check the zero page first, will be used for later tests

zeropagecheck:
    ldx     #$00
    txa
    sta     zero_page_compare_value   ; use overridden io space as scratch memory

zeropagewrite:
    sta     $00,X   ; fill zero page with accumulator value
    inx
    bne     zeropagewrite

zeropageverify:
    stx     zero_page_address
    lda     $00,X   ; load value from zero page address
    cmp     zero_page_compare_value
    bne     zeropagemismatch
    inx
    bne     zeropageverify

zeropageincrement:
    ldy     zero_page_compare_value   ; load and increment current test value
    iny
    beq     zeropagesuccess
    sty     zero_page_compare_value
    tya
    jmp     zeropagewrite

zeropagemismatch:
    sta     zero_page_mismatch_value
    jmp     done

zeropagesuccess:
    sta     zero_page_mismatch_value

    jmp     done

; now do ram check
    
    lda     #$00
    sta     read_address_low_byte   ; low byte of ram address
    sta     ram_test_compare_value   ; store current test byte, starting at 0

    lda     #ram_space_start
    sta     read_address_high_byte   ; store address for start of ram check
    sta     ram_test_address_page   ; store copy of ram address

    ldy     #$00    ; load offset into y
    tya

; check each block of 256 bytes
ramwrite:
    sta     (read_address_low_byte),y ; write byte to RAM
    iny
    bne     ramwrite

; read back the ram

ramread:
    sty     ram_test_address   ; store the current address offset
    lda     (read_address_low_byte),y ; read byte from RAM
    cmp     ram_test_compare_value   ; compare with test byte
    bne     rammismatch
    iny
    bne     ramread

ramincrement:
; increment test value
    lda     ram_test_compare_value   ; load test byte
    eor     #$FF    ; flip all bits
    sta     ram_test_compare_value   ; save new test byte
    beq     rampagesuccess  ; done if we have tested all bits
    jmp     ramwrite

rampagesuccess:
    ; current 256 byte page success
    ; increment address

    ldx     read_address_high_byte
    inx
    cpx     #ram_space_end
    beq     ramtestdone
    stx     read_address_high_byte   ; write incremented address
    stx     ram_test_address_page   ; write copy of address
    jmp     ramwrite    ; test the next ram page

rammismatch:
    ; record the value read
    sta     ram_test_mismatch_value
    lda     #ram_test_mismatch_marker
    sta     ram_test_mismatch_indicator_address
    jmp     romtest
    
ramtestdone:
    lda     #ram_test_complete_marker
    sta     ram_test_complete_indicator_address

romtest:
    lda     #$00    ; start of rom space
    sta     read_address_low_byte
    lda     #rom_space_start
    sta     read_address_high_byte
    ldy     #$00

loop:
    lda     (read_address_low_byte),y  ; read byte from ROM
    sta     (read_address_low_byte),y  ; write byte back to ROM
    iny
    bne     loop

increment_rom_page:
    ldx     read_address_high_byte
    inx
    stx     read_address_high_byte
    cpx     #rom_space_end
    beq     done
    jmp     loop

done:
    lda     #done_marker
    sta     done_indicator_address
doneloop:
    nop
    jmp     doneloop    ; wait here

