EESchema Schematic File Version 2
LIBS:device
LIBS:conn
LIBS:interface
LIBS:74xx
LIBS:switches
LIBS:CA6
LIBS:CA6Test-cache
EELAYER 25 0
EELAYER END
$Descr A2 23386 16535
encoding utf-8
Sheet 1 1
Title "ESP9266 Comm Daughterboard"
Date ""
Rev ""
Comp "Dreaming Robots"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L IDC_50PIN(2X25) J6
U 1 1 53682209
P 20300 2300
F 0 "J6" H 20300 3650 50  0000 C CNN
F 1 "AUX" H 20300 950 50  0000 C CNN
F 2 "CA6:IDC_50PIN(2X25)-90" H 20300 2300 50  0001 C CNN
F 3 "DOCUMENTATION" H 20300 2300 50  0001 C CNN
	1    20300 2300
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR9
U 1 1 53682C14
P 19450 3550
F 0 "#PWR9" H 19450 3550 30  0001 C CNN
F 1 "GND" H 19450 3480 30  0001 C CNN
F 2 "" H 19450 3550 60  0000 C CNN
F 3 "" H 19450 3550 60  0000 C CNN
	1    19450 3550
	1    0    0    -1  
$EndComp
$Comp
L JACK_RJ45 J5
U 1 1 536A1056
P 20950 6700
F 0 "J5" H 21125 7200 39  0000 C CNN
F 1 "MODULE_1" H 20775 7200 39  0000 C CNN
F 2 "CA6:RJ45_pcbwing" H 20925 6700 60  0001 C CNN
F 3 "~" H 20925 6700 60  0000 C CNN
	1    20950 6700
	0    1    1    0   
$EndComp
$Comp
L JACK_RJ45 J3
U 1 1 536A1078
P 18200 6700
F 0 "J3" H 18375 7200 39  0000 C CNN
F 1 "MODULE_0" H 18025 7200 39  0000 C CNN
F 2 "CA6:RJ45_pcbwing" H 18175 6700 60  0001 C CNN
F 3 "~" H 18175 6700 60  0000 C CNN
	1    18200 6700
	0    -1   1    0   
$EndComp
$Comp
L HOLE X1
U 1 1 53720DAD
P 16500 1250
F 0 "X1" H 16500 1150 60  0000 C CNN
F 1 "HOLE" H 16500 1350 60  0000 C CNN
F 2 "CA6:HOLE_3.0mm" H 16500 1250 60  0001 C CNN
F 3 "~" H 16500 1250 60  0000 C CNN
	1    16500 1250
	1    0    0    -1  
$EndComp
$Comp
L HOLE X2
U 1 1 53720DBA
P 16800 1250
F 0 "X2" H 16800 1150 60  0000 C CNN
F 1 "HOLE" H 16800 1350 60  0000 C CNN
F 2 "CA6:HOLE_3.0mm" H 16800 1250 60  0001 C CNN
F 3 "~" H 16800 1250 60  0000 C CNN
	1    16800 1250
	1    0    0    -1  
$EndComp
$Comp
L HOLE X3
U 1 1 53720DC0
P 17100 1250
F 0 "X3" H 17100 1150 60  0000 C CNN
F 1 "HOLE" H 17100 1350 60  0000 C CNN
F 2 "CA6:HOLE_3.0mm" H 17100 1250 60  0001 C CNN
F 3 "~" H 17100 1250 60  0000 C CNN
	1    17100 1250
	1    0    0    -1  
$EndComp
$Comp
L JACK_RJ45 J4
U 1 1 5377EFFB
P 18200 7650
F 0 "J4" H 18375 8150 39  0000 C CNN
F 1 "MODULE_2" H 18025 8150 39  0000 C CNN
F 2 "CA6:RJ45_pcbwing" H 18175 7650 60  0001 C CNN
F 3 "~" H 18175 7650 60  0000 C CNN
	1    18200 7650
	0    -1   1    0   
