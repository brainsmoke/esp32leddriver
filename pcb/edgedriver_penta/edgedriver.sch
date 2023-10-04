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
L esp32-wrover:ESP32-WROVER U1
U 1 1 5EF3B8F9
P 4050 5850
F 0 "U1" H 4025 7187 60  0000 C CNN
F 1 "ESP32-WROVER" H 4025 7081 60  0000 C CNN
F 2 "esp32-wrover:XCVR_ESP32-WROVER" H 4500 5550 60  0001 C CNN
F 3 "" H 4500 5550 60  0001 C CNN
F 4 "C701352" H -1000 750 50  0001 C CNN "LCSC"
	1    4050 5850
	1    0    0    -1  
$EndComp
Text GLabel 9100 3150 2    50   Input ~ 0
STMRX
Wire Wire Line
	9100 3150 9000 3150
Text GLabel 9100 3050 2    50   Input ~ 0
STMTX
Wire Wire Line
	9100 3050 9000 3050
Wire Wire Line
	9100 4250 9000 4250
Wire Wire Line
	9100 4150 9000 4150
$Comp
L power:GND #PWR0103
U 1 1 5EF958D2
P 8400 4750
F 0 "#PWR0103" H 8400 4500 50  0001 C CNN
F 1 "GND" H 8405 4577 50  0000 C CNN
F 2 "" H 8400 4750 50  0001 C CNN
F 3 "" H 8400 4750 50  0001 C CNN
	1    8400 4750
	1    0    0    -1  
$EndComp
Wire Wire Line
	8400 4750 8400 4650
Text GLabel 8700 1050 2    50   Input ~ 0
3V3
$Comp
L Device:R R6
U 1 1 5EFE5F67
P 7500 1550
F 0 "R6" V 7400 1550 50  0000 C CNN
F 1 "10k" V 7500 1550 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric" V 7430 1550 50  0001 C CNN
F 3 "~" H 7500 1550 50  0001 C CNN
F 4 "C25744" H -150 -2300 50  0001 C CNN "LCSC"
	1    7500 1550
	0    1    1    0   
$EndComp
$Comp
L Device:R R7
U 1 1 5EFE8183
P 7500 1750
F 0 "R7" V 7600 1750 50  0000 C CNN
F 1 "10k" V 7500 1750 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric" V 7430 1750 50  0001 C CNN
F 3 "~" H 7500 1750 50  0001 C CNN
F 4 "C25744" H -150 -2300 50  0001 C CNN "LCSC"
	1    7500 1750
	0    1    1    0   
$EndComp
Wire Wire Line
	7650 1550 7800 1550
Wire Wire Line
	7800 1750 7650 1750
Text GLabel 7150 1550 0    50   Input ~ 0
3V3
$Comp
L power:GND #PWR0106
U 1 1 5EFEB61D
P 7150 2050
F 0 "#PWR0106" H 7150 1800 50  0001 C CNN
F 1 "GND" H 7155 1877 50  0000 C CNN
F 2 "" H 7150 2050 50  0001 C CNN
F 3 "" H 7150 2050 50  0001 C CNN
	1    7150 2050
	1    0    0    -1  
$EndComp
Wire Wire Line
	7150 2050 7150 1750
Wire Wire Line
	7350 1750 7150 1750
Wire Wire Line
	7350 1550 7150 1550
Text GLabel 6200 5050 2    50   Input ~ 0
3V3
$Comp
L Device:C C8
U 1 1 5EFEF351
P 6150 5250
F 0 "C8" H 6265 5296 50  0000 L CNN
F 1 "100nF" H 6265 5205 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 6188 5100 50  0001 C CNN
F 3 "~" H 6150 5250 50  0001 C CNN
F 4 "C1525" H -1300 300 50  0001 C CNN "LCSC"
	1    6150 5250
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0107
U 1 1 5EFEF35B
P 6150 5500
F 0 "#PWR0107" H 6150 5250 50  0001 C CNN
F 1 "GND" H 6155 5327 50  0000 C CNN
F 2 "" H 6150 5500 50  0001 C CNN
F 3 "" H 6150 5500 50  0001 C CNN
	1    6150 5500
	1    0    0    -1  
$EndComp
Wire Wire Line
	6200 5050 6150 5050
Wire Wire Line
	6150 5050 6150 5100
