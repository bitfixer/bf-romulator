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
Text GLabel 4000 1150 0    50   Input ~ 0
MOSI
Text GLabel 4000 1250 0    50   Input ~ 0
CS
Text GLabel 4000 1350 0    50   Input ~ 0
RST
Text GLabel 4000 1450 0    50   Input ~ 0
SCK
Text GLabel 4000 1550 0    50   Input ~ 0
MISO
$Comp
L power:GND #PWR04
U 1 1 62142AEB
P 4700 1550
F 0 "#PWR04" H 4700 1300 50  0001 C CNN
F 1 "GND" H 4705 1377 50  0000 C CNN
F 2 "" H 4700 1550 50  0001 C CNN
F 3 "" H 4700 1550 50  0001 C CNN
	1    4700 1550
	1    0    0    -1  
$EndComp
Wire Wire Line
	4500 1550 4700 1550
Text GLabel 4500 1250 2    50   Input ~ 0
DBG
$Comp
L power:+3.3V #PWR05
U 1 1 62142E51
P 4900 1150
F 0 "#PWR05" H 4900 1000 50  0001 C CNN
F 1 "+3.3V" H 4915 1323 50  0000 C CNN
F 2 "" H 4900 1150 50  0001 C CNN
F 3 "" H 4900 1150 50  0001 C CNN
	1    4900 1150
	1    0    0    -1  
$EndComp
Wire Wire Line
	4500 1150 4900 1150
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
L power:+3.3V #PWR02
U 1 1 6214445C
P 4350 2500
F 0 "#PWR02" H 4350 2350 50  0001 C CNN
F 1 "+3.3V" H 4365 2673 50  0000 C CNN
F 2 "" H 4350 2500 50  0001 C CNN
F 3 "" H 4350 2500 50  0001 C CNN
	1    4350 2500
	1    0    0    -1  
$EndComp
Wire Wire Line
	4000 2500 4350 2500
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
Text GLabel 2950 1650 2    50   Input ~ 0
RST
Text GLabel 4500 1350 2    50   Input ~ 0
CDONE
Text GLabel 2950 1750 2    50   Input ~ 0
CDONE
$Comp
L Connector_Generic:Conn_02x08_Odd_Even J3
U 1 1 621EB698
P 4200 1450
F 0 "J3" H 4250 1967 50  0000 C CNN
F 1 "Conn_02x08_Odd_Even" H 4250 1876 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_2x08_P2.54mm_Vertical" H 4200 1450 50  0001 C CNN
F 3 "~" H 4200 1450 50  0001 C CNN
	1    4200 1450
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR01
U 1 1 621EDA97
P 3550 1850
F 0 "#PWR01" H 3550 1700 50  0001 C CNN
F 1 "+5V" H 3565 2023 50  0000 C CNN
F 2 "" H 3550 1850 50  0001 C CNN
F 3 "" H 3550 1850 50  0001 C CNN
	1    3550 1850
	1    0    0    -1  
$EndComp
Wire Wire Line
	3550 1850 4000 1850
$Comp
L Connector:Conn_01x02_Male J2
U 1 1 621F045A
P 3800 3050
F 0 "J2" H 3908 3231 50  0000 C CNN
F 1 "Conn_01x02_Male" H 3908 3140 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical" H 3800 3050 50  0001 C CNN
F 3 "~" H 3800 3050 50  0001 C CNN
	1    3800 3050
	1    0    0    -1  
$EndComp
Text GLabel 2450 900  0    50   Input ~ 0
5VIN
$Comp
L power:+5V #PWR03
U 1 1 621F0DC6
P 4350 3050
F 0 "#PWR03" H 4350 2900 50  0001 C CNN
F 1 "+5V" H 4365 3223 50  0000 C CNN
F 2 "" H 4350 3050 50  0001 C CNN
F 3 "" H 4350 3050 50  0001 C CNN
	1    4350 3050
	1    0    0    -1  
$EndComp
Text GLabel 4000 3150 2    50   Input ~ 0
5VIN
Wire Wire Line
	4000 3050 4350 3050
$EndSCHEMATC
