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
LIBS:IRPulse-cache
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
U 1 1 59C51B7D
P 1350 1750
F 0 "J1" H 1525 2250 39  0000 C CNN
F 1 "JACK_RJ45" H 1175 2250 39  0000 C CNN
F 2 "CA6:RJ45_pcbwing" H 1325 1750 60  0001 C CNN
F 3 "" H 1325 1750 60  0000 C CNN
	1    1350 1750
	0    -1   -1   0   
$EndComp
$Comp
L Ferrite_Bead L1
U 1 1 59C51D7D
P 2200 1250
F 0 "L1" V 2050 1275 50  0000 C CNN
F 1 "Ferrite_Bead" V 2350 1350 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" V 2130 1250 50  0001 C CNN
F 3 "" H 2200 1250 50  0000 C CNN
	1    2200 1250
	0    1    1    0   
$EndComp
$Comp
L Ferrite_Bead L2
U 1 1 59C51E56
P 2200 2250
F 0 "L2" V 2050 2275 50  0000 C CNN
F 1 "Ferrite_Bead" V 2350 2350 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" V 2130 2250 50  0001 C CNN
F 3 "" H 2200 2250 50  0000 C CNN
	1    2200 2250
	0    1    1    0   
$EndComp
Wire Wire Line
	1800 1400 2050 1400
Wire Wire Line
	2050 1400 2050 1250
Wire Wire Line
	1800 2100 2050 2100
Wire Wire Line
	2050 2100 2050 2250
Wire Wire Line
	1800 2000 2050 2000
Text Label 2050 2000 0    60   ~ 0
Signal
$Comp
L +3.3V #PWR1
U 1 1 59C51F37
P 2700 1200
F 0 "#PWR1" H 2700 1050 50  0001 C CNN
F 1 "+3.3V" H 2700 1340 50  0000 C CNN
F 2 "" H 2700 1200 50  0000 C CNN
F 3 "" H 2700 1200 50  0000 C CNN
	1    2700 1200
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR2
U 1 1 59C51F84
P 2700 2300
F 0 "#PWR2" H 2700 2050 50  0001 C CNN
F 1 "GND" H 2700 2150 50  0000 C CNN
F 2 "" H 2700 2300 50  0000 C CNN
F 3 "" H 2700 2300 50  0000 C CNN
	1    2700 2300
	1    0    0    -1  
$EndComp
Wire Wire Line
	2350 1250 3100 1250
Wire Wire Line
	2700 1200 2700 1600
Connection ~ 2700 1250
Wire Wire Line
	2350 2250 3100 2250
Wire Wire Line
	2700 1900 2700 2300
Connection ~ 2700 2250
$Comp
L C C1
U 1 1 59C51FD2
P 2700 1750
F 0 "C1" H 2725 1850 50  0000 L CNN
F 1 "10uF" H 2725 1650 50  0000 L CNN
F 2 "Capacitors_SMD:C_1206" H 2738 1600 50  0001 C CNN
F 3 "" H 2700 1750 50  0000 C CNN
	1    2700 1750
	1    0    0    -1  
$EndComp
$Comp
L C C2
U 1 1 59C5210D
P 3100 1750
F 0 "C2" H 3125 1850 50  0000 L CNN
F 1 "0.1uF" H 3125 1650 50  0000 L CNN
F 2 "Capacitors_SMD:C_0603" H 3138 1600 50  0001 C CNN
F 3 "" H 3100 1750 50  0000 C CNN
	1    3100 1750
	1    0    0    -1  
$EndComp
Wire Wire Line
	3100 2250 3100 1900
Wire Wire Line
	3100 1250 3100 1600
$EndSCHEMATC