Wire Wire Line
	6150 5400 6150 5500
Text GLabel 2100 5350 2    50   Input ~ 0
3V3
$Comp
L Device:C C5
U 1 1 5EFF7578
P 2050 5550
F 0 "C5" H 2165 5596 50  0000 L CNN
F 1 "100nF" H 2165 5505 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 2088 5400 50  0001 C CNN
F 3 "~" H 2050 5550 50  0001 C CNN
F 4 "C1525" H -1000 750 50  0001 C CNN "LCSC"
	1    2050 5550
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0108
U 1 1 5EFF757E
P 2050 5800
F 0 "#PWR0108" H 2050 5550 50  0001 C CNN
F 1 "GND" H 2055 5627 50  0000 C CNN
F 2 "" H 2050 5800 50  0001 C CNN
F 3 "" H 2050 5800 50  0001 C CNN
	1    2050 5800
	1    0    0    -1  
$EndComp
Wire Wire Line
	2100 5350 2050 5350
Wire Wire Line
	2050 5350 2050 5400
Wire Wire Line
	2050 5700 2050 5750
Wire Wire Line
	3150 5100 2900 5100
Wire Wire Line
	2900 5100 2900 6500
Wire Wire Line
	3150 6500 2900 6500
Connection ~ 2900 6500
Wire Wire Line
	2900 6500 2900 7200
$Comp
L power:GND #PWR0109
U 1 1 5EFFF208
P 2900 7200
F 0 "#PWR0109" H 2900 6950 50  0001 C CNN
F 1 "GND" H 2905 7027 50  0000 C CNN
F 2 "" H 2900 7200 50  0001 C CNN
F 3 "" H 2900 7200 50  0001 C CNN
	1    2900 7200
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0110
U 1 1 5EFFF22B
P 5450 5250
F 0 "#PWR0110" H 5450 5000 50  0001 C CNN
F 1 "GND" H 5455 5077 50  0000 C CNN
F 2 "" H 5450 5250 50  0001 C CNN
F 3 "" H 5450 5250 50  0001 C CNN
	1    5450 5250
	1    0    0    -1  
$EndComp
Text GLabel 2700 4850 0    50   Input ~ 0
3V3
Wire Wire Line
	2700 4850 2800 4850
Wire Wire Line
	2800 4850 2800 5200
Wire Wire Line
	2800 5200 3150 5200
Text GLabel 5000 5600 2    50   Input ~ 0
ESPRX
Text GLabel 5000 5500 2    50   Input ~ 0
ESPTX
Wire Wire Line
	5450 5200 5450 5250
Wire Wire Line
	4900 5200 5450 5200
Wire Wire Line
	5450 5100 5450 5200
Wire Wire Line
	4900 5100 5450 5100
Connection ~ 5450 5200
Wire Wire Line
	5000 5500 4900 5500
Wire Wire Line
	5000 5600 4900 5600
Text GLabel 5300 6500 2    50   Input ~ 0
ESPRST
Text GLabel 2750 5300 0    50   Input ~ 0
ESPEN
Wire Wire Line
	3150 5300 2750 5300
$Comp
L Device:C C7
U 1 1 5EFEC07D
P 4350 1650
F 0 "C7" H 4465 1696 50  0000 L CNN
F 1 "1uF" H 4465 1605 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 4388 1500 50  0001 C CNN
F 3 "~" H 4350 1650 50  0001 C CNN
F 4 "C52923" H -2000 100 50  0001 C CNN "LCSC"
	1    4350 1650
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0112
U 1 1 5EFEE86F
P 4350 1950
F 0 "#PWR0112" H 4350 1700 50  0001 C CNN
F 1 "GND" H 4355 1777 50  0000 C CNN
F 2 "" H 4350 1950 50  0001 C CNN
F 3 "" H 4350 1950 50  0001 C CNN
	1    4350 1950
	1    0    0    -1  
$EndComp
Wire Wire Line
	4350 1800 4350 1950
Text GLabel 4550 1500 2    50   Input ~ 0
ESPEN
Wire Wire Line
	4900 6500 5300 6500
