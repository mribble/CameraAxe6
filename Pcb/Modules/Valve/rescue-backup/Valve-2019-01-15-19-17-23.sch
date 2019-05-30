EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:CA6
LIBS:Valve-cache
EELAYER 25 0
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
L D D1
U 1 1 59C151BB
P 3300 1150
F 0 "D1" H 3300 1250 50  0000 C CNN
F 1 "D" H 3300 1050 50  0001 C CNN
F 2 "CA6:D_219AB" H 3300 1150 50  0001 C CNN
F 3 "" H 3300 1150 50  0000 C CNN
	1    3300 1150
	0    1    1    0   
$EndComp
$Comp
L Q_NMOS_GSD Q1
U 1 1 59C152E2
P 3400 1550
F 0 "Q1" H 3600 1600 50  0000 L CNN
F 1 "Q_NMOS_GSD" H 3600 1500 50  0001 L CNN
F 2 "TO_SOT_Packages_SMD:SOT-23" H 3600 1650 50  0001 C CNN
F 3 "" H 3400 1550 50  0000 C CNN
	1    3400 1550
	1    0    0    -1  
$EndComp
$Comp
L BARREL_JACK Valve1
U 1 1 59C1662C
P 3950 1150
F 0 "Valve1" H 3950 1400 50  0000 C CNN
F 1 "Valve1" H 3950 950 50  0000 C CNN
F 2 "Connectors:BARREL_JACK" H 3950 1150 50  0001 C CNN
F 3 "" H 3950 1150 50  0000 C CNN
	1    3950 1150
	-1   0    0    -1  
$EndComp
$Comp
L JACK_RJ45 J1
U 1 1 59C16716
P 1950 3950
F 0 "J1" H 2125 4450 39  0000 C CNN
F 1 "JACK_RJ45" H 1775 4450 39  0001 C CNN
F 2 "CA6:RJ45_pcbwing" H 1925 3950 60  0001 C CNN
F 3 "" H 1925 3950 60  0000 C CNN
	1    1950 3950
	-1   0    0    1   
$EndComp
$Comp
L CONN_01X03 Servo1
U 1 1 59C16AB4
P 1200 1150
F 0 "Servo1" H 1200 1350 50  0000 C CNN
F 1 "Servo1" V 1300 1150 50  0000 C CNN
F 2 "Socket_Strips:Socket_Strip_Angled_1x03_Pitch2.54mm" H 1200 1150 50  0001 C CNN
F 3 "" H 1200 1150 50  0000 C CNN
	1    1200 1150
	-1   0    0    -1  
$EndComp
$Comp
L GND #PWR01
U 1 1 59C17A3F
P 1100 3450
F 0 "#PWR01" H 1100 3200 50  0001 C CNN
F 1 "GND" H 1100 3300 50  0000 C CNN
F 2 "" H 1100 3450 50  0000 C CNN
F 3 "" H 1100 3450 50  0000 C CNN
	1    1100 3450
	0    1    1    0   
$EndComp
Wire Wire Line
	3650 1250 3500 1250
Wire Wire Line
	3500 1250 3500 1350
Wire Wire Line
	3500 1050 3650 1050
Wire Wire Line
	3500 800  3500 1050
$Comp
L +12V #PWR02
U 1 1 59C18400
P 3350 800
F 0 "#PWR02" H 3350 650 50  0001 C CNN
F 1 "+12V" H 3350 940 50  0000 C CNN
F 2 "" H 3350 800 50  0000 C CNN
F 3 "" H 3350 800 50  0000 C CNN
	1    3350 800 
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3300 1000 3500 1000
Connection ~ 3500 1000
Wire Wire Line
	3300 1300 3500 1300
Connection ~ 3500 1300
Wire Wire Line
	3500 800  3350 800 
Wire Wire Line
	3500 1750 3500 1800