$EndComp
$Comp
L JACK_RJ45 J8
U 1 1 537AA6D0
P 20950 7650
F 0 "J8" H 21125 8150 39  0000 C CNN
F 1 "MODULE_3" H 20775 8150 39  0000 C CNN
F 2 "CA6:RJ45_pcbwing" H 20925 7650 60  0001 C CNN
F 3 "~" H 20925 7650 60  0000 C CNN
	1    20950 7650
	0    1    1    0   
$EndComp
$Comp
L JACK_STEREO CAM0
U 1 1 5381C2F5
P 15000 1300
F 0 "CAM0" H 14625 1300 50  0000 R CNN
F 1 "CAM/FLASH" H 15000 1550 50  0000 C CNN
F 2 "CA6:161-3334-E_Big_Pad" H 15000 1300 60  0001 C CNN
F 3 "~" H 15000 1300 60  0000 C CNN
	1    15000 1300
	-1   0    0    -1  
$EndComp
$Comp
L CA_Logo_150 LOGO1
U 1 1 541ED919
P 18100 1050
F 0 "LOGO1" H 18100 777 60  0001 C CNN
F 1 "CA_LOGO_150" H 18100 1323 60  0001 C CNN
F 2 "CA6:CA_Logo_15mm" H 18100 1050 60  0001 C CNN
F 3 "" H 18100 1050 60  0000 C CNN
	1    18100 1050
	1    0    0    -1  
$EndComp
$Comp
L JACK_STEREO CAM1
U 1 1 55973B31
P 15000 2350
F 0 "CAM1" H 14625 2350 50  0000 R CNN
F 1 "CAM/FLASH" H 15000 2600 50  0000 C CNN
F 2 "CA6:161-3334-E_Big_Pad" H 15000 2350 60  0001 C CNN
F 3 "~" H 15000 2350 60  0000 C CNN
	1    15000 2350
	-1   0    0    -1  
$EndComp
$Comp
L JACK_STEREO CAM2
U 1 1 55973B62
P 15000 3400
F 0 "CAM2" H 14625 3400 50  0000 R CNN
F 1 "CAM/FLASH" H 15000 3650 50  0000 C CNN
F 2 "CA6:161-3334-E_Big_Pad" H 15000 3400 60  0001 C CNN
F 3 "~" H 15000 3400 60  0000 C CNN
	1    15000 3400
	-1   0    0    -1  
$EndComp
$Comp
L JACK_STEREO CAM3
U 1 1 55973B93
P 15000 4450
F 0 "CAM3" H 14625 4450 50  0000 R CNN
F 1 "CAM/FLASH" H 15000 4700 50  0000 C CNN
F 2 "CA6:161-3334-E_Big_Pad" H 15000 4450 60  0001 C CNN
F 3 "~" H 15000 4450 60  0000 C CNN
	1    15000 4450
	-1   0    0    -1  
$EndComp
$Comp
L JACK_STEREO CAM4
U 1 1 55973BC4
P 15000 5500
F 0 "CAM4" H 14625 5500 50  0000 R CNN
F 1 "CAM/FLASH" H 15000 5750 50  0000 C CNN
F 2 "CA6:161-3334-E_Big_Pad" H 15000 5500 60  0001 C CNN
F 3 "~" H 15000 5500 60  0000 C CNN
	1    15000 5500
	-1   0    0    -1  
$EndComp
$Comp
L JACK_STEREO CAM5
U 1 1 55973BF5
P 15000 6550
F 0 "CAM5" H 14625 6550 50  0000 R CNN
F 1 "CAM/FLASH" H 15000 6800 50  0000 C CNN
F 2 "CA6:161-3334-E_Big_Pad" H 15000 6550 60  0001 C CNN
F 3 "~" H 15000 6550 60  0000 C CNN
	1    15000 6550
	-1   0    0    -1  
