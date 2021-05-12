EESchema Schematic File Version 4
LIBS:programming-cache
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Connector_Generic:Conn_02x05_Odd_Even J2
U 1 1 60943191
P 6400 2550
F 0 "J2" H 6450 2967 50  0000 C CNN
F 1 "Conn_02x05_Odd_Even" H 6450 2876 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_2x05_P2.54mm_Vertical" H 6400 2550 50  0001 C CNN
F 3 "~" H 6400 2550 50  0001 C CNN
	1    6400 2550
	1    0    0    -1  
$EndComp
$Comp
L Connector:Raspberry_Pi_2_3 J1
U 1 1 609490FC
P 3350 2700
F 0 "J1" H 3350 4181 50  0000 C CNN
F 1 "Raspberry_Pi_2_3" H 3350 4090 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_2x20_P2.54mm_Vertical" H 3350 2700 50  0001 C CNN
F 3 "https://www.raspberrypi.org/documentation/hardware/raspberrypi/schematics/rpi_SCH_3bplus_1p0_reduced.pdf" H 3350 2700 50  0001 C CNN
	1    3350 2700
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR0101
U 1 1 6094D9D5
P 3450 1000
F 0 "#PWR0101" H 3450 850 50  0001 C CNN
F 1 "+3V3" H 3465 1173 50  0000 C CNN
F 2 "" H 3450 1000 50  0001 C CNN
F 3 "" H 3450 1000 50  0001 C CNN
	1    3450 1000
	1    0    0    -1  
$EndComp
Wire Wire Line
	3450 1000 3450 1400
$Comp
L power:GND #PWR0102
U 1 1 6094E7BD
P 3650 4150
F 0 "#PWR0102" H 3650 3900 50  0001 C CNN
F 1 "GND" H 3655 3977 50  0000 C CNN
F 2 "" H 3650 4150 50  0001 C CNN
F 3 "" H 3650 4150 50  0001 C CNN
	1    3650 4150
	1    0    0    -1  
$EndComp
Wire Wire Line
	3650 4000 3650 4150
$Comp
L power:GND #PWR0103
U 1 1 6094EBD2
P 6950 2750
F 0 "#PWR0103" H 6950 2500 50  0001 C CNN
F 1 "GND" H 6955 2577 50  0000 C CNN
F 2 "" H 6950 2750 50  0001 C CNN
F 3 "" H 6950 2750 50  0001 C CNN
	1    6950 2750
	1    0    0    -1  
$EndComp
Wire Wire Line
	6700 2750 6950 2750
Text GLabel 6200 2350 0    50   Input ~ 0
MOSI
Text GLabel 6200 2450 0    50   Input ~ 0
CS
Text GLabel 6200 2550 0    50   Input ~ 0
RST
Text GLabel 6200 2650 0    50   Input ~ 0
SCK
Text GLabel 6200 2750 0    50   Input ~ 0
MISO
$Comp
L power:+3V3 #PWR0104
U 1 1 6094F5BB
P 6950 2350
F 0 "#PWR0104" H 6950 2200 50  0001 C CNN
F 1 "+3V3" H 6965 2523 50  0000 C CNN
F 2 "" H 6950 2350 50  0001 C CNN
F 3 "" H 6950 2350 50  0001 C CNN
	1    6950 2350
	1    0    0    -1  
$EndComp
Wire Wire Line
	6700 2350 6950 2350
Text GLabel 6700 2450 2    50   Input ~ 0
CSDBG
Text GLabel 4150 2900 2    50   Input ~ 0
CS
Text GLabel 4150 3000 2    50   Input ~ 0
MISO
Text GLabel 4150 3100 2    50   Input ~ 0
MOSI
Text GLabel 4150 3200 2    50   Input ~ 0
SCK
Text GLabel 2550 3200 0    50   Input ~ 0
RST
Text GLabel 2550 2100 0    50   Input ~ 0
CSDBG
$EndSCHEMATC