$Comp
L Device:R R5
U 1 1 5F05100B
P 4350 1350
F 0 "R5" V 4250 1350 50  0000 C CNN
F 1 "10k" V 4350 1350 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric" V 4280 1350 50  0001 C CNN
F 3 "~" H 4350 1350 50  0001 C CNN
F 4 "C25744" H -2000 250 50  0001 C CNN "LCSC"
	1    4350 1350
	1    0    0    -1  
$EndComp
Text GLabel 4400 1150 2    50   Input ~ 0
3V3
Wire Wire Line
	4400 1150 4350 1150
Wire Wire Line
	4350 1150 4350 1200
Wire Wire Line
	4550 1500 4350 1500
Text GLabel 2700 6600 0    50   Input ~ 0
STMRX
Text GLabel 2700 5700 0    50   Input ~ 0
STMTX
Entry Bus Bus
	-1250 -1000 -1150 -900
Text GLabel 9100 4150 2    50   Input ~ 0
SWDIO
Text GLabel 9100 4250 2    50   Input ~ 0
SWDCLK
Wire Wire Line
	2700 5700 3150 5700
Wire Wire Line
	2700 6600 3150 6600
$Comp
L Device:C C9
U 1 1 5F011A6B
P 1650 5550
F 0 "C9" H 1765 5596 50  0000 L CNN
F 1 "1uF" H 1765 5505 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 1688 5400 50  0001 C CNN
F 3 "~" H 1650 5550 50  0001 C CNN
F 4 "C52923" H 1650 5550 50  0001 C CNN "LCSC"
	1    1650 5550
	1    0    0    -1  
$EndComp
Wire Wire Line
	2050 5350 1650 5350
Wire Wire Line
	1650 5350 1650 5400
Connection ~ 2050 5350
Wire Wire Line
	1650 5700 1650 5750
Wire Wire Line
	1650 5750 2050 5750
Connection ~ 2050 5750
Wire Wire Line
	2050 5750 2050 5800
$Comp
L Device:R R11
U 1 1 5FF95363
P 5150 2150
F 0 "R11" V 5050 2150 50  0000 C CNN
F 1 "10k" V 5150 2150 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric" V 5080 2150 50  0001 C CNN
F 3 "~" H 5150 2150 50  0001 C CNN
F 4 "C25744" H -1200 1050 50  0001 C CNN "LCSC"
	1    5150 2150
	1    0    0    -1  
$EndComp
Text GLabel 5200 1950 2    50   Input ~ 0
3V3
Wire Wire Line
	5200 1950 5150 1950
Wire Wire Line
	5150 1950 5150 2000
Text GLabel 5200 2350 2    50   Input ~ 0
ESPRST
Wire Wire Line
	5150 2300 5150 2350
Wire Wire Line
	5200 2350 5150 2350
Wire Wire Line
	8600 1050 8600 1350
Connection ~ 8600 1050
Wire Wire Line
	8600 1050 8700 1050
Wire Wire Line
	8500 1050 8500 1350
Wire Wire Line
	8500 1050 8600 1050
$Comp
L Device:C C10
U 1 1 600892EE
P 5900 5250
F 0 "C10" H 6015 5296 50  0000 L CNN
F 1 "1uF" H 6015 5205 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 5938 5100 50  0001 C CNN
F 3 "~" H 5900 5250 50  0001 C CNN
F 4 "C52923" H -1550 300 50  0001 C CNN "LCSC"
	1    5900 5250
	1    0    0    -1  
$EndComp
Wire Wire Line
	6150 5050 5900 5050
Wire Wire Line
	5900 5050 5900 5100
Connection ~ 6150 5050
Wire Wire Line
	5900 5400 5900 5500
Wire Wire Line
	5900 5500 6150 5500
Connection ~ 6150 5500
$Comp
L Connector:Conn_01x03_Male J5
U 1 1 6142A2C3
P 7400 5700
F 0 "J5" H 7506 5978 50  0000 C CNN
F 1 "Conn_01x03_Male" H 7506 5887 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 7400 5700 50  0001 C CNN
F 3 "~" H 7400 5700 50  0001 C CNN
	1    7400 5700
	1    0    0    -1  
$EndComp
Text GLabel 7850 5600 2    50   Input ~ 0
SWDCLK
Text GLabel 7850 5700 2    50   Input ~ 0
SWDIO
$Comp
L power:GND #PWR0111
U 1 1 61437544
P 7850 5900
F 0 "#PWR0111" H 7850 5650 50  0001 C CNN
F 1 "GND" H 7855 5727 50  0000 C CNN
F 2 "" H 7850 5900 50  0001 C CNN
F 3 "" H 7850 5900 50  0001 C CNN
	1    7850 5900
	1    0    0    -1  
