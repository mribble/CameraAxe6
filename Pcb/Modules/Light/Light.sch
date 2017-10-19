EESchema Schematic File Version 2
LIBS:CA6
LIBS:74xx
LIBS:74xgxx
LIBS:ac-dc
LIBS:actel
LIBS:adc-dac
LIBS:allegro
LIBS:Altera
LIBS:analog_devices
LIBS:analog_switches
LIBS:atmel
LIBS:audio
LIBS:battery_management
LIBS:bbd
LIBS:brooktre
LIBS:cmos_ieee
LIBS:cmos4000
LIBS:conn
LIBS:contrib
LIBS:cypress
LIBS:dc-dc
LIBS:device
LIBS:digital-audio
LIBS:diode
LIBS:display
LIBS:dsp
LIBS:elec-unifil
LIBS:ESD_Protection
LIBS:ftdi
LIBS:gennum
LIBS:graphic
LIBS:hc11
LIBS:intel
LIBS:interface
LIBS:ir
LIBS:Lattice
LIBS:linear
LIBS:logo
LIBS:maxim
LIBS:mechanical
LIBS:memory
LIBS:microchip
LIBS:microchip_dspic33dsc
LIBS:microchip_pic10mcu
LIBS:microchip_pic12mcu
LIBS:microchip_pic16mcu
LIBS:microchip_pic18mcu
LIBS:microchip_pic32mcu
LIBS:microcontrollers
LIBS:motor_drivers
LIBS:motorola
LIBS:motors
LIBS:msp430
LIBS:nordicsemi
LIBS:nxp_armmcu
LIBS:onsemi
LIBS:opto
LIBS:Oscillators
LIBS:philips
LIBS:power
LIBS:Power_Management
LIBS:powerint
LIBS:pspice
LIBS:references
LIBS:regul
LIBS:relays
LIBS:rfcom
LIBS:sensors
LIBS:silabs
LIBS:siliconi
LIBS:stm8
LIBS:stm32
LIBS:supertex
LIBS:switches
LIBS:texas
LIBS:transf
LIBS:transistors
LIBS:ttl_ieee
LIBS:valves
LIBS:video
LIBS:wiznet
LIBS:Worldsemi
LIBS:Xicor
LIBS:xilinx
LIBS:zetex
LIBS:Zilog
EELAYER 25 0
EELAYER END
$Descr USLetter 11000 8500
encoding utf-8
Sheet 1 1
Title "CA6 version of New Light Sensor Module with Extensible Snoot"
Date "Monday, April 24, 2017"
Rev ""
Comp "DreamingRobots.com"
Comment1 "D. M. Lenardon"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L JACK_STEREO J2
U 1 1 54012E4D
P 6150 5200
F 0 "J2" H 5775 5200 50  0000 R CNN
F 1 "JACK_STEREO" H 6150 5450 50  0000 C CNN
F 2 "CA6:161-3334-E_Big_Pad" H 6150 5200 60  0001 C CNN
F 3 "~" H 6150 5200 60  0000 C CNN
	1    6150 5200
	1    0    0    -1  
$EndComp
$Comp
L JACK_RJ45 J1
U 1 1 5401301C
P 4200 3650
F 0 "J1" H 4375 4150 39  0000 C CNN
F 1 "JACK_RJ45" H 4025 4150 39  0000 C CNN
F 2 "CA6:RJ45_pcbwing" H 4175 3650 60  0001 C CNN
F 3 "~" H 4175 3650 60  0000 C CNN
	1    4200 3650
	0    -1   1    0   
$EndComp
$Comp
L 3V3 #PWR01
U 1 1 540144B1
P 4650 3200
F 0 "#PWR01" H 4650 3300 40  0001 C CNN
F 1 "3V3" H 4650 3325 40  0000 C CNN
F 2 "" H 4650 3200 60  0000 C CNN
F 3 "" H 4650 3200 60  0000 C CNN
	1    4650 3200
	1    0    0    -1  
$EndComp
Text Label 4650 3400 0    60   ~ 0
AN1
Text Label 4650 3500 0    60   ~ 0
D1
Text Label 4650 3600 0    60   ~ 0
D2
Text Label 4650 3700 0    60   ~ 0
ID
Text Label 4650 3800 0    60   ~ 0
D3
Text Label 4650 3900 0    60   ~ 0
D4
$Comp
L GND #PWR02
U 1 1 540144C2
P 4650 4100
F 0 "#PWR02" H 4650 4100 30  0001 C CNN
F 1 "GND" H 4650 4030 30  0001 C CNN
F 2 "" H 4650 4100 60  0000 C CNN
F 3 "" H 4650 4100 60  0000 C CNN
	1    4650 4100
	1    0    0    -1  
