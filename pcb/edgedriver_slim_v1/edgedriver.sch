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
Text GLabel 8750 3850 2    50   Input ~ 0
STMRX
Wire Wire Line
	8750 3850 8650 3850
Text GLabel 8750 3750 2    50   Input ~ 0
STMTX
Wire Wire Line
	8750 3750 8650 3750
Wire Wire Line
	8750 4250 8650 4250
Wire Wire Line
	8750 4150 8650 4150
$Comp
L power:GND #PWR0103
U 1 1 5EF958D2
P 8050 4750
F 0 "#PWR0103" H 8050 4500 50  0001 C CNN
F 1 "GND" H 8055 4577 50  0000 C CNN
F 2 "" H 8050 4750 50  0001 C CNN
F 3 "" H 8050 4750 50  0001 C CNN
	1    8050 4750
	1    0    0    -1  
$EndComp
Wire Wire Line
	8050 4750 8050 4650
Text GLabel 8350 1050 2    50   Input ~ 0
3V3
$Comp
L Device:R R6
U 1 1 5EFE5F67
P 7150 1550
F 0 "R6" V 7050 1550 50  0000 C CNN
F 1 "10k" V 7150 1550 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric" V 7080 1550 50  0001 C CNN
F 3 "~" H 7150 1550 50  0001 C CNN
F 4 "C25744" H -500 -2300 50  0001 C CNN "LCSC"
	1    7150 1550
	0    1    1    0   
$EndComp
$Comp
L Device:R R7
U 1 1 5EFE8183
P 7150 1750
F 0 "R7" V 7250 1750 50  0000 C CNN
F 1 "10k" V 7150 1750 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric" V 7080 1750 50  0001 C CNN
F 3 "~" H 7150 1750 50  0001 C CNN
F 4 "C25744" H -500 -2300 50  0001 C CNN "LCSC"
	1    7150 1750
	0    1    1    0   
$EndComp
Wire Wire Line
	7300 1550 7450 1550
Wire Wire Line
	7450 1750 7300 1750
Text GLabel 6800 1550 0    50   Input ~ 0
3V3
$Comp
L power:GND #PWR0106
U 1 1 5EFEB61D
P 6800 2050
F 0 "#PWR0106" H 6800 1800 50  0001 C CNN
F 1 "GND" H 6805 1877 50  0000 C CNN
F 2 "" H 6800 2050 50  0001 C CNN
F 3 "" H 6800 2050 50  0001 C CNN
	1    6800 2050
	1    0    0    -1  
$EndComp
Wire Wire Line
	6800 2050 6800 1750
Wire Wire Line
	7000 1750 6800 1750
Wire Wire Line
	7000 1550 6800 1550
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
Text GLabel 8750 4150 2    50   Input ~ 0
SWDIO
Text GLabel 8750 4250 2    50   Input ~ 0
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
	8250 1050 8250 1350
Connection ~ 8250 1050
Wire Wire Line
	8250 1050 8350 1050
Wire Wire Line
	8150 1050 8150 1350
Wire Wire Line
	8150 1050 8250 1050
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
P 8050 2950
F 0 "U3" H 8050 1261 50  0000 C CNN
F 1 "STM32F030C8Tx" H 8050 1170 50  0000 C CNN
F 2 "Package_QFP:LQFP-48_7x7mm_P0.5mm" H 7550 1450 50  0001 R CNN
F 3 "http://www.st.com/st-web-ui/static/active/en/resource/technical/document/datasheet/DM00088500.pdf" H 8050 2950 50  0001 C CNN
F 4 "C23922" H 8050 2950 50  0001 C CNN "LCSC"
	1    8050 2950
	1    0    0    -1  
$EndComp
Wire Wire Line
	8150 1050 8050 1050
Wire Wire Line
	7950 1050 7950 1350
Connection ~ 8150 1050
Wire Wire Line
	8050 1350 8050 1050
Connection ~ 8050 1050
Wire Wire Line
	8050 1050 7950 1050
Connection ~ 4350 1500
Wire Wire Line
	8150 4550 8150 4650
Wire Wire Line
	8150 4650 8050 4650
Connection ~ 8050 4650
Wire Wire Line
	8050 4650 8050 4550
Wire Wire Line
	8050 4650 7950 4650
Wire Wire Line
	7950 4650 7950 4550