$EndComp
$Comp
L JACK_STEREO CAM6
U 1 1 55973C26
P 15000 7600
F 0 "CAM6" H 14625 7600 50  0000 R CNN
F 1 "CAM/FLASH" H 15000 7850 50  0000 C CNN
F 2 "CA6:161-3334-E_Big_Pad" H 15000 7600 60  0001 C CNN
F 3 "~" H 15000 7600 60  0000 C CNN
	1    15000 7600
	-1   0    0    -1  
$EndComp
$Comp
L JACK_STEREO CAM7
U 1 1 55973C57
P 15000 8650
F 0 "CAM7" H 14625 8650 50  0000 R CNN
F 1 "CAM/FLASH" H 15000 8900 50  0000 C CNN
F 2 "CA6:161-3334-E_Big_Pad" H 15000 8650 60  0001 C CNN
F 3 "~" H 15000 8650 60  0000 C CNN
	1    15000 8650
	-1   0    0    -1  
$EndComp
$Comp
L HOLE X4
U 1 1 58C15241
P 17400 1250
F 0 "X4" H 17400 1150 60  0000 C CNN
F 1 "HOLE" H 17400 1350 60  0000 C CNN
F 2 "CA6:HOLE_3.0mm" H 17400 1250 60  0001 C CNN
F 3 "~" H 17400 1250 60  0000 C CNN
	1    17400 1250
	1    0    0    -1  
$EndComp
Wire Wire Line
	19550 1100 19550 1050
Wire Wire Line
	19550 1050 21050 1050
Wire Wire Line
	21050 1050 21050 1100
Wire Wire Line
	19550 1200 19550 1150
Wire Wire Line
	19550 1150 21050 1150
Wire Wire Line
	21050 1150 21050 1200
Wire Wire Line
	19550 1300 19550 1250
Wire Wire Line
	19550 1250 21050 1250
Wire Wire Line
	21050 1250 21050 1300
Wire Wire Line
	19550 1400 19550 1350
Wire Wire Line
	19550 1350 21050 1350
Wire Wire Line
	21050 1350 21050 1400
Wire Wire Line
	21050 1500 21050 1450
Wire Wire Line
	21050 1450 19550 1450
Wire Wire Line
	19550 1450 19550 1500
Wire Wire Line
	19550 1600 19550 1550
Wire Wire Line
	19550 1550 21050 1550
Wire Wire Line
	21050 1550 21050 1600
Wire Wire Line
	21050 1700 21050 1650
Wire Wire Line
	21050 1650 19550 1650
Wire Wire Line
	19550 1650 19550 1700
Wire Wire Line
	19550 1800 19550 1750
Wire Wire Line
	19550 1750 21050 1750
Wire Wire Line
	21050 1750 21050 1800
Wire Wire Line
	21050 1900 21050 1850
Wire Wire Line
	21050 1850 19550 1850
Wire Wire Line
	19550 1850 19550 1900
Wire Wire Line
	19550 2000 19550 1950
Wire Wire Line
	19550 1950 21050 1950
Wire Wire Line
	21050 1950 21050 2000
Wire Wire Line
	21050 2100 21050 2050
Wire Wire Line
	21050 2050 19550 2050
Wire Wire Line
	19550 2050 19550 2100
Wire Wire Line
	19550 2200 19550 2150
Wire Wire Line
	19550 2150 21050 2150
Wire Wire Line
	21050 2150 21050 2200
Wire Wire Line
	21050 2300 21050 2250
Wire Wire Line
	21050 2250 19550 2250
Wire Wire Line
	19550 2250 19550 2300
Wire Wire Line
	19550 2400 19550 2350
Wire Wire Line
	19550 2350 21050 2350
Wire Wire Line
	21050 2350 21050 2400
Wire Wire Line
	21050 2500 21050 2450
Wire Wire Line
	21050 2450 19550 2450
