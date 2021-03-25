; set up addresses to hold test results

test_address_start = $E800
zero_page_compare_value                 =   test_address_start
alternating_counter                     =   test_address_start + 1
pass_count                              =   test_address_start + 2
flag_position_count                     =   test_address_start + 3
expected_value                          =   test_address_start + 4
read_value                              =   test_address_start + 5

page_counter                            =   test_address_start + 6
byte_counter                            =   test_address_start + 7
fault_indicator_address                 =   test_address_start + 8
done_indicator_address                  =   test_address_start + 9
test_address_end                        =   test_address_start + 9

test_address_count                      =   test_address_end - test_address_start + 1

read_address_low_byte                   =   $FB
read_address_high_byte                  =   $FC

ram_space_start                         =   $01
ram_space_end                           =   $80

rom_space_start                         =   $90
rom_space_end                           =   $FF

ram_test_mismatch_marker                =   $BB
ram_test_complete_marker                =   $CC
done_marker                             =   $DD

.segment    "CODE"

; check from 0x0000 to 0x8000
; standard RAM

start:
    
    ldx     #$00    ; load zero page address
    lda     #$FF    ; load flag value
    sta     zero_page_compare_value

init_flag_position:
    ; initialize flag position
    ; iterate through 3 offsets
    ldy     #$03
    sty     flag_position_count

begintestiteration:
    ldy     #$01
    sty     pass_count
    sty     alternating_counter

zeropagewrite:
    dey
    bne     zpcontinue  ; if not the right position, skip
    sta     $00,X   ; write the flag
    ldy     alternating_counter

zpcontinue:
    inx
    bne     zeropagewrite

    ldy     pass_count
    beq     zeropagecomparestart
    dey
    sty     pass_count

    ; set up the flag position
    ldy     #$03
    sty     alternating_counter
    ldy     flag_position_count
    eor     #$FF
    jmp     zeropagewrite

zeropagecomparestart:
    lda     zero_page_compare_value     ; load the current value for comparison
    ldy     flag_position_count

zeropagecompare:
    ; compare each value
    dey
    beq     checkflip
    cmp     $00,X
    bne     fault
    jmp     nextzeropagecompare

checkflip:
    eor     #$FF
    cmp     $00,X
    bne     fault
    eor     #$FF
    ldy     alternating_counter

nextzeropagecompare:
    inx
    bne     zeropagecompare

shiftflagposition:
    ldy     flag_position_count ; load current flag position
    dey
    beq     invert_flag
    sty     flag_position_count
    jmp     begintestiteration

invert_flag:
    lda     zero_page_compare_value
    beq     done ; if flag value is 0, we are done
    eor     #$FF
    sta     zero_page_compare_value
    jmp     init_flag_position

done:
    lda     #done_marker
    sta     done_indicator_address

doneloop:
    nop
    jmp     doneloop    ; wait here

fault:
    sta     expected_value
    lda     $00,X
    sta     read_value
    lda     #ram_test_mismatch_marker
    sta     fault_indicator_address
    stx     byte_counter
    jmp     done