$Comp
L GND #PWR03
U 1 1 59C188AE
P 3500 1800
F 0 "#PWR03" H 3500 1550 50  0001 C CNN
F 1 "GND" H 3500 1650 50  0000 C CNN
F 2 "" H 3500 1800 50  0000 C CNN
F 3 "" H 3500 1800 50  0000 C CNN
	1    3500 1800
	1    0    0    -1  
$EndComp
Wire Wire Line
	1600 3450 1600 3500
Wire Wire Line
	1100 3450 1600 3450
Wire Wire Line
	2200 3500 2200 3250
Wire Wire Line
	2100 3500 2100 3250
Text Label 2200 3250 1    60   ~ 0
Valve1
Wire Wire Line
	2000 3500 2000 3250
Wire Wire Line
	1900 3500 1900 3250
Wire Wire Line
	1800 3500 1800 3250
Wire Wire Line
	1700 3500 1700 3250
Text Label 2100 3250 1    60   ~ 0
Valve2
Text Label 2000 3250 1    60   ~ 0
Valve3
Text Label 1900 3250 1    60   ~ 0
Valve4
Text Label 1800 3250 1    60   ~ 0
Servo1
Text Label 1700 3250 1    60   ~ 0
Servo2
$Comp
L D D3
U 1 1 59C192A1
P 4950 1150
F 0 "D3" H 4950 1250 50  0000 C CNN
F 1 "D" H 4950 1050 50  0001 C CNN
F 2 "CA6:D_219AB" H 4950 1150 50  0001 C CNN
F 3 "" H 4950 1150 50  0000 C CNN
	1    4950 1150
	0    1    1    0   
$EndComp
$Comp
L Q_NMOS_GSD Q2
U 1 1 59C192A7
P 5050 1550
F 0 "Q2" H 5250 1600 50  0000 L CNN
F 1 "Q_NMOS_GSD" H 5250 1500 50  0001 L CNN
F 2 "TO_SOT_Packages_SMD:SOT-23" H 5250 1650 50  0001 C CNN
F 3 "" H 5050 1550 50  0000 C CNN
	1    5050 1550
	1    0    0    -1  
$EndComp
$Comp
L BARREL_JACK Valve2
U 1 1 59C192AD
P 5600 1150
F 0 "Valve2" H 5600 1400 50  0000 C CNN
F 1 "Valve2" H 5600 950 50  0000 C CNN
F 2 "Connectors:BARREL_JACK" H 5600 1150 50  0001 C CNN
F 3 "" H 5600 1150 50  0000 C CNN
	1    5600 1150
	-1   0    0    -1  
$EndComp
Wire Wire Line
	5300 1250 5150 1250
Wire Wire Line
	5150 1250 5150 1350
Wire Wire Line
	5150 1050 5300 1050
Wire Wire Line
	5150 800  5150 1050
$Comp
L +12V #PWR04
U 1 1 59C192BA
P 5000 800
F 0 "#PWR04" H 5000 650 50  0001 C CNN
F 1 "+12V" H 5000 940 50  0000 C CNN
F 2 "" H 5000 800 50  0000 C CNN
F 3 "" H 5000 800 50  0000 C CNN
	1    5000 800 
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4950 1000 5150 1000
Connection ~ 5150 1000
Wire Wire Line
	4950 1300 5150 1300
Connection ~ 5150 1300
Wire Wire Line
	5150 800  5000 800 
Wire Wire Line
	5150 1750 5150 1800
$Comp
L GND #PWR05
U 1 1 59C192C6
P 5150 1800
F 0 "#PWR05" H 5150 1550 50  0001 C CNN
F 1 "GND" H 5150 1650 50  0000 C CNN
F 2 "" H 5150 1800 50  0000 C CNN
F 3 "" H 5150 1800 50  0000 C CNN
	1    5150 1800
	1    0    0    -1  