$Comp
L Connector_Generic:Conn_02x05_Odd_Even J1
U 1 1 64F1FC83
P 10150 2950
F 0 "J1" H 10200 3367 50  0000 C CNN
F 1 "Conn_02x05_Odd_Even" H 10200 3276 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_2x05_P2.54mm_Vertical" H 10150 2950 50  0001 C CNN
F 3 "~" H 10150 2950 50  0001 C CNN
	1    10150 2950
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0101
U 1 1 64F209B9
P 10550 3400
F 0 "#PWR0101" H 10550 3150 50  0001 C CNN
F 1 "GND" H 10555 3227 50  0000 C CNN
F 2 "" H 10550 3400 50  0001 C CNN
F 3 "" H 10550 3400 50  0001 C CNN
	1    10550 3400
	1    0    0    -1  
$EndComp
Wire Wire Line
	10700 2050 10700 2850
Wire Wire Line
	10700 2850 10450 2850
Wire Wire Line
	10800 2150 10800 2950
Wire Wire Line
	10800 2950 10450 2950
Wire Wire Line
	10900 2250 10900 3050
Wire Wire Line
	10900 3050 10450 3050
Wire Wire Line
	9800 3550 9800 2350
Wire Wire Line
	9800 2350 11000 2350
Wire Wire Line
	11000 2350 11000 3150
Wire Wire Line
	11000 3150 10450 3150
Text GLabel 9750 1700 0    50   Input ~ 0
5V
Wire Wire Line
	9750 1700 9900 1700
Wire Wire Line
	9900 1700 9900 2750
$Comp
L Regulator_Linear:AP2112K-3.3 U2
U 1 1 64F3757C
P 1800 1050
F 0 "U2" H 1800 1392 50  0000 C CNN
F 1 "AP2112K-3.3" H 1800 1301 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-5" H 1800 1375 50  0001 C CNN
F 3 "https://www.diodes.com/assets/Datasheets/AP2112.pdf" H 1800 1150 50  0001 C CNN
F 4 "C82942" H 1800 1050 50  0001 C CNN "LCSC"
	1    1800 1050
	1    0    0    -1  
$EndComp
Wire Wire Line
	1500 950  1300 950 
Wire Wire Line
	1300 950  1300 1050
Wire Wire Line
	1300 1050 1500 1050
$Comp
L power:GND #PWR0102
U 1 1 64F3D77D
P 1800 1500
F 0 "#PWR0102" H 1800 1250 50  0001 C CNN
F 1 "GND" H 1805 1327 50  0000 C CNN
F 2 "" H 1800 1500 50  0001 C CNN
F 3 "" H 1800 1500 50  0001 C CNN
	1    1800 1500
	1    0    0    -1  
$EndComp
Wire Wire Line
	1800 1500 1800 1350
Text GLabel 1050 950  0    50   Input ~ 0
5V
Text GLabel 2450 950  2    50   Input ~ 0
3V3
Wire Wire Line
	2450 950  2300 950 
Wire Wire Line
	1050 950  1150 950 
Connection ~ 1300 950 
$Comp
L Device:C C2
U 1 1 64F4E33A
P 1150 1250
F 0 "C2" H 1265 1296 50  0000 L CNN
F 1 "1uF" H 1265 1205 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 1188 1100 50  0001 C CNN
F 3 "~" H 1150 1250 50  0001 C CNN
F 4 "C52923" H -5200 -300 50  0001 C CNN "LCSC"
	1    1150 1250
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0104
U 1 1 64F4E5BA
P 1150 1550
F 0 "#PWR0104" H 1150 1300 50  0001 C CNN
F 1 "GND" H 1155 1377 50  0000 C CNN
F 2 "" H 1150 1550 50  0001 C CNN
F 3 "" H 1150 1550 50  0001 C CNN
	1    1150 1550
	1    0    0    -1  
$EndComp
Wire Wire Line
	1150 1400 1150 1550
$Comp
L Device:C C3
U 1 1 64F5F0FA
P 2300 1250
F 0 "C3" H 2415 1296 50  0000 L CNN
F 1 "1uF" H 2415 1205 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 2338 1100 50  0001 C CNN
F 3 "~" H 2300 1250 50  0001 C CNN
F 4 "C52923" H -4050 -300 50  0001 C CNN "LCSC"
	1    2300 1250
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0105
U 1 1 64F5F390
P 2300 1550
F 0 "#PWR0105" H 2300 1300 50  0001 C CNN
F 1 "GND" H 2305 1377 50  0000 C CNN
F 2 "" H 2300 1550 50  0001 C CNN
F 3 "" H 2300 1550 50  0001 C CNN
	1    2300 1550
	1    0    0    -1  
$EndComp
Wire Wire Line
	2300 1400 2300 1550
Wire Wire Line
	2300 1100 2300 950 
Connection ~ 2300 950 
Wire Wire Line
	2300 950  2100 950 