Wire Wire Line
	19550 2450 19550 2500
Wire Wire Line
	19550 2600 19550 2550
Wire Wire Line
	19550 2550 21050 2550
Wire Wire Line
	21050 2550 21050 2600
Wire Wire Line
	21050 2700 21050 2650
Wire Wire Line
	21050 2650 19550 2650
Wire Wire Line
	19550 2650 19550 2700
Wire Wire Line
	19550 2800 19550 2750
Wire Wire Line
	19550 2750 21050 2750
Wire Wire Line
	21050 2750 21050 2800
Wire Wire Line
	21050 2900 21050 2850
Wire Wire Line
	21050 2850 19550 2850
Wire Wire Line
	19550 2850 19550 2900
Wire Wire Line
	19550 3000 19550 2950
Wire Wire Line
	19550 2950 21050 2950
Wire Wire Line
	21050 2950 21050 3000
Wire Wire Line
	21050 3100 21050 3050
Wire Wire Line
	21050 3050 19550 3050
Wire Wire Line
	19550 3050 19550 3100
Wire Wire Line
	19550 3200 19550 3150
Wire Wire Line
	19550 3150 21050 3150
Wire Wire Line
	21050 3150 21050 3200
Wire Wire Line
	21050 3300 21050 3250
Wire Wire Line
	21050 3250 19550 3250
Wire Wire Line
	19550 3250 19550 3300
Wire Wire Line
	19550 3400 19400 3400
Wire Wire Line
	21050 3400 21200 3400
Text Label 21200 3400 0    60   ~ 0
FOCUS
Text Label 19400 3400 2    60   ~ 0
SHUTTER
Wire Wire Line
	19550 3500 19450 3500
Wire Wire Line
	19450 3500 19450 3550
$Comp
L GND #PWR1
U 1 1 5B2032F7
P 14550 1500
F 0 "#PWR1" H 14550 1500 30  0001 C CNN
F 1 "GND" H 14550 1430 30  0001 C CNN
F 2 "" H 14550 1500 60  0000 C CNN
F 3 "" H 14550 1500 60  0000 C CNN
	1    14550 1500
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR2
U 1 1 5B203676
P 14550 2550
F 0 "#PWR2" H 14550 2550 30  0001 C CNN
F 1 "GND" H 14550 2480 30  0001 C CNN
F 2 "" H 14550 2550 60  0000 C CNN
F 3 "" H 14550 2550 60  0000 C CNN
	1    14550 2550
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR3
U 1 1 5B20374D
P 14550 3600
F 0 "#PWR3" H 14550 3600 30  0001 C CNN
F 1 "GND" H 14550 3530 30  0001 C CNN
F 2 "" H 14550 3600 60  0000 C CNN
F 3 "" H 14550 3600 60  0000 C CNN
	1    14550 3600
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR4
U 1 1 5B20378B
P 14550 4650
F 0 "#PWR4" H 14550 4650 30  0001 C CNN
F 1 "GND" H 14550 4580 30  0001 C CNN
F 2 "" H 14550 4650 60  0000 C CNN
F 3 "" H 14550 4650 60  0000 C CNN
	1    14550 4650
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR5
U 1 1 5B203884
P 14550 5700
F 0 "#PWR5" H 14550 5700 30  0001 C CNN
F 1 "GND" H 14550 5630 30  0001 C CNN
F 2 "" H 14550 5700 60  0000 C CNN
F 3 "" H 14550 5700 60  0000 C CNN
	1    14550 5700
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR6
U 1 1 5B2038C2
P 14550 6750
F 0 "#PWR6" H 14550 6750 30  0001 C CNN
F 1 "GND" H 14550 6680 30  0001 C CNN
F 2 "" H 14550 6750 60  0000 C CNN
F 3 "" H 14550 6750 60  0000 C CNN
	1    14550 6750
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR7
U 1 1 5B203A32
P 14550 7800
F 0 "#PWR7" H 14550 7800 30  0001 C CNN
F 1 "GND" H 14550 7730 30  0001 C CNN
F 2 "" H 14550 7800 60  0000 C CNN
F 3 "" H 14550 7800 60  0000 C CNN
	1    14550 7800
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR8
U 1 1 5B203A70
P 14550 8850
F 0 "#PWR8" H 14550 8850 30  0001 C CNN
F 1 "GND" H 14550 8780 30  0001 C CNN
F 2 "" H 14550 8850 60  0000 C CNN
F 3 "" H 14550 8850 60  0000 C CNN
	1    14550 8850
	1    0    0    -1  