$EndComp
$Comp
L D D2
U 1 1 59C195FE
P 3300 2600
F 0 "D2" H 3300 2700 50  0000 C CNN
F 1 "D" H 3300 2500 50  0001 C CNN
F 2 "CA6:D_219AB" H 3300 2600 50  0001 C CNN
F 3 "" H 3300 2600 50  0000 C CNN
	1    3300 2600
	0    1    1    0   
$EndComp
$Comp
L Q_NMOS_GSD Q3
U 1 1 59C19604
P 3400 3000
F 0 "Q3" H 3600 3050 50  0000 L CNN
F 1 "Q_NMOS_GSD" H 3600 2950 50  0001 L CNN
F 2 "TO_SOT_Packages_SMD:SOT-23" H 3600 3100 50  0001 C CNN
F 3 "" H 3400 3000 50  0000 C CNN
	1    3400 3000
	1    0    0    -1  
$EndComp
$Comp
L BARREL_JACK Valve3
U 1 1 59C1960A
P 3950 2600
F 0 "Valve3" H 3950 2850 50  0000 C CNN
F 1 "Valve3" H 3950 2400 50  0000 C CNN
F 2 "Connectors:BARREL_JACK" H 3950 2600 50  0001 C CNN
F 3 "" H 3950 2600 50  0000 C CNN
	1    3950 2600
	-1   0    0    -1  
$EndComp
Wire Wire Line
	3650 2700 3500 2700
Wire Wire Line
	3500 2700 3500 2800
Wire Wire Line
	3500 2500 3650 2500
Wire Wire Line
	3500 2250 3500 2500
$Comp
L +12V #PWR06
U 1 1 59C19617
P 3350 2250
F 0 "#PWR06" H 3350 2100 50  0001 C CNN
F 1 "+12V" H 3350 2390 50  0000 C CNN
F 2 "" H 3350 2250 50  0000 C CNN
F 3 "" H 3350 2250 50  0000 C CNN
	1    3350 2250
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3300 2450 3500 2450
Connection ~ 3500 2450
Wire Wire Line
	3300 2750 3500 2750
Connection ~ 3500 2750
Wire Wire Line
	3500 2250 3350 2250
Wire Wire Line
	3500 3200 3500 3250
$Comp
L GND #PWR07
U 1 1 59C19623
P 3500 3250
F 0 "#PWR07" H 3500 3000 50  0001 C CNN
F 1 "GND" H 3500 3100 50  0000 C CNN
F 2 "" H 3500 3250 50  0000 C CNN
F 3 "" H 3500 3250 50  0000 C CNN
	1    3500 3250
	1    0    0    -1  
$EndComp
$Comp
L D D4
U 1 1 59C1962B
P 4950 2600
F 0 "D4" H 4950 2700 50  0000 C CNN
F 1 "D" H 4950 2500 50  0001 C CNN
F 2 "CA6:D_219AB" H 4950 2600 50  0001 C CNN
F 3 "" H 4950 2600 50  0000 C CNN
	1    4950 2600
	0    1    1    0   
$EndComp
$Comp
L Q_NMOS_GSD Q4
U 1 1 59C19631
P 5050 3000
F 0 "Q4" H 5250 3050 50  0000 L CNN
F 1 "Q_NMOS_GSD" H 5250 2950 50  0001 L CNN
F 2 "TO_SOT_Packages_SMD:SOT-23" H 5250 3100 50  0001 C CNN
F 3 "" H 5050 3000 50  0000 C CNN
	1    5050 3000
	1    0    0    -1  
$EndComp
$Comp
L BARREL_JACK Valve4
U 1 1 59C19637
P 5600 2600
F 0 "Valve4" H 5600 2850 50  0000 C CNN
F 1 "Valve4" H 5600 2400 50  0000 C CNN
F 2 "Connectors:BARREL_JACK" H 5600 2600 50  0001 C CNN
F 3 "" H 5600 2600 50  0000 C CNN
	1    5600 2600
	-1   0    0    -1  
