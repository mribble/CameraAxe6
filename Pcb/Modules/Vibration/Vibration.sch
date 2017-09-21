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
U 1 1 59C40B7A
P 1500 1400
F 0 "J1" H 1675 1900 39  0000 C CNN
F 1 "JACK_RJ45" H 1325 1900 39  0001 C CNN
F 2 "CA6:RJ45_pcbwing" H 1475 1400 60  0001 C CNN
F 3 "" H 1475 1400 60  0000 C CNN
	1    1500 1400
	0    -1   -1   0   
$EndComp
$Comp
L GND #PWR1
U 1 1 59C40C28
P 2100 2150
F 0 "#PWR1" H 2100 1900 50  0001 C CNN
F 1 "GND" H 2100 2000 50  0000 C CNN
F 2 "" H 2100 2150 50  0000 C CNN
F 3 "" H 2100 2150 50  0000 C CNN
	1    2100 2150
	1    0    0    -1  
$EndComp
$Comp
L Ferrite_Bead L1
U 1 1 59C40C66
P 2100 1900
F 0 "L1" V 1950 1925 50  0000 C CNN
F 1 "Ferrite_Bead" V 2250 1900 50  0001 C CNN
F 2 "Resistors_SMD:R_0805" V 2030 1900 50  0001 C CNN
F 3 "" H 2100 1900 50  0000 C CNN
	1    2100 1900
	1    0    0    -1  
$EndComp
Wire Wire Line
	1950 1750 2100 1750
Wire Wire Line
	2100 2050 2100 2150
Wire Wire Line
	1950 1650 3050 1650
$Comp
L R R1
U 1 1 59C40D7E
P 2500 1850
F 0 "R1" V 2580 1850 50  0000 C CNN
F 1 "1M" V 2500 1850 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" V 2430 1850 50  0001 C CNN
F 3 "" H 2500 1850 50  0000 C CNN
	1    2500 1850
	-1   0    0    1   
$EndComp
Wire Wire Line
	2100 2100 3050 2100
Wire Wire Line
	2500 2100 2500 2000
Connection ~ 2100 2100
Wire Wire Line
	2500 1650 2500 1700
$Comp
L D_Schottky D1
U 1 1 59C40E4B
P 2800 1850
F 0 "D1" H 2800 1950 50  0000 C CNN
F 1 "D_Schottky" H 2800 1750 50  0001 C CNN
F 2 "Diodes_SMD:D_SOT-23_ANK" H 2800 1850 50  0001 C CNN
F 3 "" H 2800 1850 50  0000 C CNN
	1    2800 1850
	0    1    1    0   
$EndComp
Wire Wire Line
	2800 1650 2800 1700
Connection ~ 2500 1650
Wire Wire Line
	2800 2100 2800 2000
Connection ~ 2500 2100
Connection ~ 2800 1650
Connection ~ 2800 2100
$Comp
L CONN_01X01 P1
U 1 1 59C4105F
P 3250 1650
F 0 "P1" H 3250 1750 50  0000 C CNN
F 1 "C+" V 3350 1650 50  0000 C CNN
F 2 "Wire_Pads:SolderWirePad_single_SMD_5x10mm" H 3250 1650 50  0001 C CNN
F 3 "" H 3250 1650 50  0000 C CNN
	1    3250 1650
	1    0    0    -1  
$EndComp
$Comp
L CONN_01X01 P2
U 1 1 59C410A1
P 3250 2100
F 0 "P2" H 3250 2200 50  0000 C CNN
F 1 "C-" V 3350 2100 50  0000 C CNN
F 2 "Wire_Pads:SolderWirePad_single_SMD_5x10mm" H 3250 2100 50  0001 C CNN
F 3 "" H 3250 2100 50  0000 C CNN
	1    3250 2100
	1    0    0    -1  
$EndComp
$EndSCHEMATC