$EndComp
Wire Wire Line
	14650 1450 14550 1450
Wire Wire Line
	14550 1450 14550 1500
Wire Wire Line
	14650 2500 14550 2500
Wire Wire Line
	14550 2500 14550 2550
Wire Wire Line
	14650 3550 14550 3550
Wire Wire Line
	14550 3550 14550 3600
Wire Wire Line
	14650 4600 14550 4600
Wire Wire Line
	14550 4600 14550 4650
Wire Wire Line
	14650 5650 14550 5650
Wire Wire Line
	14550 5650 14550 5700
Wire Wire Line
	14650 6700 14550 6700
Wire Wire Line
	14550 6700 14550 6750
Wire Wire Line
	14650 7750 14550 7750
Wire Wire Line
	14550 7750 14550 7800
Wire Wire Line
	14650 8800 14550 8800
Wire Wire Line
	14550 8800 14550 8850
Wire Wire Line
	14650 1150 14550 1150
Wire Wire Line
	14650 1350 14550 1350
Wire Wire Line
	14650 2200 14550 2200
Wire Wire Line
	14650 2400 14550 2400
Text Label 14550 1150 2    60   ~ 0
SHUTTER
Text Label 14550 2200 2    60   ~ 0
SHUTTER
Wire Wire Line
	14650 3250 14550 3250
Wire Wire Line
	14650 3450 14550 3450
Wire Wire Line
	14650 4300 14550 4300
Wire Wire Line
	14650 4500 14550 4500
Wire Wire Line
	14650 6400 14550 6400
Wire Wire Line
	14650 6600 14550 6600
Wire Wire Line
	14650 7450 14550 7450
Wire Wire Line
	14650 7650 14550 7650
Wire Wire Line
	14650 8500 14550 8500
Wire Wire Line
	14650 8700 14550 8700
Text Label 14550 1350 2    60   ~ 0
FOCUS
Text Label 14550 3250 2    60   ~ 0
SHUTTER
Text Label 14550 4300 2    60   ~ 0
SHUTTER
Wire Wire Line
	14650 5350 14550 5350
Wire Wire Line
	14650 5550 14550 5550
Text Label 14550 5350 2    60   ~ 0
SHUTTER
Text Label 14550 6400 2    60   ~ 0
SHUTTER
Text Label 14550 7450 2    60   ~ 0
SHUTTER
Text Label 14550 8500 2    60   ~ 0
SHUTTER
Text Label 14550 2400 2    60   ~ 0
FOCUS
Text Label 14550 3450 2    60   ~ 0
FOCUS
Text Label 14550 4500 2    60   ~ 0
FOCUS
Text Label 14550 5550 2    60   ~ 0
FOCUS
Text Label 14550 6600 2    60   ~ 0
FOCUS
Text Label 14550 7650 2    60   ~ 0
FOCUS
Text Label 14550 8700 2    60   ~ 0
FOCUS
$Comp
L R R5
U 1 1 5B205F7A
P 20700 4000
F 0 "R5" V 20780 4000 50  0000 C CNN
F 1 "470" V 20700 4000 50  0000 C CNN
F 2 "Resistors_ThroughHole:R_Axial_DIN0309_L9.0mm_D3.2mm_P12.70mm_Horizontal" V 20630 4000 50  0001 C CNN
F 3 "" H 20700 4000 50  0001 C CNN
	1    20700 4000
	0    1    1    0   