$EndComp
Wire Wire Line
	5300 2700 5150 2700
Wire Wire Line
	5150 2700 5150 2800
Wire Wire Line
	5150 2500 5300 2500
Wire Wire Line
	5150 2250 5150 2500
$Comp
L +12V #PWR08
U 1 1 59C19644
P 5000 2250
F 0 "#PWR08" H 5000 2100 50  0001 C CNN
F 1 "+12V" H 5000 2390 50  0000 C CNN
F 2 "" H 5000 2250 50  0000 C CNN
F 3 "" H 5000 2250 50  0000 C CNN
	1    5000 2250
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4950 2450 5150 2450
Connection ~ 5150 2450
Wire Wire Line
	4950 2750 5150 2750
Connection ~ 5150 2750
Wire Wire Line
	5150 2250 5000 2250
Wire Wire Line
	5150 3200 5150 3250
$Comp
L GND #PWR09
U 1 1 59C19650
P 5150 3250
F 0 "#PWR09" H 5150 3000 50  0001 C CNN
F 1 "GND" H 5150 3100 50  0000 C CNN
F 2 "" H 5150 3250 50  0000 C CNN
F 3 "" H 5150 3250 50  0000 C CNN
	1    5150 3250
	1    0    0    -1  
$EndComp
Wire Wire Line
	1400 1250 1450 1250
Wire Wire Line
	1450 1250 1450 1400
$Comp
L GND #PWR010
U 1 1 59C1A02E
P 1450 1400
F 0 "#PWR010" H 1450 1150 50  0001 C CNN
F 1 "GND" H 1450 1250 50  0000 C CNN
F 2 "" H 1450 1400 50  0000 C CNN
F 3 "" H 1450 1400 50  0000 C CNN
	1    1450 1400
	1    0    0    -1  
$EndComp
$Comp
L +12V #PWR011
U 1 1 59C1A081
P 1550 1150
F 0 "#PWR011" H 1550 1000 50  0001 C CNN
F 1 "+12V" H 1550 1290 50  0000 C CNN
F 2 "" H 1550 1150 50  0000 C CNN
F 3 "" H 1550 1150 50  0000 C CNN
	1    1550 1150
	0    1    1    0   
$EndComp
Wire Wire Line
	1400 1150 1550 1150
Wire Wire Line
	1400 1050 1800 1050
Text Label 1800 1050 0    60   ~ 0
Servo1
$Comp
L CONN_01X03 Servo2
U 1 1 59C1A1B4
P 1200 1850
F 0 "Servo2" H 1200 2050 50  0000 C CNN
F 1 "Servo2" V 1300 1850 50  0000 C CNN
F 2 "Socket_Strips:Socket_Strip_Angled_1x03_Pitch2.54mm" H 1200 1850 50  0001 C CNN
F 3 "" H 1200 1850 50  0000 C CNN
	1    1200 1850
	-1   0    0    -1  
$EndComp
Wire Wire Line
	1400 1950 1450 1950
Wire Wire Line
	1450 1950 1450 2100
$Comp
L GND #PWR012
U 1 1 59C1A1BC
P 1450 2100
F 0 "#PWR012" H 1450 1850 50  0001 C CNN
F 1 "GND" H 1450 1950 50  0000 C CNN
F 2 "" H 1450 2100 50  0000 C CNN
F 3 "" H 1450 2100 50  0000 C CNN
	1    1450 2100
	1    0    0    -1  
$EndComp
$Comp
L +12V #PWR013
U 1 1 59C1A1C2
P 1550 1850
F 0 "#PWR013" H 1550 1700 50  0001 C CNN
F 1 "+12V" H 1550 1990 50  0000 C CNN
F 2 "" H 1550 1850 50  0000 C CNN
F 3 "" H 1550 1850 50  0000 C CNN
	1    1550 1850
	0    1    1    0   
