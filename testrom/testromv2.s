; ROMULATOR_PET_RAMTEST v2
; Tests full memory map using a memory test adapted from
; Jim Butterfield's 1977 article:
; http://archive.6502.org/publications/dr_dobbs_journal_selected_articles/high_speed_memory_test_for_6502.pdf
; The basic method is:
; 1. Value FF is stored in every location to be tested.
; 2. Value 00 is stored in every third location, giving a pattern of FF FF 00 FF FF 00 ...
; 3. Memory is checked for all values.
; repeat 3 times, shifting the position of the 00 each time.
; then repeat entire sequence, flipping FF and 00 values.
; Do this process once per memory page (256 bytes)

; this version uses ROMulator's memory mapped over the PET IO space
; during the test to hold results.

; set up addresses to hold test results

;test_address_start = $E800
;test_address_start = $8100

screen_memory = $8000
page_hex_high_position                  =   screen_memory
page_hex_low_position                   =   screen_memory + 1

test_address_start = $9000

page_counter_offset     = 4
byte_counter_offset     = 5
expected_value_offset   = 6
read_value_offset       = 7
fault_offset_done       = 8

zero_page_compare_value                 =   test_address_start
alternating_counter                     =   test_address_start + 1
pass_count                              =   test_address_start + 2
flag_position_count                     =   test_address_start + 3
page_counter                            =   test_address_start + page_counter_offset
byte_counter                            =   test_address_start + byte_counter_offset
expected_value                          =   test_address_start + expected_value_offset
read_value                              =   test_address_start + read_value_offset

fault_indicator_address                 =   test_address_start + 8
done_indicator_address                  =   test_address_start + 9
temp_value                              =   test_address_start + 10
temp_value_2                            =   test_address_start + 11
text_table_start                        =   test_address_start + 12

read_address_low_byte                   =   $FB
read_address_high_byte                  =   $FC

ram_space_start                         =   $01
ram_space_end                           =   $7F

rom_space_start                         =   $90
rom_space_end                           =   $FF

ram_test_mismatch_marker                =   $BB
ram_test_complete_marker                =   $CC
done_marker                             =   $DD

.segment    "CODE"

; check from 0x0000 to 0x8000
; standard RAM

start:
    ldy     #$00    ; load index
    sty     page_counter

clear_video_ram_page:
    lda     #$20
video_loop:
    sta     $8000,Y
    iny
    bne     video_loop

startpage:
    lda     #$FF    ; load flag value
    sta     zero_page_compare_value

init_flag_position:
    ; initialize flag position
    ; iterate through 3 offsets
    ldx     #$03
    stx     flag_position_count

begintestiteration:
    ldx     #$01
    stx     pass_count
    stx     alternating_counter

; write one page of memory
zeropagewrite:
    dex
    bne     zpcontinue  ; if not the right position, skip

    ; check what page we are on
    ; to determine addressing method
    ldx     page_counter
    bne     pagewriteflag

zpwriteflag:
    sta     $0000,Y
    jmp     writeflagend

pagewriteflag:
    sta     (read_address_low_byte),Y

writeflagend:
    ldx     alternating_counter

zpcontinue:
    iny
    bne     zeropagewrite

    ldx     pass_count
    beq     zeropagecomparestart
    dex
    stx     pass_count

    ; set up the flag position
    ldx     #$03
    stx     alternating_counter
    ldx     flag_position_count
    eor     #$FF
    jmp     zeropagewrite

zeropagecomparestart:
    lda     zero_page_compare_value     ; load the current value for comparison
    ldx     flag_position_count

zeropagecompare:
    ; compare each value
    dex                                 ; decrement alternating
    bne     no_flip
    eor     #$FF                        ;flip the flag
    
no_flip:
    sta     temp_value
    lda     page_counter
    beq     zpcompare

compare:
    lda     temp_value
    cmp     (read_address_low_byte),Y
    bne     fault
    jmp     donecompare

zpcompare:
    lda     temp_value
    cmp     $0000,Y
    bne     fault

donecompare:
    cpx     #$00
    bne     no_flip_2
    eor     #$FF
    ldx     alternating_counter

no_flip_2:
    jmp     nextzeropagecompare

nextzeropagecompare:
    iny
    bne     zeropagecompare

shiftflagposition:
    ldx     flag_position_count ; load current flag position
    dex
    beq     invert_flag
    stx     flag_position_count
    jmp     begintestiteration

invert_flag:
    lda     zero_page_compare_value
    beq     done_page ; if flag value is 0, we are done
    eor     #$FF
    sta     zero_page_compare_value
    jmp     init_flag_position

done_page:
    lda     page_counter
    cmp     #$02
    bcs     display_page
    ; first two pages have not been tested yet
    ; can't use jsr yet, since we don't know if the stack is functional.
    ; just display the digit
    ldx     #$30    ; '0' digit
    stx     page_hex_high_position
    adc     #$30    ; add the offset of '0' digit
    sta     page_hex_low_position
    
ready_next_page:
    ldx     page_counter
    cpx     #ram_space_end

    beq     done                ; done testing ram
    inx     ; increment page
    stx     page_counter
    stx     read_address_high_byte
    jmp     startpage

; printhex
; store value in A
; store offset in Y
display_page:
    ldy     #$00
    jsr     printhex
    ldy     #$00
    jmp     ready_next_page

done:
    lda     #done_marker
    sta     done_indicator_address
    sta     $8010

doneloop:
    nop
    jmp     doneloop    ; wait here

; memory test failed
; display the failed address 
fault:
    sta     expected_value
    sty     byte_counter
    lda     page_counter
    beq     zpfault
pagefault:
    lda     (read_address_low_byte),Y
    jmp     showfault
zpfault:
    lda     $0000,Y
showfault:
    sta     read_value
    ldx     #page_counter_offset
    ldy     #0
display_fault_information:
    lda     test_address_start,X
    
    stx     temp_value
    sty     temp_value_2
    jmp     printhex_nojsr
printhex_nojsr_return:
    ;jmp     display_fault_done
    ldx     temp_value
    inx
    cpx     #fault_offset_done
    bcs     display_fault_done
    ; prepare next value to display
    lda     temp_value_2
    adc     #4
    tay
    jmp     display_fault_information

display_fault_done:
    lda     #ram_test_mismatch_marker
    sta     fault_indicator_address
    jmp     done

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

printhex_nojsr:
    tax
    lsr
    lsr
    lsr
    lsr

    cmp     #$0A
    bcs     get_letter_nojsr
    adc     #$30
    jmp     high_hex
get_letter_nojsr:
    adc     #$36
high_hex:
    sta     screen_memory,Y
    iny
    txa
    and     #$0F
    
    cmp     #$0A
    bcs     get_letter_nojsr2
    adc     #$30
    jmp     low_hex
get_letter_nojsr2:
    adc     #$36

low_hex:
    sta     screen_memory,Y
    jmp     printhex_nojsr_return
