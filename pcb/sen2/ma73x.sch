EESchema Schematic File Version 4
LIBS:ma73x-cache
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Angle Sensor Board with MA732"
Date "2020-08-08"
Rev "v1.0"
Comp "Crescent"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L ma732:MA732 U1
U 1 1 5F2EB893
P 3800 3200
F 0 "U1" H 4575 3575 50  0000 L CNN
F 1 "MA732" H 4475 3500 50  0000 L CNN
F 2 "Package_DFN_QFN:QFN-16-1EP_3x3mm_P0.5mm_EP1.7x1.7mm" H 3800 3200 50  0001 C CNN
F 3 "" H 3800 3200 50  0001 C CNN
	1    3800 3200
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x06 J1
U 1 1 5F2EC098
P 5900 3075
F 0 "J1" H 5980 3067 50  0000 L CNN
F 1 "Conn_01x06" H 5980 2976 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x06_P2.54mm_Vertical" H 5900 3075 50  0001 C CNN
F 3 "~" H 5900 3075 50  0001 C CNN
	1    5900 3075
	1    0    0    -1  
$EndComp
Text Label 5700 2875 2    50   ~ 0
3.3V
Text Label 5700 2975 2    50   ~ 0
GND
Text Label 5700 3075 2    50   ~ 0
CS
Text Label 5700 3175 2    50   ~ 0
CLK
Text Label 5700 3275 2    50   ~ 0
MISO
Text Label 5700 3375 2    50   ~ 0
MOSI
$Comp
L Device:C C1
U 1 1 5F2EC6AE
P 4750 2575
F 0 "C1" H 4865 2621 50  0000 L CNN
F 1 "C" H 4865 2530 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 4788 2425 50  0001 C CNN
F 3 "~" H 4750 2575 50  0001 C CNN
	1    4750 2575
	1    0    0    -1  
$EndComp
Text Label 4750 2725 2    50   ~ 0
GND
Text Label 4750 2425 2    50   ~ 0
3.3V
Text Label 3700 3500 2    50   ~ 0
MOSI
Text Label 4250 3800 3    50   ~ 0
MISO
Text Label 4050 3800 3    50   ~ 0
CS
Text Label 4700 3200 0    50   ~ 0
CLK
Text Label 4350 3800 3    50   ~ 0
GND
Text Label 3900 2850 2    50   ~ 0
GND
Text Label 4700 3400 0    50   ~ 0
GND
Text Label 4350 2850 1    50   ~ 0
3.3V
$Comp
L Mechanical:MountingHole H2
U 1 1 5F2F70B9
P 5600 2075
F 0 "H2" H 5700 2121 50  0000 L CNN
F 1 "MountingHole" H 5700 2030 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3" H 5600 2075 50  0001 C CNN
F 3 "~" H 5600 2075 50  0001 C CNN
	1    5600 2075
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H1
U 1 1 5F2F7D69
P 4875 2075
F 0 "H1" H 4975 2121 50  0000 L CNN
F 1 "MountingHole" H 4975 2030 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3" H 4875 2075 50  0001 C CNN
F 3 "~" H 4875 2075 50  0001 C CNN
	1    4875 2075
	1    0    0    -1  
$EndComp
NoConn ~ 4050 2850
NoConn ~ 4150 2850
NoConn ~ 4700 3300
NoConn ~ 4700 3500
NoConn ~ 4150 3800
NoConn ~ 3700 3400
NoConn ~ 3700 3300
NoConn ~ 3700 3200
$EndSCHEMATC
