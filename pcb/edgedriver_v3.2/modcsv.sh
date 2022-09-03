#!/bin/bash

f='./edgedriver.csv'

while read regex; do sed -i "$regex" "$f"; done <<EOF
s:Comment,Designator,Footprint,LCSC, LCSC:Comment,Designator,Footprint,LCSC Part Number:
s:"",::
s/esp32-wrover:C_0603_1608Metric/0603/
s/Capacitor_SMD:C_0402_1005Metric/0402/
s/footprints:USB_C_Receptacle_HRO_TYPE-C-31-M-12/TYPE-C-31_L8.94_W7.3/
s/Package_TO_SOT_SMD:SOT-23/SOT-23-3L/
s/Resistor_SMD:R_0402_1005Metric/0402/
s/Button_Switch_SMD:SW_Push_1P1T_NO_CK_KMR2/SW-SMD-4_3.0x3.65x1.5P/
s/Package_QFP:LQFP-48_7x7mm_P0.5mm/LQFP-48_7.0x7.0x0.5P/
s/Package_TO_SOT_SMD:SOT-23-5/SOT-23-5/
s/Package_DFN_QFN:QFN-24-1EP_4x4mm_P0.5mm_EP2.6x2.6mm/QFN-24_L4.0-W4.0-P0.50EP2.8/
/"J1"/d
/"J3"/d
/"J4"/d
/"U1"/d
EOF