$EndComp
Wire Wire Line
	7850 5900 7850 5800
Wire Wire Line
	7850 5800 7600 5800
Wire Wire Line
	7850 5700 7600 5700
Wire Wire Line
	7850 5600 7600 5600
$Comp
L MCU_ST_STM32F0:STM32F030C8Tx U3
U 1 1 64E2C13C
P 8400 2950
F 0 "U3" H 8400 1261 50  0000 C CNN
F 1 "STM32F030C8Tx" H 8400 1170 50  0000 C CNN
F 2 "Package_QFP:LQFP-48_7x7mm_P0.5mm" H 7900 1450 50  0001 R CNN
F 3 "http://www.st.com/st-web-ui/static/active/en/resource/technical/document/datasheet/DM00088500.pdf" H 8400 2950 50  0001 C CNN
F 4 "C23922" H 8400 2950 50  0001 C CNN "LCSC"
	1    8400 2950
	1    0    0    -1  
$EndComp
Wire Wire Line
	8500 1050 8400 1050
Wire Wire Line
	8300 1050 8300 1350
Connection ~ 8500 1050
Wire Wire Line
	8400 1350 8400 1050
Connection ~ 8400 1050
Wire Wire Line
	8400 1050 8300 1050
Connection ~ 4350 1500
Wire Wire Line
	8500 4550 8500 4650
Wire Wire Line
	8500 4650 8400 4650
Connection ~ 8400 4650
Wire Wire Line
	8400 4650 8400 4550
Wire Wire Line
	8400 4650 8300 4650
Wire Wire Line
	8300 4650 8300 4550
Text GLabel 2400 950  2    50   Input ~ 0
3V3
Wire Wire Line
	2400 950  2250 950 
$Comp
L Device:C C2
U 1 1 64F4E33A
P 1850 1250
F 0 "C2" H 1965 1296 50  0000 L CNN
F 1 "1uF" H 1965 1205 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 1888 1100 50  0001 C CNN
F 3 "~" H 1850 1250 50  0001 C CNN
F 4 "C52923" H -4500 -300 50  0001 C CNN "LCSC"
	1    1850 1250
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0104
U 1 1 64F4E5BA
P 1850 1550
F 0 "#PWR0104" H 1850 1300 50  0001 C CNN
F 1 "GND" H 1855 1377 50  0000 C CNN
F 2 "" H 1850 1550 50  0001 C CNN
F 3 "" H 1850 1550 50  0001 C CNN
	1    1850 1550
	1    0    0    -1  
$EndComp
Wire Wire Line
	1850 1400 1850 1550
$Comp
L Device:C C3
U 1 1 64F5F0FA
P 2250 1250
F 0 "C3" H 2365 1296 50  0000 L CNN
F 1 "1uF" H 2365 1205 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 2288 1100 50  0001 C CNN
F 3 "~" H 2250 1250 50  0001 C CNN
F 4 "C52923" H -4100 -300 50  0001 C CNN "LCSC"
	1    2250 1250
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0105
U 1 1 64F5F390
P 2250 1550
F 0 "#PWR0105" H 2250 1300 50  0001 C CNN
F 1 "GND" H 2255 1377 50  0000 C CNN
F 2 "" H 2250 1550 50  0001 C CNN
F 3 "" H 2250 1550 50  0001 C CNN
	1    2250 1550
	1    0    0    -1  
$EndComp
Wire Wire Line
	2250 1400 2250 1550
Wire Wire Line
	2250 1100 2250 950 
$Comp
L Connector:Conn_01x05_Male J2
U 1 1 64FD1AF7
P 3200 3150
F 0 "J2" H 3308 3531 50  0000 C CNN
F 1 "Conn_01x05_Male" H 3308 3440 50  0000 C CNN
F 2 "Connector_Molex:Molex_PicoBlade_53261-0571_1x05-1MP_P1.25mm_Horizontal" H 3200 3150 50  0001 C CNN
F 3 "~" H 3200 3150 50  0001 C CNN
F 4 "C225114" H 3200 3150 50  0001 C CNN "LCSC"
	1    3200 3150
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0113
U 1 1 64FD4390
P 3700 3650
F 0 "#PWR0113" H 3700 3400 50  0001 C CNN
F 1 "GND" H 3705 3477 50  0000 C CNN
F 2 "" H 3700 3650 50  0001 C CNN
F 3 "" H 3700 3650 50  0001 C CNN
	1    3700 3650
	1    0    0    -1  