$EndComp
Wire Wire Line
	4650 3200 4650 3300
$Comp
L 3V3 #PWR03
U 1 1 5401450D
P 6550 4300
F 0 "#PWR03" H 6550 4400 40  0001 C CNN
F 1 "3V3" H 6550 4425 40  0000 C CNN
F 2 "" H 6550 4300 60  0000 C CNN
F 3 "" H 6550 4300 60  0000 C CNN
	1    6550 4300
	-1   0    0    -1  
$EndComp
$Comp
L GND #PWR04
U 1 1 5401451B
P 6500 5450
F 0 "#PWR04" H 6500 5450 30  0001 C CNN
F 1 "GND" H 6500 5380 30  0001 C CNN
F 2 "" H 6500 5450 60  0000 C CNN
F 3 "" H 6500 5450 60  0000 C CNN
	1    6500 5450
	1    0    0    -1  
$EndComp
Wire Wire Line
	6500 5350 6500 5450
Wire Wire Line
	4650 4000 4650 4100
Text Label 5450 4600 0    60   ~ 0
D1
$Comp
L R R3
U 1 1 54AA118B
P 5800 4600
F 0 "R3" V 5880 4600 40  0000 C CNN
F 1 "1K" V 5807 4601 40  0000 C CNN
F 2 "CA6:SM0603_Resistor" V 5730 4600 30  0001 C CNN
F 3 "~" H 5800 4600 30  0000 C CNN
	1    5800 4600
	0    -1   -1   0   
$EndComp
$Comp
L OPTO_NPN Q1
U 1 1 54AA12A2
P 5850 3050
F 0 "Q1" H 6000 3100 50  0000 L CNN
F 1 "TEPT4400" H 5500 3200 50  0000 L CNN
F 2 "CA6:PhotoTrans_Conn_Pads_1x02_Pitch2.54mm" H 5850 3050 60  0001 C CNN
F 3 "http://www.mouser.com/ds/2/427/tept4400-244748.pdf" H 5850 3050 60  0001 C CNN
	1    5850 3050
	-1   0    0    -1  
$EndComp
$Comp
L R R1
U 1 1 54AA13F6
P 5500 3400
F 0 "R1" V 5580 3400 40  0000 C CNN
F 1 "1K" V 5507 3401 40  0000 C CNN
F 2 "CA6:SM0603_Resistor" V 5430 3400 30  0001 C CNN
F 3 "~" H 5500 3400 30  0000 C CNN
	1    5500 3400
	0    -1   -1   0   
$EndComp
$Comp
L POT RV1
U 1 1 54AA14B9
P 5750 3600
F 0 "RV1" H 5900 3700 50  0000 C CNN
F 1 "20K" V 5750 3600 50  0000 C CNN
F 2 "Potentiometers:Potentiometer_Alps_RK09K_Vertical" H 5750 3600 591 0001 C CNN
F 3 "http://www.mouser.com/ProductDetail/ALPS/RK09K1110AH8/?qs=sGAEpiMZZMtC25l1F4XBU3JeeCo7XMTlexI3BEsH1y4%3d" H 5750 3600 60  0001 C CNN
	1    5750 3600
	1    0    0    -1  
$EndComp
$Comp
L R R2
U 1 1 54AA1792
P 5750 4050
F 0 "R2" V 5830 4050 40  0000 C CNN
F 1 "174 1%" V 5757 4051 40  0000 C CNN
F 2 "CA6:SM0805_Resistor" V 5680 4050 30  0001 C CNN
F 3 "http://www.mouser.com/ds/2/315/AOA0000C283-1093288.pdf" H 5750 4050 30  0001 C CNN
	1    5750 4050
	1    0    0    -1  
$EndComp
$Comp
L 3V3 #PWR05
U 1 1 54AA199D
P 5750 2750
F 0 "#PWR05" H 5750 2850 40  0001 C CNN
F 1 "3V3" H 5750 2875 40  0000 C CNN
F 2 "" H 5750 2750 60  0000 C CNN
F 3 "" H 5750 2750 60  0000 C CNN
	1    5750 2750
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR06
U 1 1 54AA1A1B
P 5750 4300
F 0 "#PWR06" H 5750 4300 30  0001 C CNN
F 1 "GND" H 5750 4230 30  0001 C CNN
F 2 "" H 5750 4300 60  0000 C CNN
F 3 "" H 5750 4300 60  0000 C CNN
	1    5750 4300
	1    0    0    -1  