$EndComp
Wire Wire Line
	1400 1850 1550 1850
Wire Wire Line
	1400 1750 1800 1750
Text Label 1800 1750 0    60   ~ 0
Servo2
$Comp
L BARREL_JACK POWER1
U 1 1 59C1A763
P 4450 3950
F 0 "POWER1" H 4450 4200 50  0000 C CNN
F 1 "Power" H 4450 3750 50  0000 C CNN
F 2 "Connectors:BARREL_JACK" H 4450 3950 50  0001 C CNN
F 3 "" H 4450 3950 50  0000 C CNN
	1    4450 3950
	-1   0    0    -1  
$EndComp
$Comp
L GND #PWR014
U 1 1 59C1B397
P 3500 4400
F 0 "#PWR014" H 3500 4150 50  0001 C CNN
F 1 "GND" H 3500 4250 50  0000 C CNN
F 2 "" H 3500 4400 50  0000 C CNN
F 3 "" H 3500 4400 50  0000 C CNN
	1    3500 4400
	1    0    0    -1  
$EndComp
$Comp
L +12V #PWR015
U 1 1 59C1B42A
P 3500 3700
F 0 "#PWR015" H 3500 3550 50  0001 C CNN
F 1 "+12V" H 3500 3840 50  0000 C CNN
F 2 "" H 3500 3700 50  0000 C CNN
F 3 "" H 3500 3700 50  0000 C CNN
	1    3500 3700
	1    0    0    -1  
$EndComp
$Comp
L CAP_ELEC C2
U 1 1 59C1B624
P 3800 4050
F 0 "C2" H 3850 4150 50  0000 L CNN
F 1 "100uF" H 3850 3950 50  0000 L CNN
F 2 "CA6:c_elec_6.3x5.7" H 3800 4050 60  0001 C CNN
F 3 "" H 3800 4050 60  0000 C CNN
	1    3800 4050
	1    0    0    -1  
$EndComp
$Comp
L C C1
U 1 1 59C1B6A1
P 3500 4050
F 0 "C1" H 3525 4150 50  0000 L CNN
F 1 "10uF" H 3525 3950 50  0000 L CNN
F 2 "Capacitors_SMD:C_1206" H 3538 3900 50  0001 C CNN
F 3 "" H 3500 4050 50  0000 C CNN
	1    3500 4050
	1    0    0    -1  
$EndComp
Wire Wire Line
	3100 3850 4150 3850
Wire Wire Line
	4150 4250 4150 4050
Wire Wire Line
	3500 4250 4150 4250
Wire Wire Line
	3500 3700 3500 3900
Connection ~ 3800 3850
Wire Wire Line
	3500 4200 3500 4400
Connection ~ 3800 4250
Connection ~ 3500 3850
Connection ~ 3500 4250
$Comp
L LED D5
U 1 1 59DEC903
P 3100 4400
F 0 "D5" H 3100 4500 50  0000 C CNN
F 1 "LED" H 3100 4300 50  0000 C CNN
F 2 "LEDs:LED_0805" H 3100 4400 50  0001 C CNN
F 3 "" H 3100 4400 50  0000 C CNN
	1    3100 4400
	0    -1   -1   0   
$EndComp
$Comp
L R R9
U 1 1 59DECC88
P 3100 4050
F 0 "R9" V 3180 4050 50  0000 C CNN
F 1 "1K" V 3100 4050 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" V 3030 4050 50  0001 C CNN
F 3 "" H 3100 4050 50  0000 C CNN
	1    3100 4050
	1    0    0    -1  
$EndComp
Wire Wire Line
	3100 3850 3100 3900
Wire Wire Line
	3100 4200 3100 4250
Wire Wire Line
	3100 4550 3100 4700
