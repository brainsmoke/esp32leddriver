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
P 5050 5100
F 0 "U1" H 5025 6437 60  0000 C CNN
F 1 "ESP32-WROVER" H 5025 6331 60  0000 C CNN
F 2 "esp32-wrover:XCVR_ESP32-WROVER" H 5500 4800 60  0001 C CNN
F 3 "" H 5500 4800 60  0001 C CNN
	1    5050 5100
	1    0    0    -1  
$EndComp
$Comp
L Interface_USB:CP2104 U4
U 1 1 5EF3BE6A
P 2800 1950
F 0 "U4" H 2800 864 50  0000 C CNN
F 1 "CH9102F" H 2800 773 50  0000 C CNN
F 2 "Package_DFN_QFN:QFN-24-1EP_4x4mm_P0.5mm_EP2.6x2.6mm" H 2950 1000 50  0001 L CNN
F 3 "https://datasheet.lcsc.com/lcsc/2108181630_WCH-Jiangsu-Qin-Heng-CH9102F_C2858418.pdf" H 2250 3200 50  0001 C CNN
F 4 "C2858418" H 0   0   50  0001 C CNN "LCSC"
	1    2800 1950
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x08_Male J1
U 1 1 5EF3BFB6
P 10400 4600
F 0 "J1" H 10506 5078 50  0000 C CNN
F 1 "Conn_01x08_Male" H 10506 4987 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x08_P2.54mm_Vertical" H 10400 4600 50  0001 C CNN
F 3 "~" H 10400 4600 50  0001 C CNN
	1    10400 4600
	-1   0    0    1   
$EndComp
$Comp
L Connector:Conn_01x04_Male J3
U 1 1 5EF72B41
P 10400 5800
F 0 "J3" H 10506 6078 50  0000 C CNN
F 1 "Conn_01x04_Male" H 10506 5987 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical" H 10400 5800 50  0001 C CNN
F 3 "~" H 10400 5800 50  0001 C CNN
	1    10400 5800
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR0101
U 1 1 5EF8F1ED
P 10050 5200
F 0 "#PWR0101" H 10050 4950 50  0001 C CNN
F 1 "GND" H 10055 5027 50  0000 C CNN
F 2 "" H 10050 5200 50  0001 C CNN
F 3 "" H 10050 5200 50  0001 C CNN
	1    10050 5200
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR0102
U 1 1 5EF8F2EE
P 10050 4100
F 0 "#PWR0102" H 10050 3950 50  0001 C CNN
F 1 "+5V" H 10065 4273 50  0000 C CNN
F 2 "" H 10050 4100 50  0001 C CNN
F 3 "" H 10050 4100 50  0001 C CNN
	1    10050 4100
	1    0    0    -1  
$EndComp
Wire Wire Line
	10050 4100 10050 4200
Wire Wire Line
	10050 4200 10200 4200
Wire Wire Line
	10050 4200 10050 4300
Wire Wire Line
	10050 4300 10200 4300
Connection ~ 10050 4200
Wire Wire Line
	10050 5200 10050 4900
Wire Wire Line
	10050 4900 10200 4900
Wire Wire Line
	10050 4900 10050 4800
Wire Wire Line
	10050 4800 10200 4800
Connection ~ 10050 4900
Wire Wire Line
	9850 4900 9850 5600
Wire Wire Line
	9850 5600 10200 5600
Wire Wire Line
	9750 5000 9750 5700
Wire Wire Line
	9750 5700 10200 5700
Wire Wire Line
	9650 5100 9650 5800
Wire Wire Line
	9650 5800 10200 5800
Wire Wire Line
	9550 5200 9550 5900
Wire Wire Line
	9550 5900 10200 5900
Text GLabel 9100 5500 2    50   Input ~ 0
STMRX
Wire Wire Line
	9000 4500 9650 4500
Wire Wire Line
	9000 4700 9850 4700
Wire Wire Line
	9000 4800 9950 4800
Wire Wire Line
	9000 4900 9850 4900
Wire Wire Line
	9000 5000 9750 5000
Wire Wire Line
	9000 5100 9650 5100
Wire Wire Line
	9000 5200 9550 5200
Wire Wire Line
	9100 5500 9000 5500
Text GLabel 9100 5400 2    50   Input ~ 0
STMTX
Wire Wire Line
	9100 5400 9000 5400
Wire Wire Line
	9100 5900 9000 5900
Text GLabel 10000 6450 0    50   Input ~ 0
SWDIO
Wire Wire Line
	9100 5800 9000 5800