$EndComp
Text GLabel 3750 3250 2    50   Input ~ 0
ESPRST
Text GLabel 3750 3150 2    50   Input ~ 0
ESPEN
Text GLabel 3750 3050 2    50   Input ~ 0
ESPRX
Text GLabel 3750 2950 2    50   Input ~ 0
ESPTX
Wire Wire Line
	3400 2950 3750 2950
Wire Wire Line
	3400 3050 3750 3050
Wire Wire Line
	3400 3150 3750 3150
Wire Wire Line
	3400 3250 3750 3250
Wire Wire Line
	3400 3350 3700 3350
Wire Wire Line
	3700 3350 3700 3650
$Comp
L power:GND #PWR0114
U 1 1 64F2D09E
P 7400 2400
F 0 "#PWR0114" H 7400 2150 50  0001 C CNN
F 1 "GND" H 7405 2227 50  0000 C CNN
F 2 "" H 7400 2400 50  0001 C CNN
F 3 "" H 7400 2400 50  0001 C CNN
	1    7400 2400
	1    0    0    -1  
$EndComp
Wire Wire Line
	7800 2150 7400 2150
Wire Wire Line
	7400 2150 7400 2400
Wire Wire Line
	7800 2250 7650 2250
Text GLabel 7650 2250 0    50   Input ~ 0
3V3
Wire Notes Line
	7700 2100 7300 2100
Wire Notes Line
	7300 2100 7300 2700
Wire Notes Line
	7300 2700 7700 2700
Wire Notes Line
	7700 2700 7700 2100
Text Notes 6550 2700 0    50   ~ 0
compatibility with\nstm32f072
Text GLabel 7600 2850 0    50   Input ~ 0
DATA0
Text GLabel 7600 2950 0    50   Input ~ 0
DATA1
Text GLabel 7600 3050 0    50   Input ~ 0
DATA2
Text GLabel 7600 3150 0    50   Input ~ 0
DATA3
Text GLabel 7600 3250 0    50   Input ~ 0
DATA4
Text GLabel 7600 3350 0    50   Input ~ 0
DATA5
Text GLabel 7600 3450 0    50   Input ~ 0
DATA6
Text GLabel 7600 3550 0    50   Input ~ 0
DATA7
Text GLabel 7600 3650 0    50   Input ~ 0
DATA8
Text GLabel 7600 3750 0    50   Input ~ 0
DATA9
Text GLabel 7600 3850 0    50   Input ~ 0
DATA10
Text GLabel 7600 3950 0    50   Input ~ 0
DATA11
Text GLabel 7600 4050 0    50   Input ~ 0
DATA12
Text GLabel 7600 4150 0    50   Input ~ 0
DATA13
Text GLabel 7600 4250 0    50   Input ~ 0
DATA14
Text GLabel 7600 4350 0    50   Input ~ 0
DATA15
Wire Wire Line
	7600 2850 7800 2850
Wire Wire Line
	7600 2950 7800 2950
Wire Wire Line
	7600 3050 7800 3050
Wire Wire Line
	7600 3150 7800 3150
Wire Wire Line
	7600 3250 7800 3250
Wire Wire Line
	7600 3350 7800 3350
Wire Wire Line
	7600 3450 7800 3450
Wire Wire Line
	7600 3550 7800 3550
Wire Wire Line
	7600 3650 7800 3650
Wire Wire Line
	7600 3750 7800 3750
Wire Wire Line
	7600 3850 7800 3850
Wire Wire Line
	7600 3950 7800 3950
Wire Wire Line
	7600 4050 7800 4050
Wire Wire Line
	7600 4150 7800 4150
Wire Wire Line
	7600 4250 7800 4250
Wire Wire Line
	7600 4350 7800 4350