$Comp
L GND #PWR016
U 1 1 59DED33E
P 3100 4700
F 0 "#PWR016" H 3100 4450 50  0001 C CNN
F 1 "GND" H 3100 4550 50  0000 C CNN
F 2 "" H 3100 4700 50  0000 C CNN
F 3 "" H 3100 4700 50  0000 C CNN
	1    3100 4700
	1    0    0    -1  
$EndComp
$Comp
L HOLE X1
U 1 1 59DEEB5B
P 6450 1150
F 0 "X1" H 6450 1050 60  0000 C CNN
F 1 "HOLE" H 6450 1250 60  0000 C CNN
F 2 "Mounting_Holes:MountingHole_2.5mm_Pad" H 6450 1150 60  0001 C CNN
F 3 "" H 6450 1150 60  0000 C CNN
	1    6450 1150
	1    0    0    -1  
$EndComp
$Comp
L HOLE X2
U 1 1 59DEEE16
P 6450 1500
F 0 "X2" H 6450 1400 60  0000 C CNN
F 1 "HOLE" H 6450 1600 60  0000 C CNN
F 2 "Mounting_Holes:MountingHole_2.5mm_Pad" H 6450 1500 60  0001 C CNN
F 3 "" H 6450 1500 60  0000 C CNN
	1    6450 1500
	1    0    0    -1  
$EndComp
$Comp
L HOLE X3
U 1 1 59DEEE7F
P 6450 1850
F 0 "X3" H 6450 1750 60  0000 C CNN
F 1 "HOLE" H 6450 1950 60  0000 C CNN
F 2 "Mounting_Holes:MountingHole_2.5mm_Pad" H 6450 1850 60  0001 C CNN
F 3 "" H 6450 1850 60  0000 C CNN
	1    6450 1850
	1    0    0    -1  
$EndComp
$Comp
L HOLE X4
U 1 1 59DEEEE1
P 6450 2200
F 0 "X4" H 6450 2100 60  0000 C CNN
F 1 "HOLE" H 6450 2300 60  0000 C CNN
F 2 "Mounting_Holes:MountingHole_2.5mm_Pad" H 6450 2200 60  0001 C CNN
F 3 "" H 6450 2200 60  0000 C CNN
	1    6450 2200
	1    0    0    -1  
$EndComp
$Comp
L R R1
U 1 1 59DF08C8
P 2950 1550
F 0 "R1" V 3030 1550 50  0000 C CNN
F 1 "1K" V 2950 1550 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" V 2880 1550 50  0001 C CNN
F 3 "" H 2950 1550 50  0000 C CNN
	1    2950 1550
	0    1    1    0   
$EndComp
$Comp
L R R2
U 1 1 59DF0BD6
P 3150 1700
F 0 "R2" V 3230 1700 50  0000 C CNN
F 1 "10K" V 3150 1700 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" V 3080 1700 50  0001 C CNN
F 3 "" H 3150 1700 50  0000 C CNN
	1    3150 1700
	-1   0    0    1   
$EndComp
$Comp
L GND #PWR017
U 1 1 59DF0C59
P 3150 1850
F 0 "#PWR017" H 3150 1600 50  0001 C CNN
F 1 "GND" H 3150 1700 50  0000 C CNN
F 2 "" H 3150 1850 50  0000 C CNN
F 3 "" H 3150 1850 50  0000 C CNN
	1    3150 1850
	1    0    0    -1  
$EndComp
Wire Wire Line
	3100 1550 3200 1550
Connection ~ 3150 1550
Wire Wire Line
	2800 1550 2450 1550
Text Label 2450 1550 0    60   ~ 0
Valve1
$Comp
L R R3
U 1 1 59DF1299
P 4600 1550
F 0 "R3" V 4680 1550 50  0000 C CNN
F 1 "1K" V 4600 1550 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" V 4530 1550 50  0001 C CNN
F 3 "" H 4600 1550 50  0000 C CNN
	1    4600 1550
	0    1    1    0   