Text GLabel 10000 6350 0    50   Input ~ 0
SWDCLK
$Comp
L power:GND #PWR0103
U 1 1 5EF958D2
P 8400 6500
F 0 "#PWR0103" H 8400 6250 50  0001 C CNN
F 1 "GND" H 8405 6327 50  0000 C CNN
F 2 "" H 8400 6500 50  0001 C CNN
F 3 "" H 8400 6500 50  0001 C CNN
	1    8400 6500
	1    0    0    -1  
$EndComp
Wire Wire Line
	8400 6500 8400 6300
Text GLabel 8700 2700 2    50   Input ~ 0
3V3
Connection ~ 8400 2700
Wire Wire Line
	8400 2700 8300 2700
$Comp
L Device:C C2
U 1 1 5EF972CD
P 1450 6500
F 0 "C2" H 1565 6546 50  0000 L CNN
F 1 "10uF" H 1565 6455 50  0000 L CNN
F 2 "esp32-wrover:C_0603_1608Metric" H 1488 6350 50  0001 C CNN
F 3 "~" H 1450 6500 50  0001 C CNN
F 4 "C19702" H 0   0   50  0001 C CNN "LCSC"
	1    1450 6500
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0104
U 1 1 5EF973B1
P 1850 6850
F 0 "#PWR0104" H 1850 6600 50  0001 C CNN
F 1 "GND" H 1855 6677 50  0000 C CNN
F 2 "" H 1850 6850 50  0001 C CNN
F 3 "" H 1850 6850 50  0001 C CNN
	1    1850 6850
	1    0    0    -1  
$EndComp
Wire Wire Line
	1850 6850 1850 6700
Wire Wire Line
	1550 6250 1450 6250
Wire Wire Line
	1450 6250 1450 6350
Connection ~ 1850 6700
Wire Wire Line
	1850 6700 1450 6700
Wire Wire Line
	1450 6700 1450 6650
$Comp
L Device:R R6
U 1 1 5EFE5F67
P 7500 3200
F 0 "R6" V 7400 3200 50  0000 C CNN
F 1 "10k" V 7500 3200 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric" V 7430 3200 50  0001 C CNN
F 3 "~" H 7500 3200 50  0001 C CNN
F 4 "C25744" H -150 -650 50  0001 C CNN "LCSC"
	1    7500 3200
	0    1    1    0   
$EndComp
$Comp
L Device:R R7
U 1 1 5EFE8183
P 7500 3400
F 0 "R7" V 7600 3400 50  0000 C CNN
F 1 "10k" V 7500 3400 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric" V 7430 3400 50  0001 C CNN
F 3 "~" H 7500 3400 50  0001 C CNN
F 4 "C25744" H -150 -650 50  0001 C CNN "LCSC"
	1    7500 3400
	0    1    1    0   
$EndComp
Wire Wire Line
	7650 3200 7800 3200
Wire Wire Line
	7800 3400 7650 3400
Text GLabel 7150 3200 0    50   Input ~ 0
3V3
$Comp
L power:GND #PWR0106
U 1 1 5EFEB61D
P 7150 3700
F 0 "#PWR0106" H 7150 3450 50  0001 C CNN
F 1 "GND" H 7155 3527 50  0000 C CNN
F 2 "" H 7150 3700 50  0001 C CNN
F 3 "" H 7150 3700 50  0001 C CNN
	1    7150 3700
	1    0    0    -1  
$EndComp
Wire Wire Line
	7150 3700 7150 3400
Wire Wire Line
	7350 3400 7150 3400
Wire Wire Line
	7350 3200 7150 3200
Text GLabel 7200 4300 2    50   Input ~ 0
3V3
$Comp
L Device:C C8
U 1 1 5EFEF351
P 7150 4500
F 0 "C8" H 7265 4546 50  0000 L CNN
F 1 "100nF" H 7265 4455 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 7188 4350 50  0001 C CNN
F 3 "~" H 7150 4500 50  0001 C CNN
F 4 "C1525" H -300 -450 50  0001 C CNN "LCSC"
	1    7150 4500
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0107
U 1 1 5EFEF35B
P 7150 4750
F 0 "#PWR0107" H 7150 4500 50  0001 C CNN
F 1 "GND" H 7155 4577 50  0000 C CNN
F 2 "" H 7150 4750 50  0001 C CNN
F 3 "" H 7150 4750 50  0001 C CNN
	1    7150 4750
	1    0    0    -1  
$EndComp
Wire Wire Line
	7200 4300 7150 4300
Wire Wire Line
	7150 4300 7150 4350
Wire Wire Line
	7150 4650 7150 4750
