; set up addresses to hold test results

test_address_start = $E800
zero_page_compare_value                 =   test_address_start
alternating_counter                     =   test_address_start + 1
pass_count                              =   test_address_start + 2


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
ram_space_end                           =   $D0

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
    ldy     #$01
    sty     pass_count
    sty     alternating_counter

; first pass
;zeropagewrite:
;    sta     $00,X   ; fill zero page with accumulator value
;    inx             ; increment counter
;    bne     zeropagewrite   ; keep writing until zero page done

;    ldy     #$03    ; set up counter for writing alternate values
    ; invert value
;    eor     zero_page_compare_value
;    sta     zero_page_compare_value

zeropagewrite:
    dey
    bne     zpcontinue  ; if not the right position, skip
    sta     $00,X   ; write the flag
    ldy     alternating_counter
zpcontinue:
    inx
    bne     zeropagewrite

done:
    lda     #done_marker
    sta     done_indicator_address
doneloop:
    nop
    jmp     doneloop    ; wait here