$EndComp
$Comp
L R R4
U 1 1 59DF129F
P 4800 1700
F 0 "R4" V 4880 1700 50  0000 C CNN
F 1 "10K" V 4800 1700 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" V 4730 1700 50  0001 C CNN
F 3 "" H 4800 1700 50  0000 C CNN
	1    4800 1700
	-1   0    0    1   
$EndComp
$Comp
L GND #PWR018
U 1 1 59DF12A5
P 4800 1850
F 0 "#PWR018" H 4800 1600 50  0001 C CNN
F 1 "GND" H 4800 1700 50  0000 C CNN
F 2 "" H 4800 1850 50  0000 C CNN
F 3 "" H 4800 1850 50  0000 C CNN
	1    4800 1850
	1    0    0    -1  
$EndComp
Wire Wire Line
	4750 1550 4850 1550
Connection ~ 4800 1550
Wire Wire Line
	4450 1550 4100 1550
Text Label 4100 1550 0    60   ~ 0
Valve2
$Comp
L R R7
U 1 1 59DF1508
P 4600 3000
F 0 "R7" V 4680 3000 50  0000 C CNN
F 1 "1K" V 4600 3000 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" V 4530 3000 50  0001 C CNN
F 3 "" H 4600 3000 50  0000 C CNN
	1    4600 3000
	0    1    1    0   
$EndComp
$Comp
L R R8
U 1 1 59DF150E
P 4800 3150
F 0 "R8" V 4880 3150 50  0000 C CNN
F 1 "10K" V 4800 3150 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" V 4730 3150 50  0001 C CNN
F 3 "" H 4800 3150 50  0000 C CNN
	1    4800 3150
	-1   0    0    1   
$EndComp
$Comp
L GND #PWR019
U 1 1 59DF1514
P 4800 3300
F 0 "#PWR019" H 4800 3050 50  0001 C CNN
F 1 "GND" H 4800 3150 50  0000 C CNN
F 2 "" H 4800 3300 50  0000 C CNN
F 3 "" H 4800 3300 50  0000 C CNN
	1    4800 3300
	1    0    0    -1  
$EndComp
Wire Wire Line
	4750 3000 4850 3000
Connection ~ 4800 3000
Wire Wire Line
	4450 3000 4100 3000
Text Label 4100 3000 0    60   ~ 0
Valve4
$Comp
L R R5
U 1 1 59DF1610
P 2950 3000
F 0 "R5" V 3030 3000 50  0000 C CNN
F 1 "1K" V 2950 3000 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" V 2880 3000 50  0001 C CNN
F 3 "" H 2950 3000 50  0000 C CNN
	1    2950 3000
	0    1    1    0   
$EndComp
$Comp
L R R6
U 1 1 59DF1616
P 3150 3150
F 0 "R6" V 3230 3150 50  0000 C CNN
F 1 "10K" V 3150 3150 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" V 3080 3150 50  0001 C CNN
F 3 "" H 3150 3150 50  0000 C CNN
	1    3150 3150
	-1   0    0    1   
$EndComp
$Comp
L GND #PWR020
U 1 1 59DF161C
P 3150 3300
F 0 "#PWR020" H 3150 3050 50  0001 C CNN
F 1 "GND" H 3150 3150 50  0000 C CNN
F 2 "" H 3150 3300 50  0000 C CNN
F 3 "" H 3150 3300 50  0000 C CNN
	1    3150 3300
	1    0    0    -1  
$EndComp
Wire Wire Line
	3100 3000 3200 3000
Connection ~ 3150 3000
Wire Wire Line
	2800 3000 2450 3000
Text Label 2450 3000 0    60   ~ 0
Valve3
NoConn ~ 2300 3500
NoConn ~ 4150 3950
NoConn ~ 3650 2600
NoConn ~ 5300 2600
NoConn ~ 5300 1150
NoConn ~ 3650 1150
$EndSCHEMATC