Text GLabel 3100 4600 2    50   Input ~ 0
3V3
$Comp
L Device:C C5
U 1 1 5EFF7578
P 3050 4800
F 0 "C5" H 3165 4846 50  0000 L CNN
F 1 "100nF" H 3165 4755 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 3088 4650 50  0001 C CNN
F 3 "~" H 3050 4800 50  0001 C CNN
F 4 "C1525" H 0   0   50  0001 C CNN "LCSC"
	1    3050 4800
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0108
U 1 1 5EFF757E
P 3050 5050
F 0 "#PWR0108" H 3050 4800 50  0001 C CNN
F 1 "GND" H 3055 4877 50  0000 C CNN
F 2 "" H 3050 5050 50  0001 C CNN
F 3 "" H 3050 5050 50  0001 C CNN
	1    3050 5050
	1    0    0    -1  
$EndComp
Wire Wire Line
	3100 4600 3050 4600
Wire Wire Line
	3050 4600 3050 4650
Wire Wire Line
	3050 4950 3050 5000
Wire Wire Line
	4150 4350 3900 4350
Wire Wire Line
	3900 4350 3900 5750
Wire Wire Line
	4150 5750 3900 5750
Connection ~ 3900 5750
Wire Wire Line
	3900 5750 3900 6450
$Comp
L power:GND #PWR0109
U 1 1 5EFFF208
P 3900 6450
F 0 "#PWR0109" H 3900 6200 50  0001 C CNN
F 1 "GND" H 3905 6277 50  0000 C CNN
F 2 "" H 3900 6450 50  0001 C CNN
F 3 "" H 3900 6450 50  0001 C CNN
	1    3900 6450
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0110
U 1 1 5EFFF22B
P 6450 4500
F 0 "#PWR0110" H 6450 4250 50  0001 C CNN
F 1 "GND" H 6455 4327 50  0000 C CNN
F 2 "" H 6450 4500 50  0001 C CNN
F 3 "" H 6450 4500 50  0001 C CNN
	1    6450 4500
	1    0    0    -1  
$EndComp
Text GLabel 3700 4100 0    50   Input ~ 0
3V3
Wire Wire Line
	3700 4100 3800 4100
Wire Wire Line
	3800 4100 3800 4450
Wire Wire Line
	3800 4450 4150 4450
Text GLabel 6000 4850 2    50   Input ~ 0
ESPRX
Text GLabel 6000 4750 2    50   Input ~ 0
ESPTX
Wire Wire Line
	6450 4450 6450 4500
Wire Wire Line
	5900 4450 6450 4450
Wire Wire Line
	6450 4350 6450 4450
Wire Wire Line
	5900 4350 6450 4350
Connection ~ 6450 4450
Wire Wire Line
	6000 4750 5900 4750
Wire Wire Line
	6000 4850 5900 4850
Text GLabel 4100 1850 2    50   Input ~ 0
ESPRX
Text GLabel 4100 1950 2    50   Input ~ 0
ESPTX
Wire Wire Line
	3700 1850 3500 1850
Text GLabel 6300 5750 2    50   Input ~ 0
ESPRST
Text GLabel 3750 4550 0    50   Input ~ 0
ESPEN
Wire Wire Line
	4150 4550 3750 4550
$Comp
L power:GND #PWR0111
U 1 1 5EFE96A0
P 2800 3050
F 0 "#PWR0111" H 2800 2800 50  0001 C CNN
F 1 "GND" H 2805 2877 50  0000 C CNN
F 2 "" H 2800 3050 50  0001 C CNN
F 3 "" H 2800 3050 50  0001 C CNN
	1    2800 3050
	1    0    0    -1  
$EndComp
Wire Wire Line
	2800 2950 2800 3000
$Comp
L Device:C C7
U 1 1 5EFEC07D
P 6350 1550
F 0 "C7" H 6465 1596 50  0000 L CNN
F 1 "1uF" H 6465 1505 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 6388 1400 50  0001 C CNN
F 3 "~" H 6350 1550 50  0001 C CNN
F 4 "C52923" H 0   0   50  0001 C CNN "LCSC"
	1    6350 1550
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0112
U 1 1 5EFEE86F
P 6350 1850
F 0 "#PWR0112" H 6350 1600 50  0001 C CNN
F 1 "GND" H 6355 1677 50  0000 C CNN
F 2 "" H 6350 1850 50  0001 C CNN
F 3 "" H 6350 1850 50  0001 C CNN
	1    6350 1850
	1    0    0    -1  
$EndComp
Wire Wire Line
	6350 1700 6350 1850
Wire Wire Line
	2900 2950 2900 3000
Wire Wire Line
	2900 3000 2800 3000
Connection ~ 2800 3000
Wire Wire Line
	2800 3000 2800 3050
$Comp
L Transistor_FET:BSS138 Q1
U 1 1 5EFF9465
P 5550 1550
F 0 "Q1" H 5755 1596 50  0000 L CNN
F 1 "AO3400" H 5755 1505 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 5750 1475 50  0001 L CIN
F 3 "" H 5550 1550 50  0001 L CNN
F 4 "C20917" H 0   0   50  0001 C CNN "LCSC"
	1    5550 1550
	1    0    0    -1  
