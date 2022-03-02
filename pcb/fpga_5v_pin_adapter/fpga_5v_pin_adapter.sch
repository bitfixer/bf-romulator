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
L Connector_Generic:Conn_02x20_Counter_Clockwise J1
U 1 1 621EB390
P 2100 2150
F 0 "J1" H 2150 3267 50  0000 C CNN
F 1 "Conn_02x20_Counter_Clockwise" H 2150 3176 50  0000 C CNN
F 2 "Package_DIP:DIP-40_W15.24mm" H 2100 2150 50  0001 C CNN
F 3 "~" H 2100 2150 50  0001 C CNN
	1    2100 2150
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR0101
U 1 1 621ECE42
P 1650 3150
F 0 "#PWR0101" H 1650 3000 50  0001 C CNN
F 1 "+5V" H 1665 3323 50  0000 C CNN
F 2 "" H 1650 3150 50  0001 C CNN
F 3 "" H 1650 3150 50  0001 C CNN
	1    1650 3150
	1    0    0    -1  
$EndComp
Wire Wire Line
	1650 3150 1900 3150
$Comp
L Connector:Conn_01x01_Male J2
U 1 1 621ED297
P 3550 1250
F 0 "J2" H 3658 1431 50  0000 C CNN
F 1 "Conn_01x01_Male" H 3658 1340 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x01_P2.54mm_Vertical" H 3550 1250 50  0001 C CNN
F 3 "~" H 3550 1250 50  0001 C CNN
	1    3550 1250
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR0102
U 1 1 621ED604
P 4150 1250
F 0 "#PWR0102" H 4150 1100 50  0001 C CNN
F 1 "+5V" H 4165 1423 50  0000 C CNN
F 2 "" H 4150 1250 50  0001 C CNN
F 3 "" H 4150 1250 50  0001 C CNN
	1    4150 1250
	1    0    0    -1  
$EndComp
Wire Wire Line
	3750 1250 4150 1250
$EndSCHEMATC