$EndComp
$Comp
L LED D3
U 1 1 5B2067BD
P 20300 4000
F 0 "D3" H 20300 4100 50  0000 C CNN
F 1 "LED" H 20300 3900 50  0000 C CNN
F 2 "LEDs:LED_D5.0mm" H 20300 4000 50  0001 C CNN
F 3 "" H 20300 4000 50  0001 C CNN
	1    20300 4000
	1    0    0    -1  
$EndComp
$Comp
L R R1
U 1 1 5B206C73
P 19150 6500
F 0 "R1" V 19230 6500 50  0000 C CNN
F 1 "470" V 19150 6500 50  0000 C CNN
F 2 "Resistors_ThroughHole:R_Axial_DIN0309_L9.0mm_D3.2mm_P12.70mm_Horizontal" V 19080 6500 50  0001 C CNN
F 3 "" H 19150 6500 50  0001 C CNN
	1    19150 6500
	-1   0    0    1   
$EndComp
$Comp
L LED D1
U 1 1 5B206D1B
P 19150 6900
F 0 "D1" H 19150 7000 50  0000 C CNN
F 1 "LED" H 19150 6800 50  0000 C CNN
F 2 "LEDs:LED_D5.0mm" H 19150 6900 50  0001 C CNN
F 3 "" H 19150 6900 50  0001 C CNN
	1    19150 6900
	0    -1   -1   0   
$EndComp
$Comp
L R R3
U 1 1 5B206F7C
P 20000 6500
F 0 "R3" V 20080 6500 50  0000 C CNN
F 1 "470" V 20000 6500 50  0000 C CNN
F 2 "Resistors_ThroughHole:R_Axial_DIN0309_L9.0mm_D3.2mm_P12.70mm_Horizontal" V 19930 6500 50  0001 C CNN
F 3 "" H 20000 6500 50  0001 C CNN
	1    20000 6500
	-1   0    0    1   
$EndComp
$Comp
L LED D4
U 1 1 5B207028
P 20000 6900
F 0 "D4" H 20000 7000 50  0000 C CNN
F 1 "LED" H 20000 6800 50  0000 C CNN
F 2 "LEDs:LED_D5.0mm" H 20000 6900 50  0001 C CNN
F 3 "" H 20000 6900 50  0001 C CNN
	1    20000 6900
	0    -1   -1   0   
$EndComp
$Comp
L LED D2
U 1 1 5B2070EF
P 19150 7850
F 0 "D2" H 19150 7950 50  0000 C CNN
F 1 "LED" H 19150 7750 50  0000 C CNN
F 2 "LEDs:LED_D5.0mm" H 19150 7850 50  0001 C CNN
F 3 "" H 19150 7850 50  0001 C CNN
	1    19150 7850
	0    -1   -1   0   
$EndComp
$Comp
L LED D5
U 1 1 5B207175
P 20000 7850
F 0 "D5" H 20000 7950 50  0000 C CNN
F 1 "LED" H 20000 7750 50  0000 C CNN
F 2 "LEDs:LED_D5.0mm" H 20000 7850 50  0001 C CNN
F 3 "" H 20000 7850 50  0001 C CNN
	1    20000 7850
	0    -1   -1   0   
$EndComp
$Comp
L R R4
U 1 1 5B20721C
P 20000 7450
F 0 "R4" V 20080 7450 50  0000 C CNN
F 1 "470" V 20000 7450 50  0000 C CNN
F 2 "Resistors_ThroughHole:R_Axial_DIN0309_L9.0mm_D3.2mm_P12.70mm_Horizontal" V 19930 7450 50  0001 C CNN
F 3 "" H 20000 7450 50  0001 C CNN
	1    20000 7450
	-1   0    0    1   