$EndComp
$Comp
L Device:R R3
U 1 1 5EFF9670
P 5050 1550
F 0 "R3" V 4950 1550 50  0000 C CNN
F 1 "10k" V 5050 1550 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric" V 4980 1550 50  0001 C CNN
F 3 "~" H 5050 1550 50  0001 C CNN
F 4 "C25744" H 0   0   50  0001 C CNN "LCSC"
	1    5050 1550
	0    1    1    0   
$EndComp
Wire Wire Line
	5200 1550 5350 1550
$Comp
L Transistor_FET:BSS138 Q2
U 1 1 5F0048E8
P 5550 2150
F 0 "Q2" H 5755 2196 50  0000 L CNN
F 1 "AO3400" H 5755 2105 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 5750 2075 50  0001 L CIN
F 3 "" H 5550 2150 50  0001 L CNN
F 4 "C20917" H 0   0   50  0001 C CNN "LCSC"
	1    5550 2150
	1    0    0    -1  
$EndComp
$Comp
L Device:R R4
U 1 1 5F0048EE
P 5050 2150
F 0 "R4" V 4950 2150 50  0000 C CNN
F 1 "10k" V 5050 2150 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric" V 4980 2150 50  0001 C CNN
F 3 "~" H 5050 2150 50  0001 C CNN
F 4 "C25744" H 0   0   50  0001 C CNN "LCSC"
	1    5050 2150
	0    1    1    0   
$EndComp
Wire Wire Line
	5200 2150 5350 2150
Text GLabel 5800 1900 2    50   Input ~ 0
ESPRST
Text GLabel 6550 1300 2    50   Input ~ 0
ESPEN
$Comp
L Device:R R1
U 1 1 5F0136EA
P 3850 1850
F 0 "R1" V 3750 1850 50  0000 C CNN
F 1 "1k" V 3850 1850 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric" V 3780 1850 50  0001 C CNN
F 3 "~" H 3850 1850 50  0001 C CNN
F 4 "C11702" H 0   0   50  0001 C CNN "LCSC"
	1    3850 1850
	0    1    1    0   
$EndComp
Wire Wire Line
	4000 1850 4100 1850
Wire Wire Line
	3500 1950 4100 1950
Wire Wire Line
	3500 2150 4850 2150
Wire Wire Line
	4850 2150 4850 1800
Wire Wire Line
	4850 1800 5650 1800
Wire Wire Line
	5650 1800 5650 1750
Connection ~ 4850 2150
Wire Wire Line
	4850 2150 4900 2150
Wire Wire Line
	3500 1550 4700 1550
Wire Wire Line
	4700 1550 4700 2400
Wire Wire Line
	4700 2400 5650 2400
Wire Wire Line
	5650 2400 5650 2350
Wire Wire Line
	5650 1950 5650 1900
Wire Wire Line
	5650 1350 5650 1300
Wire Wire Line
	5900 5750 6300 5750
$Comp
L Device:R R5
U 1 1 5F05100B
P 7250 950
F 0 "R5" V 7150 950 50  0000 C CNN
F 1 "10k" V 7250 950 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric" V 7180 950 50  0001 C CNN
F 3 "~" H 7250 950 50  0001 C CNN
F 4 "C25744" H 900 -150 50  0001 C CNN "LCSC"
	1    7250 950 
	1    0    0    -1  
$EndComp
Wire Wire Line
	7250 1100 7250 1150
Text GLabel 7300 750  2    50   Input ~ 0
3V3
Wire Wire Line
	7300 750  7250 750 
Wire Wire Line
	7250 750  7250 800 
Wire Wire Line
	6550 1300 6350 1300
Connection ~ 6350 1300
Wire Wire Line
	6350 1400 6350 1300
Wire Wire Line
	5800 1900 5650 1900
Wire Wire Line
	6350 1300 5650 1300
Text GLabel 3700 5850 0    50   Input ~ 0
STMRX
Text GLabel 3700 4950 0    50   Input ~ 0
STMTX
Wire Wire Line
	1700 1350 2100 1350
Wire Wire Line
	1700 1550 2100 1550
Wire Wire Line
	2600 1050 2600 900 
Wire Wire Line
	2600 900  2700 900 
Wire Wire Line
	2800 900  2800 1050
