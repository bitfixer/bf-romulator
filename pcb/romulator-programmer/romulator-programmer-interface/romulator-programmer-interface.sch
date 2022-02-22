EESchema Schematic File Version 4
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
L MCU_Module:WeMos_D1_mini U1
U 1 1 62141207
P 2550 1950
F 0 "U1" H 2550 1061 50  0000 C CNN
F 1 "WeMos_D1_mini" H 2550 970 50  0000 C CNN
F 2 "Module:WEMOS_D1_mini_light" H 2550 800 50  0001 C CNN
F 3 "https://wiki.wemos.cc/products:d1:d1_mini#documentation" H 700 800 50  0001 C CNN
	1    2550 1950
	1    0    0    -1  
$EndComp
Text GLabel 4100 1500 0    50   Input ~ 0
MOSI
Text GLabel 4100 1600 0    50   Input ~ 0
CS
Text GLabel 4100 1700 0    50   Input ~ 0
RST
Text GLabel 4100 1800 0    50   Input ~ 0
SCK
Text GLabel 4100 1900 0    50   Input ~ 0
MISO
$Comp
L power:GND #PWR0101
U 1 1 62142AEB
P 4800 1900
F 0 "#PWR0101" H 4800 1650 50  0001 C CNN
F 1 "GND" H 4805 1727 50  0000 C CNN
F 2 "" H 4800 1900 50  0001 C CNN
F 3 "" H 4800 1900 50  0001 C CNN
	1    4800 1900
	1    0    0    -1  
$EndComp
Wire Wire Line
	4600 1900 4800 1900
Text GLabel 4600 1600 2    50   Input ~ 0
DBG
$Comp
L power:+3.3V #PWR0102
U 1 1 62142E51
P 5000 1500
F 0 "#PWR0102" H 5000 1350 50  0001 C CNN
F 1 "+3.3V" H 5015 1673 50  0000 C CNN
F 2 "" H 5000 1500 50  0001 C CNN
F 3 "" H 5000 1500 50  0001 C CNN
	1    5000 1500
	1    0    0    -1  
$EndComp
Wire Wire Line
	4600 1500 5000 1500
$Comp
L Connector:Conn_01x02_Male J1
U 1 1 62143487
P 3800 2500
F 0 "J1" H 3908 2681 50  0000 C CNN
F 1 "Conn_01x02_Male" H 3908 2590 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical" H 3800 2500 50  0001 C CNN
F 3 "~" H 3800 2500 50  0001 C CNN
	1    3800 2500
	1    0    0    -1  
$EndComp
Text GLabel 2650 900  2    50   Input ~ 0
3v3IN
Wire Wire Line
	2650 1150 2650 900 
Text GLabel 4000 2600 2    50   Input ~ 0
3v3IN
$Comp
L power:+3.3V #PWR0103
U 1 1 6214445C
P 4350 2500
F 0 "#PWR0103" H 4350 2350 50  0001 C CNN
F 1 "+3.3V" H 4365 2673 50  0000 C CNN
F 2 "" H 4350 2500 50  0001 C CNN
F 3 "" H 4350 2500 50  0001 C CNN
	1    4350 2500
	1    0    0    -1  
$EndComp
Wire Wire Line
	4000 2500 4350 2500
$Comp
L power:+5V #PWR0104
U 1 1 6214471A
P 2450 900
F 0 "#PWR0104" H 2450 750 50  0001 C CNN
F 1 "+5V" H 2465 1073 50  0000 C CNN
F 2 "" H 2450 900 50  0001 C CNN
F 3 "" H 2450 900 50  0001 C CNN
	1    2450 900 
	1    0    0    -1  
$EndComp
Wire Wire Line
	2450 1150 2450 900 
Text GLabel 2950 2150 2    50   Input ~ 0
MISO
Text GLabel 2950 2250 2    50   Input ~ 0
MOSI
Text GLabel 2950 2350 2    50   Input ~ 0
CS
Text GLabel 2950 2050 2    50   Input ~ 0
SCK
Text GLabel 2950 1550 2    50   Input ~ 0
DBG
$Comp
L Connector:Conn_01x01_Male J2
U 1 1 62145122
P 3850 900
F 0 "J2" H 3958 1081 50  0000 C CNN
F 1 "Conn_01x01_Male" H 3958 990 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x01_P2.54mm_Vertical" H 3850 900 50  0001 C CNN
F 3 "~" H 3850 900 50  0001 C CNN
	1    3850 900 
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR0105
U 1 1 62145398
P 4350 900
F 0 "#PWR0105" H 4350 750 50  0001 C CNN
F 1 "+5V" H 4365 1073 50  0000 C CNN
F 2 "" H 4350 900 50  0001 C CNN
F 3 "" H 4350 900 50  0001 C CNN
	1    4350 900 
	1    0    0    -1  
$EndComp
Wire Wire Line
	4050 900  4350 900 
$Comp
L Connector_Generic:Conn_02x05_Odd_Even J3
U 1 1 621482E1
P 4300 1700
F 0 "J3" H 4350 2117 50  0000 C CNN
F 1 "Conn_02x05_Odd_Even" H 4350 2026 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_2x05_P2.54mm_Vertical" H 4300 1700 50  0001 C CNN
F 3 "~" H 4300 1700 50  0001 C CNN
	1    4300 1700
	1    0    0    -1  
$EndComp
Text GLabel 2950 1650 2    50   Input ~ 0
RST
$EndSCHEMATC