$Comp
L Connector_Generic:Conn_02x10_Odd_Even J1
U 1 1 65233A12
P 10550 3150
F 0 "J1" H 10600 3767 50  0000 C CNN
F 1 "Conn_02x10_Odd_Even" H 10600 3676 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_2x10_P2.54mm_Vertical" H 10550 3150 50  0001 C CNN
F 3 "~" H 10550 3150 50  0001 C CNN
	1    10550 3150
	1    0    0    -1  
$EndComp
Wire Wire Line
	10150 2750 10350 2750
Wire Wire Line
	10150 2850 10350 2850
$Comp
L power:GND #PWR0101
U 1 1 652A4A3C
P 11000 3800
F 0 "#PWR0101" H 11000 3550 50  0001 C CNN
F 1 "GND" H 11005 3627 50  0000 C CNN
F 2 "" H 11000 3800 50  0001 C CNN
F 3 "" H 11000 3800 50  0001 C CNN
	1    11000 3800
	1    0    0    -1  
$EndComp
Wire Wire Line
	11000 3800 11000 3650
Wire Wire Line
	11000 3650 10850 3650
Wire Wire Line
	11000 3650 11000 3550
Wire Wire Line
	11000 2750 10850 2750
Connection ~ 11000 3650
Wire Wire Line
	10850 2850 11000 2850
Connection ~ 11000 2850
Wire Wire Line
	11000 2850 11000 2750
Wire Wire Line
	10850 2950 11000 2950
Connection ~ 11000 2950
Wire Wire Line
	11000 2950 11000 2850
Wire Wire Line
	10850 3050 11000 3050
Connection ~ 11000 3050
Wire Wire Line
	11000 3050 11000 2950
Wire Wire Line
	10850 3150 11000 3150
Connection ~ 11000 3150
Wire Wire Line
	11000 3150 11000 3050
Wire Wire Line
	10850 3250 11000 3250
Connection ~ 11000 3250
Wire Wire Line
	11000 3250 11000 3150
Wire Wire Line
	10850 3350 11000 3350
Connection ~ 11000 3350
Wire Wire Line
	11000 3350 11000 3250
Wire Wire Line
	10850 3450 11000 3450
Connection ~ 11000 3450
Wire Wire Line
	11000 3450 11000 3350
Wire Wire Line
	10850 3550 11000 3550
Connection ~ 11000 3550
Wire Wire Line
	11000 3550 11000 3450
$Comp
L Connector_Generic:Conn_02x10_Odd_Even J3
U 1 1 6540305B
P 10550 4850
F 0 "J3" H 10600 5467 50  0000 C CNN
F 1 "Conn_02x10_Odd_Even" H 10600 5376 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_2x10_P2.54mm_Vertical" H 10550 4850 50  0001 C CNN
F 3 "~" H 10550 4850 50  0001 C CNN
	1    10550 4850
	1    0    0    -1  
$EndComp
Wire Wire Line
	10150 5250 10350 5250
Wire Wire Line
	10150 5350 10350 5350
$Comp
L power:GND #PWR0118
U 1 1 654030E2
P 11000 5500
F 0 "#PWR0118" H 11000 5250 50  0001 C CNN
F 1 "GND" H 11005 5327 50  0000 C CNN
F 2 "" H 11000 5500 50  0001 C CNN
F 3 "" H 11000 5500 50  0001 C CNN
	1    11000 5500
	1    0    0    -1  
$EndComp
Wire Wire Line
	11000 5500 11000 5350
Wire Wire Line
	11000 5350 10850 5350
Wire Wire Line
	11000 5350 11000 5250
Wire Wire Line
	11000 4450 10850 4450
Connection ~ 11000 5350
Wire Wire Line
	10850 4550 11000 4550
Connection ~ 11000 4550
Wire Wire Line
	11000 4550 11000 4450
Wire Wire Line
	10850 4650 11000 4650
Connection ~ 11000 4650
Wire Wire Line
	11000 4650 11000 4550
Wire Wire Line
	10850 4750 11000 4750
Connection ~ 11000 4750
Wire Wire Line
	11000 4750 11000 4650
Wire Wire Line
	10850 4850 11000 4850
Connection ~ 11000 4850
Wire Wire Line
	11000 4850 11000 4750
Wire Wire Line
	10850 4950 11000 4950
Connection ~ 11000 4950
Wire Wire Line
	11000 4950 11000 4850