$Comp
L Device:C C6
U 1 1 5F0A26DB
P 3800 800
F 0 "C6" H 3915 846 50  0000 L CNN
F 1 "1uF" H 3915 755 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 3838 650 50  0001 C CNN
F 3 "~" H 3800 800 50  0001 C CNN
F 4 "C52923" H 0   0   50  0001 C CNN "LCSC"
	1    3800 800 
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0113
U 1 1 5F0A26E1
P 3800 1100
F 0 "#PWR0113" H 3800 850 50  0001 C CNN
F 1 "GND" H 3805 927 50  0000 C CNN
F 2 "" H 3800 1100 50  0001 C CNN
F 3 "" H 3800 1100 50  0001 C CNN
	1    3800 1100
	1    0    0    -1  
$EndComp
Wire Wire Line
	3800 950  3800 1100
Wire Wire Line
	3800 650  3800 550 
Wire Wire Line
	3800 550  3550 550 
Wire Wire Line
	2700 550  2700 900 
Connection ~ 2700 900 
Wire Wire Line
	2700 900  2800 900 
$Comp
L Device:C C3
U 1 1 5F0AD19E
P 3550 800
F 0 "C3" H 3665 846 50  0000 L CNN
F 1 "100nF" H 3665 755 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 3588 650 50  0001 C CNN
F 3 "~" H 3550 800 50  0001 C CNN
F 4 "C1525" H 0   0   50  0001 C CNN "LCSC"
	1    3550 800 
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0114
U 1 1 5F0AD1A4
P 3550 1100
F 0 "#PWR0114" H 3550 850 50  0001 C CNN
F 1 "GND" H 3555 927 50  0000 C CNN
F 2 "" H 3550 1100 50  0001 C CNN
F 3 "" H 3550 1100 50  0001 C CNN
	1    3550 1100
	1    0    0    -1  
$EndComp
Wire Wire Line
	3550 950  3550 1100
$Comp
L power:GND #PWR0115
U 1 1 5F0BE6D3
P -100 3250
F 0 "#PWR0115" H -100 3000 50  0001 C CNN
F 1 "GND" H -95 3077 50  0000 C CNN
F 2 "" H -100 3250 50  0001 C CNN
F 3 "" H -100 3250 50  0001 C CNN
	1    -100 3250
	1    0    0    -1  
$EndComp
Wire Wire Line
	-100 2850 -100 3250
$Comp
L Device:C C1
U 1 1 5F0CAAEE
P 1700 2400
F 0 "C1" H 1815 2446 50  0000 L CNN
F 1 "1uF" H 1815 2355 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 1738 2250 50  0001 C CNN
F 3 "~" H 1700 2400 50  0001 C CNN
F 4 "C52923" H 350 0   50  0001 C CNN "LCSC"
	1    1700 2400
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0116
U 1 1 5F0CAAF4
P 1700 2700
F 0 "#PWR0116" H 1700 2450 50  0001 C CNN
F 1 "GND" H 1705 2527 50  0000 C CNN
F 2 "" H 1700 2700 50  0001 C CNN
F 3 "" H 1700 2700 50  0001 C CNN
	1    1700 2700
	1    0    0    -1  
$EndComp
Wire Wire Line
	1700 2550 1700 2700
Entry Bus Bus
	-1250 -1000 -1150 -900
Text GLabel 2600 550  0    50   Input ~ 0
VIO
Wire Wire Line
	2600 550  2700 550 
Connection ~ 2700 550 
Text GLabel 4100 2450 2    50   Input ~ 0
VIO
Wire Wire Line
	3700 2450 3500 2450
$Comp
L Device:R R2
U 1 1 5F0EC236
P 3850 2450
F 0 "R2" V 3750 2450 50  0000 C CNN
F 1 "10k" V 3850 2450 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric" V 3780 2450 50  0001 C CNN
F 3 "~" H 3850 2450 50  0001 C CNN
F 4 "C25744" H 0   0   50  0001 C CNN "LCSC"
	1    3850 2450
	0    1    1    0   
$EndComp
Wire Wire Line
	4000 2450 4100 2450
Wire Wire Line
	9950 4800 9950 4400
Wire Wire Line
	9950 4400 10200 4400
Wire Wire Line
	9850 4700 9850 4500
Wire Wire Line
	9850 4500 10200 4500
Wire Wire Line
	9000 4600 10200 4600
Wire Wire Line
	9650 4500 9650 4650
Wire Wire Line
	10100 4650 10100 4700
Wire Wire Line
	10100 4700 10200 4700
Wire Wire Line
	9650 4650 10100 4650
$Comp
L Connector:Conn_01x02_Male J4
U 1 1 5EFF6FE7
P 10400 6450
F 0 "J4" H 10373 6330 50  0000 R CNN
F 1 "Conn_01x02_Male" H 10373 6421 50  0000 R CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical" H 10400 6450 50  0001 C CNN
F 3 "~" H 10400 6450 50  0001 C CNN
	1    10400 6450
	-1   0    0    1   
