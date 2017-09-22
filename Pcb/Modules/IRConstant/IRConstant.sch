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
LIBS:IRConstant-cache
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
L JACK_RJ45 J1
U 1 1 59C5154F
P 1250 1500
F 0 "J1" H 1425 2000 39  0000 C CNN
F 1 "JACK_RJ45" H 1075 2000 39  0000 C CNN
F 2 "CA6:RJ45_pcbwing" H 1225 1500 60  0001 C CNN
F 3 "" H 1225 1500 60  0000 C CNN
	1    1250 1500
	0    -1   -1   0   
$EndComp
$Comp
L +3.3V #PWR1
U 1 1 59C51660
P 2600 950
F 0 "#PWR1" H 2600 800 50  0001 C CNN
F 1 "+3.3V" H 2600 1090 50  0000 C CNN
F 2 "" H 2600 950 50  0000 C CNN
F 3 "" H 2600 950 50  0000 C CNN
	1    2600 950 
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR2
U 1 1 59C51676
P 2600 2050
F 0 "#PWR2" H 2600 1800 50  0001 C CNN
F 1 "GND" H 2600 1900 50  0000 C CNN
F 2 "" H 2600 2050 50  0000 C CNN
F 3 "" H 2600 2050 50  0000 C CNN
	1    2600 2050
	1    0    0    -1  
$EndComp
$Comp
L Ferrite_Bead L2
U 1 1 59C516B4
P 2100 2000
F 0 "L2" V 1950 2025 50  0000 C CNN
F 1 "Ferrite_Bead" V 2250 2100 50  0000 C CNN
F 2 "Resistors_SMD:R_0805" V 2030 2000 50  0001 C CNN
F 3 "" H 2100 2000 50  0000 C CNN
	1    2100 2000
	0    1    1    0   
$EndComp
$Comp
L Ferrite_Bead L1
U 1 1 59C51723
P 2100 1000
F 0 "L1" V 1950 1025 50  0000 C CNN
F 1 "Ferrite_Bead" V 2250 1100 50  0000 C CNN
F 2 "Resistors_SMD:R_0805" V 2030 1000 50  0001 C CNN
F 3 "" H 2100 1000 50  0000 C CNN
	1    2100 1000
	0    1    1    0   
$EndComp
$Comp
L C C1
U 1 1 59C5188D
P 2600 1500
F 0 "C1" H 2625 1600 50  0000 L CNN
F 1 "10uF" H 2625 1400 50  0000 L CNN
F 2 "Capacitors_SMD:C_1206" H 2638 1350 50  0001 C CNN
F 3 "" H 2600 1500 50  0000 C CNN
	1    2600 1500
	1    0    0    -1  
$EndComp
$Comp
L C C2
U 1 1 59C518E6
P 3000 1500
F 0 "C2" H 3025 1600 50  0000 L CNN
F 1 "0.1uF" H 3025 1400 50  0000 L CNN
F 2 "Capacitors_SMD:C_0603" H 3038 1350 50  0001 C CNN
F 3 "" H 3000 1500 50  0000 C CNN
	1    3000 1500
	1    0    0    -1  
$EndComp
$Comp
L R R1
U 1 1 59C519EC
P 3650 1500
F 0 "R1" V 3730 1500 50  0000 C CNN
F 1 "R" V 3650 1500 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" V 3580 1500 50  0001 C CNN
F 3 "" H 3650 1500 50  0000 C CNN
	1    3650 1500
	1    0    0    -1  
$EndComp
Wire Wire Line
	1700 1150 1950 1150
Wire Wire Line
	1950 1150 1950 1000
Wire Wire Line
	1950 2000 1950 1850
Wire Wire Line
	1950 1850 1700 1850
Wire Wire Line
	2250 1000 3000 1000
Wire Wire Line
	2600 950  2600 1350
Wire Wire Line
	2250 2000 3000 2000
Wire Wire Line
	2600 1650 2600 2050
Connection ~ 2600 1000
Connection ~ 2600 2000
Wire Wire Line
	3000 2000 3000 1650
Wire Wire Line
	3000 1000 3000 1350
Wire Wire Line
	1700 1750 1950 1750
Text Label 1950 1750 0    60   ~ 0
Signal
$EndSCHEMATC