Wire Wire Line
	10850 5050 11000 5050
Connection ~ 11000 5050
Wire Wire Line
	11000 5050 11000 4950
Wire Wire Line
	10850 5150 11000 5150
Connection ~ 11000 5150
Wire Wire Line
	11000 5150 11000 5050
Wire Wire Line
	10850 5250 11000 5250
Connection ~ 11000 5250
Wire Wire Line
	11000 5250 11000 5150
Wire Wire Line
	9950 2950 10150 2950
Wire Wire Line
	9950 3050 10350 3050
Wire Wire Line
	9950 3150 10350 3150
Wire Wire Line
	9950 3250 10350 3250
Wire Wire Line
	9950 3350 10350 3350
Wire Wire Line
	9950 3450 10350 3450
Wire Wire Line
	9950 3550 10350 3550
Wire Wire Line
	9950 3650 10350 3650
Wire Wire Line
	10150 2750 10150 2850
Wire Wire Line
	10150 2850 10150 2950
Connection ~ 10150 2850
Connection ~ 10150 2950
Wire Wire Line
	10150 2950 10350 2950
Wire Wire Line
	9950 4450 10350 4450
Wire Wire Line
	9950 4550 10350 4550
Wire Wire Line
	9950 4650 10350 4650
Wire Wire Line
	9950 4750 10350 4750
Wire Wire Line
	9950 4850 10350 4850
Wire Wire Line
	9950 4950 10350 4950
Wire Wire Line
	9950 5050 10350 5050
Wire Wire Line
	9950 5150 10150 5150
Wire Wire Line
	10150 5150 10150 5250
Connection ~ 10150 5150
Wire Wire Line
	10150 5150 10350 5150
Wire Wire Line
	10150 5350 10150 5250
Connection ~ 10150 5250
Wire Wire Line
	2250 950  1850 950 
Wire Wire Line
	1850 950  1850 1100
Connection ~ 2250 950 
Text GLabel 3650 1450 2    50   Input ~ 0
3V3
$Comp
L power:GND #PWR0102
U 1 1 651BFD97
P 3650 1700
F 0 "#PWR0102" H 3650 1450 50  0001 C CNN
F 1 "GND" H 3655 1527 50  0000 C CNN
F 2 "" H 3650 1700 50  0001 C CNN
F 3 "" H 3650 1700 50  0001 C CNN
	1    3650 1700
	1    0    0    -1  
$EndComp
Wire Wire Line
	3650 1550 3650 1700
Wire Wire Line
	3400 1450 3650 1450
Wire Wire Line
	3400 1550 3550 1550
Text GLabel 9950 4450 0    50   Input ~ 0
DATA15
Text GLabel 9950 4550 0    50   Input ~ 0
DATA14
Text GLabel 9950 4650 0    50   Input ~ 0
DATA13
Text GLabel 9950 4750 0    50   Input ~ 0
DATA12
Text GLabel 9950 4850 0    50   Input ~ 0
DATA11
Text GLabel 9950 4950 0    50   Input ~ 0
DATA10
Text GLabel 9950 5050 0    50   Input ~ 0
DATA9
Text GLabel 9950 5150 0    50   Input ~ 0
DATA8
Text GLabel 9950 2950 0    50   Input ~ 0
DATA7
Text GLabel 9950 3050 0    50   Input ~ 0
DATA6
Text GLabel 9950 3150 0    50   Input ~ 0
DATA5
Text GLabel 9950 3250 0    50   Input ~ 0
DATA4
Text GLabel 9950 3350 0    50   Input ~ 0
DATA3
Text GLabel 9950 3450 0    50   Input ~ 0
DATA2
Text GLabel 9950 3550 0    50   Input ~ 0
DATA1
Text GLabel 9950 3650 0    50   Input ~ 0
DATA0
Text GLabel 3650 1350 2    50   Input ~ 0
3V3
Wire Wire Line
	3400 1350 3650 1350
Wire Wire Line
	3550 1250 3550 1550
Connection ~ 3550 1550
Wire Wire Line
	3550 1550 3650 1550
Wire Wire Line
	3400 1250 3550 1250