$EndComp
Wire Wire Line
	10000 6350 10200 6350
Wire Wire Line
	10000 6450 10200 6450
Text GLabel 9100 5800 2    50   Input ~ 0
SWDIO
Text GLabel 9100 5900 2    50   Input ~ 0
SWDCLK
Wire Wire Line
	3700 4950 4150 4950
Wire Wire Line
	3700 5850 4150 5850
$Comp
L Device:C C9
U 1 1 5F011A6B
P 2650 4800
F 0 "C9" H 2765 4846 50  0000 L CNN
F 1 "10uF" H 2765 4755 50  0000 L CNN
F 2 "esp32-wrover:C_0603_1608Metric" H 2688 4650 50  0001 C CNN
F 3 "~" H 2650 4800 50  0001 C CNN
F 4 "C19702" H 2650 4800 50  0001 C CNN "LCSC"
	1    2650 4800
	1    0    0    -1  
$EndComp
Wire Wire Line
	3050 4600 2650 4600
Wire Wire Line
	2650 4600 2650 4650
Connection ~ 3050 4600
Wire Wire Line
	2650 4950 2650 5000
Wire Wire Line
	2650 5000 3050 5000
Connection ~ 3050 5000
Wire Wire Line
	3050 5000 3050 5050
Wire Wire Line
	3550 650  3550 550 
Connection ~ 3550 550 
Wire Wire Line
	3550 550  2700 550 
$Comp
L edgedriver-rescue:USB_C_Receptacle_USB2.0-Connector J2
U 1 1 5F8C6D1B
P 50 1850
F 0 "J2" H 155 2717 50  0000 C CNN
F 1 "USB_C_Receptacle_USB2.0" H 155 2626 50  0000 C CNN
F 2 "footprints:USB_C_Receptacle_HRO_TYPE-C-31-M-12" H 200 1850 50  0001 C CNN
F 3 "https://www.usb.org/sites/default/files/documents/usb_type-c.zip" H 200 1850 50  0001 C CNN
F 4 "C165948" H 50  1850 50  0001 C CNN "LCSC"
	1    50   1850
	1    0    0    -1  
$EndComp
Wire Wire Line
	650  1850 750  1850
Wire Wire Line
	2100 1950 750  1950
Wire Wire Line
	650  2050 750  2050
Wire Wire Line
	750  2050 750  1950
Connection ~ 750  1950
Wire Wire Line
	750  1950 650  1950
Wire Wire Line
	650  1750 750  1750
Wire Wire Line
	750  1750 750  1850
Connection ~ 750  1850
Wire Wire Line
	750  1850 2100 1850
Wire Wire Line
	650  1250 1700 1250
Wire Wire Line
	1700 1250 1700 1350
Wire Wire Line
	1700 1350 1700 1550
Connection ~ 1700 1350
Wire Wire Line
	1700 1550 1700 2250
Connection ~ 1700 1550
Wire Wire Line
	-250 2750 -250 2850
Wire Wire Line
	-250 2850 -100 2850
Wire Wire Line
	-100 2850 50   2850
Wire Wire Line
	50   2850 50   2750
Connection ~ -100 2850
Connection ~ 4700 1550
Wire Wire Line
	4700 1550 4900 1550
$Comp
L Device:R R8
U 1 1 5F9384F2
P 1000 1450
F 0 "R8" V 900 1450 50  0000 C CNN
F 1 "5.1k" V 1000 1450 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric" V 930 1450 50  0001 C CNN
F 3 "~" H 1000 1450 50  0001 C CNN
F 4 "C25905" H -4050 -100 50  0001 C CNN "LCSC"
	1    1000 1450
	0    1    1    0   
$EndComp
Wire Wire Line
	1150 1450 1300 1450
Wire Wire Line
	650  1450 850  1450
$Comp
L Device:R R9
U 1 1 5F93FCAB
P 1000 1550
F 0 "R9" V 900 1550 50  0000 C CNN
F 1 "5.1k" V 1000 1550 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric" V 930 1550 50  0001 C CNN
F 3 "~" H 1000 1550 50  0001 C CNN
F 4 "C25905" H -4050 0   50  0001 C CNN "LCSC"
	1    1000 1550
	0    1    1    0   
$EndComp
Wire Wire Line
	1150 1550 1300 1550
Wire Wire Line
	650  1550 850  1550
$Comp
L power:GND #PWR01
U 1 1 5F947912
P 1300 1600
F 0 "#PWR01" H 1300 1350 50  0001 C CNN
F 1 "GND" H 1305 1427 50  0000 C CNN
F 2 "" H 1300 1600 50  0001 C CNN
F 3 "" H 1300 1600 50  0001 C CNN
	1    1300 1600
	1    0    0    -1  