$EndComp
$Comp
L R R2
U 1 1 5B20729C
P 19150 7450
F 0 "R2" V 19230 7450 50  0000 C CNN
F 1 "470" V 19150 7450 50  0000 C CNN
F 2 "Resistors_ThroughHole:R_Axial_DIN0309_L9.0mm_D3.2mm_P12.70mm_Horizontal" V 19080 7450 50  0001 C CNN
F 3 "" H 19150 7450 50  0001 C CNN
	1    19150 7450
	-1   0    0    1   
$EndComp
Wire Wire Line
	21050 3500 21050 4000
Wire Wire Line
	21050 4000 20850 4000
Wire Wire Line
	20550 4000 20450 4000
$Comp
L GND #PWR10
U 1 1 5B207A7E
P 20050 4050
F 0 "#PWR10" H 20050 4050 30  0001 C CNN
F 1 "GND" H 20050 3980 30  0001 C CNN
F 2 "" H 20050 4050 60  0000 C CNN
F 3 "" H 20050 4050 60  0000 C CNN
	1    20050 4050
	1    0    0    -1  
$EndComp
Wire Wire Line
	20150 4000 20050 4000
Wire Wire Line
	20050 4000 20050 4050
Wire Wire Line
	18650 6350 19150 6350
Wire Wire Line
	19150 6650 19150 6750
Wire Wire Line
	19150 7050 18650 7050
Wire Wire Line
	20500 6350 20000 6350
Wire Wire Line
	20000 6650 20000 6750
Wire Wire Line
	20000 7050 20500 7050
Wire Wire Line
	20500 7300 20000 7300
Wire Wire Line
	20000 7600 20000 7700
Wire Wire Line
	20000 8000 20500 8000
Wire Wire Line
	18650 7300 19150 7300
Wire Wire Line
	19150 7600 19150 7700
Wire Wire Line
	19150 8000 18650 8000
Wire Wire Line
	18650 6450 18700 6450
Wire Wire Line
	18700 6450 18700 6550
Wire Wire Line
	18700 6550 18650 6550
Wire Wire Line
	18650 6650 18700 6650
Wire Wire Line
	18700 6650 18700 6750
Wire Wire Line
	18700 6750 18650 6750
Wire Wire Line
	18650 6850 18700 6850
Wire Wire Line
	18700 6850 18700 6950
Wire Wire Line
	18700 6950 18650 6950
Wire Wire Line
	20500 6450 20450 6450
Wire Wire Line
	20450 6450 20450 6550
Wire Wire Line
	20450 6550 20500 6550
Wire Wire Line
	20500 6650 20450 6650
Wire Wire Line
	20450 6650 20450 6750
Wire Wire Line
	20450 6750 20500 6750
Wire Wire Line
	20500 6850 20450 6850
Wire Wire Line
	20450 6850 20450 6950
Wire Wire Line
	20450 6950 20500 6950
Wire Wire Line
	20500 7400 20450 7400
Wire Wire Line
	20450 7400 20450 7500
Wire Wire Line
	20450 7500 20500 7500
Wire Wire Line
	20500 7600 20450 7600
Wire Wire Line
	20450 7600 20450 7700
Wire Wire Line
	20450 7700 20500 7700
Wire Wire Line
	20500 7800 20450 7800
Wire Wire Line
	20450 7800 20450 7900
Wire Wire Line
	20450 7900 20500 7900
Wire Wire Line
	18650 7400 18700 7400
Wire Wire Line
	18700 7400 18700 7500
Wire Wire Line
	18700 7500 18650 7500
Wire Wire Line
	18650 7600 18700 7600
Wire Wire Line
	18700 7600 18700 7700
Wire Wire Line
	18700 7700 18650 7700
Wire Wire Line
	18650 7800 18700 7800
Wire Wire Line
	18700 7800 18700 7900
Wire Wire Line
	18700 7900 18650 7900
$EndSCHEMATC