$EndComp
Connection ~ 5750 4300
Text Notes 6650 5300 0    60   ~ 0
3.5mm Jack for\nLaser Power
Text Notes 6000 3650 0    39   ~ 0
Sensitivity Adj.
Text Notes 3900 2450 0    60   ~ 0
CA6 version of New Light Sensor Module with Extensible Snoot
$Comp
L C C1
U 1 1 54AABC31
P 5250 2850
F 0 "C1" H 5250 2950 40  0000 L CNN
F 1 "10uF" H 5256 2765 40  0000 L CNN
F 2 "CA6:c_1206" H 5288 2700 30  0001 C CNN
F 3 "~" H 5250 2850 60  0000 C CNN
	1    5250 2850
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR07
U 1 1 54AABCE0
P 5250 3100
F 0 "#PWR07" H 5250 3100 30  0001 C CNN
F 1 "GND" H 5250 3030 30  0001 C CNN
F 2 "" H 5250 3100 60  0000 C CNN
F 3 "" H 5250 3100 60  0000 C CNN
	1    5250 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	5750 3250 5750 3450
Connection ~ 5750 3400
Wire Wire Line
	5750 3750 5750 3800
Wire Wire Line
	5900 3600 5900 3750
Wire Wire Line
	5900 3750 5750 3750
Connection ~ 5750 3750
Wire Wire Line
	4650 3400 5250 3400
Wire Wire Line
	5400 4600 5400 3500
Wire Wire Line
	5400 3500 4650 3500
NoConn ~ 4650 3600
NoConn ~ 4650 3800
NoConn ~ 4650 3900
NoConn ~ 6500 5250
$Comp
L PWR_FLAG #FLG08
U 1 1 58FEFDE5
P 4300 2950
F 0 "#FLG08" H 4300 3025 50  0001 C CNN
F 1 "PWR_FLAG" H 4300 3100 50  0000 C CNN
F 2 "" H 4300 2950 50  0001 C CNN
F 3 "" H 4300 2950 50  0001 C CNN
	1    4300 2950
	-1   0    0    1   
$EndComp
$Comp
L 3V3 #PWR09
U 1 1 58FEFE63
P 4300 2850
F 0 "#PWR09" H 4300 2950 40  0001 C CNN
F 1 "3V3" H 4300 2975 40  0000 C CNN
F 2 "" H 4300 2850 60  0000 C CNN
F 3 "" H 4300 2850 60  0000 C CNN
	1    4300 2850
	1    0    0    -1  
$EndComp
Wire Wire Line
	4300 2950 4300 2850
Wire Wire Line
	5250 3100 5250 3050
Wire Wire Line
	5750 2750 5750 2850
Wire Wire Line
	5250 2650 5500 2650
Wire Wire Line
	5500 2650 5500 2800
Wire Wire Line
	5500 2800 5750 2800
Connection ~ 5750 2800
Text Notes 6000 3150 0    60   ~ 0
just wire pads on board\nphoto trans. connected via wires
Wire Wire Line
	6250 4600 6050 4600
Wire Wire Line
	6550 4800 6550 5050
Wire Wire Line
	6550 5050 6500 5050
Wire Wire Line
	5550 4600 5400 4600
Wire Wire Line
	6550 4400 6550 4300
Text Notes 5900 4100 0    39   ~ 0
Choose R2 for MAX current of 20mA\n180 = 18.3 mA\n174 = 18.9 mA\n169 = 19.5 mA
NoConn ~ 4650 3700
$Comp
L PWR_FLAG #FLG010
U 1 1 59E23186
P 4200 4350
F 0 "#FLG010" H 4200 4425 50  0001 C CNN
F 1 "PWR_FLAG" H 4200 4500 50  0000 C CNN
F 2 "" H 4200 4350 50  0001 C CNN
F 3 "" H 4200 4350 50  0001 C CNN
	1    4200 4350
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR011
U 1 1 59E231B3
P 4200 4450
F 0 "#PWR011" H 4200 4450 30  0001 C CNN
F 1 "GND" H 4200 4380 30  0001 C CNN
F 2 "" H 4200 4450 60  0000 C CNN
F 3 "" H 4200 4450 60  0000 C CNN
	1    4200 4450
	1    0    0    -1  
$EndComp
Wire Wire Line
	4200 4350 4200 4450
$Comp
L Q_NMOS_GSD Q2
U 1 1 59E3F4ED
P 6450 4600
F 0 "Q2" H 6650 4650 50  0000 L CNN
F 1 "Q_NMOS_GSD" H 6650 4550 50  0001 L CNN
F 2 "TO_SOT_Packages_SMD:SOT-23" H 6650 4700 50  0001 C CNN
F 3 "" H 6450 4600 50  0000 C CNN
	1    6450 4600
	1    0    0    -1  
$EndComp
$EndSCHEMATC