$EndComp
Wire Wire Line
	1300 1450 1300 1550
Connection ~ 1300 1550
Wire Wire Line
	1300 1550 1300 1600
$Comp
L Switch:SW_SPST SW1
U 1 1 5FF43269
P 7250 1900
F 0 "SW1" V 7204 1998 50  0000 L CNN
F 1 "SW_SPST" V 7295 1998 50  0000 L CNN
F 2 "Button_Switch_SMD:SW_Push_1P1T_NO_CK_KMR2" H 7250 1900 50  0001 C CNN
F 3 "" H 7250 1900 50  0001 C CNN
F 4 "C318893" H 7250 1900 50  0001 C CNN "LCSC"
	1    7250 1900
	0    1    1    0   
$EndComp
$Comp
L Switch:SW_SPST SW2
U 1 1 5FF43495
P 7950 1900
F 0 "SW2" V 7904 1998 50  0000 L CNN
F 1 "SW_SPST" V 7995 1998 50  0000 L CNN
F 2 "Button_Switch_SMD:SW_Push_1P1T_NO_CK_KMR2" H 7950 1900 50  0001 C CNN
F 3 "" H 7950 1900 50  0001 C CNN
F 4 "C318893" H 7950 1900 50  0001 C CNN "LCSC"
	1    7950 1900
	0    1    1    0   
$EndComp
Text GLabel 7300 1150 2    50   Input ~ 0
ESPEN
$Comp
L power:GND #PWR02
U 1 1 5FF5335F
P 7250 2250
F 0 "#PWR02" H 7250 2000 50  0001 C CNN
F 1 "GND" H 7255 2077 50  0000 C CNN
F 2 "" H 7250 2250 50  0001 C CNN
F 3 "" H 7250 2250 50  0001 C CNN
	1    7250 2250
	1    0    0    -1  
$EndComp
$Comp
L Device:R R10
U 1 1 5FF53437
P 7250 1450
F 0 "R10" V 7150 1450 50  0000 C CNN
F 1 "470 Ohm" V 7250 1450 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric" V 7180 1450 50  0001 C CNN
F 3 "~" H 7250 1450 50  0001 C CNN
F 4 "C25117" H 900 350 50  0001 C CNN "LCSC"
	1    7250 1450
	1    0    0    -1  
$EndComp
$Comp
L Device:R R12
U 1 1 5FF5360B
P 7950 1450
F 0 "R12" V 7850 1450 50  0000 C CNN
F 1 "470 Ohm" V 7950 1450 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric" V 7880 1450 50  0001 C CNN
F 3 "~" H 7950 1450 50  0001 C CNN
F 4 "C25117" H 1600 350 50  0001 C CNN "LCSC"
	1    7950 1450
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR03
U 1 1 5FF5366E
P 7950 2250
F 0 "#PWR03" H 7950 2000 50  0001 C CNN
F 1 "GND" H 7955 2077 50  0000 C CNN
F 2 "" H 7950 2250 50  0001 C CNN
F 3 "" H 7950 2250 50  0001 C CNN
	1    7950 2250
	1    0    0    -1  
$EndComp
Wire Wire Line
	7250 1600 7250 1700
Wire Wire Line
	7950 1600 7950 1700
Wire Wire Line
	7950 2100 7950 2250
Wire Wire Line
	7250 2100 7250 2250
Wire Wire Line
	7300 1150 7250 1150
Wire Wire Line
	7250 1150 7250 1300
Connection ~ 7250 1150
$Comp
L Device:R R11
U 1 1 5FF95363
P 7950 950
F 0 "R11" V 7850 950 50  0000 C CNN
F 1 "10k" V 7950 950 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric" V 7880 950 50  0001 C CNN
F 3 "~" H 7950 950 50  0001 C CNN
F 4 "C25744" H 1600 -150 50  0001 C CNN "LCSC"
	1    7950 950 
	1    0    0    -1  
$EndComp
Text GLabel 8000 750  2    50   Input ~ 0
3V3
Wire Wire Line
	8000 750  7950 750 
Wire Wire Line
	7950 750  7950 800 
Text GLabel 8000 1150 2    50   Input ~ 0
ESPRST
Wire Wire Line
	7950 1100 7950 1150
Wire Wire Line
	8000 1150 7950 1150
Connection ~ 7950 1150
Wire Wire Line
	7950 1150 7950 1300