$Comp
L Connector:Conn_01x04_Male J4
U 1 1 651AF4F9
P 3200 1350
F 0 "J4" H 3308 1631 50  0000 C CNN
F 1 "Conn_01x04_Male" H 3308 1540 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical" H 3200 1350 50  0001 C CNN
F 3 "~" H 3200 1350 50  0001 C CNN
	1    3200 1350
	1    0    0    -1  
$EndComp
Text GLabel 2150 2450 2    50   Input ~ 0
3V3
Wire Wire Line
	2150 2450 2000 2450
$Comp
L Device:C C1
U 1 1 651BB7D0
P 1600 2750
F 0 "C1" H 1715 2796 50  0000 L CNN
F 1 "22uF" H 1715 2705 50  0000 L CNN
F 2 "Capacitor_SMD:C_1210_3225Metric" H 1638 2600 50  0001 C CNN
F 3 "~" H 1600 2750 50  0001 C CNN
F 4 "C52923" H -4750 1200 50  0001 C CNN "LCSC"
	1    1600 2750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0115
U 1 1 651BB7DA
P 1600 3050
F 0 "#PWR0115" H 1600 2800 50  0001 C CNN
F 1 "GND" H 1605 2877 50  0000 C CNN
F 2 "" H 1600 3050 50  0001 C CNN
F 3 "" H 1600 3050 50  0001 C CNN
	1    1600 3050
	1    0    0    -1  
$EndComp
Wire Wire Line
	1600 2900 1600 3050
$Comp
L Device:C C4
U 1 1 651BB7E6
P 2000 2750
F 0 "C4" H 2115 2796 50  0000 L CNN
F 1 "22uF" H 2115 2705 50  0000 L CNN
F 2 "Capacitor_SMD:C_1210_3225Metric" H 2038 2600 50  0001 C CNN
F 3 "~" H 2000 2750 50  0001 C CNN
F 4 "C52923" H -4350 1200 50  0001 C CNN "LCSC"
	1    2000 2750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0116
U 1 1 651BB7F0
P 2000 3050
F 0 "#PWR0116" H 2000 2800 50  0001 C CNN
F 1 "GND" H 2005 2877 50  0000 C CNN
F 2 "" H 2000 3050 50  0001 C CNN
F 3 "" H 2000 3050 50  0001 C CNN
	1    2000 3050
	1    0    0    -1  
$EndComp
Wire Wire Line
	2000 2900 2000 3050
Wire Wire Line
	2000 2600 2000 2450
Wire Wire Line
	2000 2450 1600 2450
Wire Wire Line
	1600 2450 1600 2600
Connection ~ 2000 2450
$Comp
L Connector:Conn_01x01_Male J6
U 1 1 651DAA76
P 5200 3050
F 0 "J6" H 5308 3231 50  0000 C CNN
F 1 "Conn_01x01_Male" H 5308 3140 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x01_P2.54mm_Vertical" H 5200 3050 50  0001 C CNN
F 3 "~" H 5200 3050 50  0001 C CNN
	1    5200 3050
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0117
U 1 1 651DBDBF
P 5550 3150
F 0 "#PWR0117" H 5550 2900 50  0001 C CNN
F 1 "GND" H 5555 2977 50  0000 C CNN
F 2 "" H 5550 3150 50  0001 C CNN
F 3 "" H 5550 3150 50  0001 C CNN
	1    5550 3150
	1    0    0    -1  
$EndComp
Wire Wire Line
	5550 3150 5550 3050
Wire Wire Line
	5550 3050 5400 3050
$Comp
L Connector:Conn_01x01_Male J7
U 1 1 651A52AB
P 5200 3750
F 0 "J7" H 5308 3931 50  0000 C CNN
F 1 "Conn_01x01_Male" H 5308 3840 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x01_P2.54mm_Vertical" H 5200 3750 50  0001 C CNN
F 3 "~" H 5200 3750 50  0001 C CNN
	1    5200 3750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0119
U 1 1 651A52B5
P 5550 3850
F 0 "#PWR0119" H 5550 3600 50  0001 C CNN
F 1 "GND" H 5555 3677 50  0000 C CNN
F 2 "" H 5550 3850 50  0001 C CNN
F 3 "" H 5550 3850 50  0001 C CNN
	1    5550 3850
	1    0    0    -1  
$EndComp
Wire Wire Line
	5550 3850 5550 3750
Wire Wire Line
	5550 3750 5400 3750
$EndSCHEMATC