Wire Wire Line
	1150 950  1150 1100
Connection ~ 1150 950 
Wire Wire Line
	1150 950  1300 950 
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
	10450 2750 10550 2750
Wire Wire Line
	10550 2750 10550 3400
Wire Wire Line
	9950 2750 9900 2750
Wire Wire Line
	9950 3150 9750 3150
Wire Wire Line
	9950 3050 9650 3050
Wire Wire Line
	10900 2250 9700 2250
Wire Wire Line
	3700 3350 3700 3650
$Comp
L power:GND #PWR0114
U 1 1 64F2D09E
P 7050 2400
F 0 "#PWR0114" H 7050 2150 50  0001 C CNN
F 1 "GND" H 7055 2227 50  0000 C CNN
F 2 "" H 7050 2400 50  0001 C CNN
F 3 "" H 7050 2400 50  0001 C CNN
	1    7050 2400
	1    0    0    -1  
$EndComp
Wire Wire Line
	7450 2150 7050 2150
Wire Wire Line
	7050 2150 7050 2400
Wire Wire Line
	7450 2250 7300 2250
Text GLabel 7300 2250 0    50   Input ~ 0
3V3
Wire Notes Line
	7350 2100 6950 2100
Wire Notes Line
	6950 2100 6950 2700
Wire Notes Line
	6950 2700 7350 2700
Wire Notes Line
	7350 2700 7350 2100
Text Notes 6600 2900 0    50   ~ 0
compatibility with\nstm32f072
Wire Wire Line
	8650 2850 8700 2850
Wire Wire Line
	8650 3550 9050 3550
Wire Wire Line
	9750 3450 9750 3150
Wire Wire Line
	8650 3450 9000 3450
Wire Wire Line
	9700 3350 9700 2250
Wire Wire Line
	8650 3350 8950 3350
Wire Wire Line
	9650 3250 9650 3050
Wire Wire Line
	8650 3250 8900 3250
Wire Wire Line
	9600 3150 9600 2150
Wire Wire Line
	9600 2150 10800 2150
Wire Wire Line
	8650 3150 8850 3150
Wire Wire Line
	9550 3050 9550 2950
Wire Wire Line
	9550 2950 9950 2950
Wire Wire Line
	8650 3050 8800 3050
Wire Wire Line
	9500 2950 9500 2050
Wire Wire Line
	9500 2050 10700 2050
Wire Wire Line
	8650 2950 8750 2950
Text GLabel 8800 1650 2    50   Input ~ 0
P0
Text GLabel 8950 2100 2    50   Input ~ 0
P3
Text GLabel 8900 1950 2    50   Input ~ 0
P2
Text GLabel 8850 1800 2    50   Input ~ 0
P1
Text GLabel 9000 2250 2    50   Input ~ 0
P4
Text GLabel 9150 2700 2    50   Input ~ 0
P7
Text GLabel 9100 2550 2    50   Input ~ 0
P6
Text GLabel 9050 2400 2    50   Input ~ 0
P5
Wire Wire Line
	9150 2700 9050 2700
Wire Wire Line
	9050 2700 9050 3550
Connection ~ 9050 3550
Wire Wire Line
	9050 3550 9800 3550
Wire Wire Line
	9100 2550 9000 2550
Wire Wire Line
	9000 2550 9000 3450
Connection ~ 9000 3450
Wire Wire Line
	9000 3450 9750 3450
Wire Wire Line
	9050 2400 8950 2400
Wire Wire Line
	8950 2400 8950 3350
Connection ~ 8950 3350
Wire Wire Line
	8950 3350 9700 3350
Wire Wire Line
	9000 2250 8900 2250
Wire Wire Line
	8900 2250 8900 3250
Connection ~ 8900 3250
Wire Wire Line
	8900 3250 9650 3250
Wire Wire Line
	8950 2100 8850 2100
Wire Wire Line
	8850 2100 8850 3150
Connection ~ 8850 3150
Wire Wire Line
	8850 3150 9600 3150
Wire Wire Line
	8900 1950 8800 1950
Wire Wire Line
	8800 1950 8800 3050
Connection ~ 8800 3050
Wire Wire Line
	8800 3050 9550 3050
Wire Wire Line
	8850 1800 8750 1800
Wire Wire Line
	8750 1800 8750 2950
Connection ~ 8750 2950
Wire Wire Line
	8750 2950 9500 2950
Wire Wire Line
	8800 1650 8700 1650
Wire Wire Line
	8700 1650 8700 2850
Connection ~ 8700 2850
Wire Wire Line
	8700 2850 9950 2850
$EndSCHEMATC