$Comp
L MCU_ST_STM32F0:STM32F030C8Tx U2
U 1 1 6007B2E1
P 8400 4600
F 0 "U2" H 8400 2914 50  0000 C CNN
F 1 "STM32F030C8Tx" H 8400 2823 50  0000 C CNN
F 2 "Package_QFP:LQFP-48_7x7mm_P0.5mm" H 7900 3100 50  0001 R CNN
F 3 "http://www.st.com/st-web-ui/static/active/en/resource/technical/document/datasheet/DM00088500.pdf" H 8400 4600 50  0001 C CNN
F 4 "C23922" H 8400 4600 50  0001 C CNN "LCSC"
	1    8400 4600
	1    0    0    -1  
$EndComp
Wire Wire Line
	8300 2700 8300 3000
Wire Wire Line
	8400 2700 8400 3000
Wire Wire Line
	8400 2700 8500 2700
Wire Wire Line
	8600 2700 8600 3000
Connection ~ 8600 2700
Wire Wire Line
	8600 2700 8700 2700
Wire Wire Line
	8500 2700 8500 3000
Connection ~ 8500 2700
Wire Wire Line
	8500 2700 8600 2700
Wire Wire Line
	8300 6200 8300 6300
Wire Wire Line
	8300 6300 8400 6300
Connection ~ 8400 6300
Wire Wire Line
	8400 6300 8400 6200
Wire Wire Line
	8400 6300 8500 6300
Wire Wire Line
	8500 6300 8500 6200
$Comp
L Device:C C10
U 1 1 600892EE
P 6900 4500
F 0 "C10" H 7015 4546 50  0000 L CNN
F 1 "100nF" H 7015 4455 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 6938 4350 50  0001 C CNN
F 3 "~" H 6900 4500 50  0001 C CNN
F 4 "C1525" H -550 -450 50  0001 C CNN "LCSC"
	1    6900 4500
	1    0    0    -1  
$EndComp
Wire Wire Line
	7150 4300 6900 4300
Wire Wire Line
	6900 4300 6900 4350
Connection ~ 7150 4300
Wire Wire Line
	6900 4650 6900 4750
Wire Wire Line
	6900 4750 7150 4750
Connection ~ 7150 4750
Connection ~ 2250 6250
Wire Wire Line
	2350 6250 2250 6250
Wire Wire Line
	2250 6650 2250 6700
Wire Wire Line
	2250 6700 1850 6700
Wire Wire Line
	2250 6250 2250 6350
Wire Wire Line
	2150 6250 2250 6250
$Comp
L Device:C C4
U 1 1 5EF97297
P 2250 6500
F 0 "C4" H 2365 6546 50  0000 L CNN
F 1 "10uF" H 2365 6455 50  0000 L CNN
F 2 "esp32-wrover:C_0603_1608Metric" H 2288 6350 50  0001 C CNN
F 3 "~" H 2250 6500 50  0001 C CNN
F 4 "C19702" H 0   0   50  0001 C CNN "LCSC"
	1    2250 6500
	1    0    0    -1  
$EndComp
Text GLabel 2350 6250 2    50   Input ~ 0
3V3
Connection ~ 1450 6250
Wire Wire Line
	1450 6150 1450 6250
$Comp
L power:+5V #PWR0105
U 1 1 5EF97E73
P 1450 6150
F 0 "#PWR0105" H 1450 6000 50  0001 C CNN
F 1 "+5V" H 1465 6323 50  0000 C CNN
F 2 "" H 1450 6150 50  0001 C CNN
F 3 "" H 1450 6150 50  0001 C CNN
	1    1450 6150
	1    0    0    -1  
$EndComp
$Comp
L Regulator_Linear:AP2112K-3.3 U3
U 1 1 62EB0A23
P 1850 6350
F 0 "U3" H 1850 6692 50  0000 C CNN
F 1 "ME6211C33M5G-N" H 1850 6601 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-5" H 1850 6675 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/lcsc/1811131510_MICRONE-Nanjing-Micro-One-Elec-ME6211C33M5G-N_C82942.pdf" H 1850 6450 50  0001 C CNN
F 4 "C82942" H 1850 6350 50  0001 C CNN "LCSC"
	1    1850 6350
	1    0    0    -1  
$EndComp
Wire Wire Line
	1550 6350 1450 6350
Connection ~ 1450 6350
Wire Wire Line
	1850 6650 1850 6700
Text GLabel 7700 3900 0    50   Input ~ 0
3V3
Wire Wire Line
	7700 3900 7800 3900
$Comp
L power:GND #PWR?
U 1 1 64F46701
P 7400 3950
F 0 "#PWR?" H 7400 3700 50  0001 C CNN
F 1 "GND" H 7405 3777 50  0000 C CNN
F 2 "" H 7400 3950 50  0001 C CNN
F 3 "" H 7400 3950 50  0001 C CNN
	1    7400 3950
	1    0    0    -1  
$EndComp
Wire Wire Line
	7400 3950 7400 3800
Wire Wire Line
	7400 3800 7800 3800
$EndSCHEMATC
