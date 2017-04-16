////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Lightning Menu Copyright 2017 Dan Lenardon
 This work is licensed under the Creative Commons Attribution-ShareAlike 4.0 (CC BY-SA 4.0) International License.
 To view a copy of this license, visit https://creativecommons.org/licenses/by-sa/4.0
	 Release Notes
	 First version for CA6
*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////
// Function: LightningMenu - A specific threshold algorithm for Lightning.
// Uses a light sensor module in Module 0 and triggers all enabled Camera Ports
// Parameters:
//   None
// Returns:
//   None
////////////////////////////////////////

typedef struct {
	uint32_t triggerDiffThreshold;
	hwPortPin ppLight;
	uint16_t sensorVal = 0;
	uint32_t timeToDisplayMS;
	uint32_t updateRefPeriodMS = 200;
	uint16_t referenceSensorVal = 0;
	uint16_t triggerCount = 0;
	// variables for the Advanced Photo Display - last 5 strike details  
	int16_t strk_num[5] = { 0, 0, 0, 0, 0};
	int16_t ref[6] = { 0, 0, 0, 0, 0, 0};  // extra index for temp storage of ref at beginning of strike
	int16_t peak[5] = { 0, 0, 0, 0, 0};
	uint32_t strk_duration_us[5] = { 0, 0, 0, 0, 0};
	int16_t peakOfStrike = 0;
	uint32_t StrikeStartTimeUs;  // used for strike duration display
	uint32_t StrikeStartTimeMS; // used for 2-second max strike duration to prevent lock-up with ambient or sensitivity changes

} MenuLightningData;

MenuLightningData gLightningData;

void MenuLightning_Info() {
	g_ctx.packetHelper.writePacketMenuList(3, 2, 0x1, 0, 0x0, 0, 0x0, 0, 0x0, 0, 0x0, 0, 0x0, "Lightning Menu");
}

void MenuLightning_MenuInit() {
  const uint8_t sDataMenu[] PROGMEM = {
    17,0,PID_MENU_HEADER,2,0,1,0,'L','i','g','h','t','n','i','n','g',0,  // MENU_HEADER 2 1 "Lightning"
    49,0,PID_EDIT_NUMBER,0,0,4,2,0,0,0,254,15,0,0,40,0,0,0,'T','r','i','g','g','e','r',' ','D','i','f','f','e','r','e','n','c','e',' ','T','h','r','e','s','h','o','l','d',':',' ',0,  // EDIT_NUMBER 0 4 0 2 4094 40 "Trigger Difference Threshold: "  **gClientHostId_0**
    49,0,PID_EDIT_NUMBER,1,0,4,0,0,0,0,15,39,0,0,200,0,0,0,'U','p','d','a','t','e',' ','R','e','f','e','r','e','n','c','e',' ','P','e','r','i','o','d',' ','(','m','s',')',':',' ',0,  // EDIT_NUMBER 0 4 0 0 9999 200 "Update Reference Period (ms): "  **gClientHostId_1**
    38,0,PID_TEXT_DYNAMIC,2,0,'-','-','C','u','r','r','e','n','t',' ','S','e','n','s','o','r',' ','V','a','l','u','e',' ','i','s',':',' ',0,'?','?','?','?',0,  // TEXT_DYNAMIC 0 "--Current Sensor Value is: " "????"  **gClientHostId_2**
    3,0,PID_SCRIPT_END,  // SCRIPT_END
  };  // Total Bytes = 156
  g_ctx.packetHelper.writeMenu(sDataMenu, 156);

//	caAllPortsLow();  This screws up packet processing!!
	CAU::initializeAnalog();
	gLightningData.ppLight = CAU::getModulePin(0, 0);
	CAU::pinMode(gLightningData.ppLight, ANALOG_INPUT);
	gLightningData.timeToDisplayMS = millis();
  gLightningData.sensorVal = CAU::analogRead(gLightningData.ppLight);
  gLightningData.referenceSensorVal = gLightningData.sensorVal;
}

void MenuLightning_PhotoInit() {
  const uint8_t sDataActive[] PROGMEM = {
    24,0,PID_MENU_HEADER,2,0,1,0,'L','i','g','h','t','n','i','n','g',' ','A','c','t','i','v','e',0,  // MENU_HEADER 2 1 "Lightning Active"
    27,0,PID_TEXT_DYNAMIC,0,0,'R','e','f','e','r','e','n','c','e',' ','B','a','s','e',':',' ',0,'?','?','?','?',0,  // TEXT_DYNAMIC 0 "Reference Base: " "????"  **gClientHostId_0**
    38,0,PID_TEXT_DYNAMIC,1,0,'-','-','C','u','r','r','e','n','t',' ','S','e','n','s','o','r',' ','V','a','l','u','e',' ','i','s',':',' ',0,'?','?','?','?',0,  // TEXT_DYNAMIC 0 "--Current Sensor Value is: " "????"  **gClientHostId_1**
    26,0,PID_TEXT_DYNAMIC,2,0,' ',' ',' ','D','i','f','f','e','r','e','n','c','e',':',' ',0,'?','?','?','?',0,  // TEXT_DYNAMIC 0 "   Difference: " "????"  **gClientHostId_2**
    29,0,PID_TEXT_DYNAMIC,3,0,' ',' ',' ','T','r','i','g','g','e','r',' ','V','a','l','u','e',':',' ',0,'?','?','?','?',0,  // TEXT_DYNAMIC 0 "   Trigger Value: " "????"  **gClientHostId_3**
    40,0,PID_TEXT_DYNAMIC,4,0,' ','U','p','d','a','t','e',' ','R','e','f','e','r','e','n','c','e',' ','P','e','r','i','o','d',' ','(','m','s',')',0,'?','?','?','?',0,  // TEXT_DYNAMIC 0 " Update Reference Period (ms)" "????"  **gClientHostId_4**
    27,0,PID_TEXT_DYNAMIC,5,0,' ','T','r','i','g','g','e','r',' ','C','o','u','n','t',':',' ',0,'?','?','?','?',0,  // TEXT_DYNAMIC 0 " Trigger Count: " "????"  **gClientHostId_5**
    36,0,PID_TEXT_STATIC,'-','-','-','D','e','t','a','i','l','s',' ','F','o','r',' ','L','a','s','t',' ','5',' ','S','t','r','i','k','e','s','-','-','-',0,  // TEXT_STATIC "---Details For Last 5 Strikes---"
    33,0,PID_TEXT_STATIC,'S','t','r','i','k','e',' ','R','e','f','-',' ','P','e','a','k',' ','D','u','r','a','t','i','o','n','(','m','s',')',0,  // TEXT_STATIC "Strike Ref- Peak Duration(ms)"
    12,0,PID_TEXT_DYNAMIC,6,0,'L',' ',' ',' ',0,'?',0,  // TEXT_DYNAMIC 0 "L   " "?"  **gClientHostId_6**
    12,0,PID_TEXT_DYNAMIC,7,0,'L','-','1',' ',0,'?',0,  // TEXT_DYNAMIC 0 "L-1 " "?"  **gClientHostId_7**
    12,0,PID_TEXT_DYNAMIC,8,0,'L','-','2',' ',0,'?',0,  // TEXT_DYNAMIC 0 "L-2 " "?"  **gClientHostId_8**
    12,0,PID_TEXT_DYNAMIC,9,0,'L','-','3',' ',0,'?',0,  // TEXT_DYNAMIC 0 "L-3 " "?"  **gClientHostId_9**
    12,0,PID_TEXT_DYNAMIC,10,0,'L','-','4',' ',0,'?',0,  // TEXT_DYNAMIC 0 "L-4 " "?"  **gClientHostId_10**
    3,0,PID_SCRIPT_END,  // SCRIPT_END
  };  // Total Bytes = 343
  g_ctx.packetHelper.writeMenu(sDataActive, 343);

	/* shouldn't have to do these twice, we already did them in Menu_Init
	CAU::initializeAnalog();
	gLightningData.ppLight = CAU::getModulePin(0, 0);
	CAU::pinMode(gLightningData.ppLight, ANALOG_INPUT);
  */
	gLightningData.timeToDisplayMS = millis();
  gLightningData.sensorVal = CAU::analogRead(gLightningData.ppLight);
  gLightningData.referenceSensorVal = gLightningData.sensorVal;
}

void MenuLightning_MenuRun() {
#define DISPLAYFREQMS 1000  // Update the display variables every DISPLAYFREQMS milliseconds
	uint32_t curTimeMS = millis();

	// Handle incoming packets
	CAPacketElement *packet = processIncomingPacket();
	packet = incomingPacketCheckEditNumber(packet, 0, gLightningData.triggerDiffThreshold);
	packet = incomingPacketCheckEditNumber(packet, 1, gLightningData.updateRefPeriodMS);
	incomingPacketFinish(packet);

	// Handle outgoing packets
	if ((curTimeMS >= gLightningData.timeToDisplayMS) && (curTimeMS - gLightningData.timeToDisplayMS < DISPLAYFREQMS * 1000)) { // Handles wraparounds
		gLightningData.sensorVal = CAU::analogRead(gLightningData.ppLight);
		g_ctx.packetHelper.writePacketTextDynamic(2, 0, String(gLightningData.sensorVal).c_str());
		gLightningData.timeToDisplayMS = curTimeMS + DISPLAYFREQMS;
	}

// Still may need to deal with Camera settings, e.g. want to default to Focus active, no delay, etc.

}

void MenuLightning_PhotoRun() {
	uint32_t curTimeMS = millis();
	gLightningData.sensorVal = CAU::analogRead(gLightningData.ppLight);

	// Handle incoming packets
	CAPacketElement *packet = processIncomingPacket();
	incomingPacketFinish(packet);


	// Handle outgoing packets
	if ((curTimeMS >= gLightningData.timeToDisplayMS) && (curTimeMS - gLightningData.timeToDisplayMS < DISPLAYFREQMS * 1000)) { // Handles wraparounds
    g_ctx.packetHelper.writePacketTextDynamic(0, 0, String(gLightningData.referenceSensorVal).c_str());
    g_ctx.packetHelper.writePacketTextDynamic(1, 0, String(gLightningData.sensorVal).c_str());
    g_ctx.packetHelper.writePacketTextDynamic(2, 0, String(gLightningData.referenceSensorVal - gLightningData.sensorVal).c_str());
    g_ctx.packetHelper.writePacketTextDynamic(3, 0, String(gLightningData.triggerDiffThreshold).c_str());
    g_ctx.packetHelper.writePacketTextDynamic(4, 0, String(gLightningData.updateRefPeriodMS).c_str());
    g_ctx.packetHelper.writePacketTextDynamic(5, 0, String(gLightningData.triggerCount).c_str());
    gLightningData.timeToDisplayMS = curTimeMS + DISPLAYFREQMS;
    g_ctx.packetHelper.writePacketTextDynamic(6, 0, "Last strike details");
    g_ctx.packetHelper.writePacketTextDynamic(7, 0, "Last-1 strike details");
    g_ctx.packetHelper.writePacketTextDynamic(8, 0, "Last-2 strike details");
    g_ctx.packetHelper.writePacketTextDynamic(9, 0, "Last-3 strike details");
    g_ctx.packetHelper.writePacketTextDynamic(10, 0, "Last-4 strike details");
  }

	// Handle triggering
	uint8_t trigger = false;
	if (trigger) {
		triggerCameras();
		CA_LOG("Trigger\n");
	}

}









//---------------------------------------------------------------------------------------//
/*
#include <CAStd.h>
#include <CAstd_sam3X.h>
#include <CAUtility.h>
#include <CAEeprom.h> // maybe not needed if we don't check module type IDs
#include <CAPacket.h>

// function declarations


// *** Global Constants for Lightning Menu and Functions
const int16_t workingMaxSensorVal = 4015; // light sensors saturate before reaching max of 4096 - measured for Vishay TEPT4400

// void lightningMenu()
void setup() // Lightning Menu
{
	String warnTooHi = "      ";
 
	caAllPortsLow();
	CAU::initializeAnalog();
	hwPortPin ppLight = CAU::getModulePin(0, 0); //plug sensor into Module 0 RJ-45 pin 2 MOD0_AN1
	CAU::pinMode(ppLight, ANALOG_INPUT);
	
	// Read the current value for display with menu items
	gLightningData.sensorVal = CAU::analogRead(ppLight);
	
	//  Get input variables from Android interface
	// All these variables will need to be passed by reference
	getLTGMenuSettings(EEPROM_LTG_SENSOR_TRIG_VAL, gLightningData.sensorVal, gLightningData.updateRefPeriodMS,
		EEPROM_LTG_PHOTO_MENU_SIMPLE_ADV, EEPROM_LTG_NUM_CAM_PORTS, EEPROM_LTG_DEVICE_PREFOCUS[], EEPROM_LTG_DEVICE_CYCLE[]);

// Do the Too High warning in the Android app
	if( (gLightningData.sensorVal + gLightningData.triggerDiffThreshold) >= workingMaxSensorVal)
		warnTooHi = "Too Hi";
	else
		warnTooHi = " ";

}

////////////////////////////////////////
// Function: lightningFunc - Handles camera/flash triggering for lightning similar to the advancedSensorFunc
// Parameters:
//   None
// Returns:
//   None
////////////////////////////////////////

void lightningFunc()
{
#define DISPLAYFREQMS 1000  // Update the display variables every DISPLAYFREQMS milliseconds
	int16_t referenceSensorVal = -1;
	int16_t currentDif = 0;
	uint32_t ReferenceUpdateTimeMS = 0;
	int16_t triggerCount = 0;
	boolean inStrikeCycle = false;        // logical indicating that we are in a strike cycle
	uint32_t shutterHoldTimeMS[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	uint32_t curTimeMS;
	boolean TriggerTooHigh = false;
	int16_t refAtStrike = 0;
	hwPortPin ppLight;
	hwPortPin ppCamShutter[8];
	hwPortPin ppCamFocus[8];
	int d;
	
// variables for the Advanced Photo Display - last 4 strike details  
	int16_t strk_num[4] = { 0, 0, 0, 0 };
	int16_t ref[5] = { 0, 0, 0, 0, 0 };  // extra index for temp storage of ref at beginning of strike
	int16_t peak[4] = { 0, 0, 0, 0 };
	uint32_t strk_duration_us[4] = { 0, 0, 0, 0 };
	int16_t peakOfStrike = 0;
	uint32_t StrikeStartTimeUs;  // used for strike duration display
	uint32_t StrikeStartTimeMS; // used for 2-second max strike duration to prevent lock-up with ambient or sensitivity changes
	
	// eepromDeviceCycles units are tenths????? of a second
	int16_t eepromDeviceCycles[8**************] = { 
		eepromReadInt(EEPROM_LTG_DEVICE_CYCLE1),       eepromReadInt(EEPROM_LTG_DEVICE_CYCLE2)     };
	int16_t eepromDevicePrefocuses[8******************] = { 
		eepromReadInt(EEPROM_LTG_DEVICE_PREFOCUS1),    eepromReadInt(EEPROM_LTG_DEVICE_PREFOCUS2)     };
	int16_t eepromPhotoMenuSimpleAdv = eepromReadInt(EEPROM_LTG_PHOTO_MENU_SIMPLE_ADV);

	extern void  LTGdisplayDetailsOnLCD ( int eepromPhotoMenuSimpleAdv, int *referenceSensorVal, int *sensorVal, int *currentDif,
				boolean TriggerTooHigh, int *gLightningData.triggerDiffThreshold, int *gLightningData.updateRefPeriodMS, int *triggerCount,
				int *peakOfStrike, int *strk_num, int *ref, int *peak, unsigned long *strk_duration_us); // added by DML1333 for compatibility with IDE 1.6.6+
				
//  attachInterrupt(0, activeButtonISR, LOW);

	// Set up ports
	ppLight = CAU::getModulePin(0, 0); //plug sensor into Module 0 RJ-45 pin 2 MOD0_AN1
	for(d = 0; d < EEPROM_LTG_NUM_CAM_PORTS; d++) {
		ppCamShutter[d] = CAU::getCameraPin(d, SHUTTER);  // Cam0 Shutter pin
		CAU::pinMode(ppCamShutter[d], OUTPUT);
		ppCamFocus[d] = CAU::getCameraPin(d, FOCUS);  // Cam0 Focus pin
		CAU::pinMode(ppCamFocus[d], OUTPUT);
	}
	
	// Initialize Prefocus and Shutter for Cam/Flash ports
	for(d = 0; d < EEPROM_LTG_NUM_CAM_PORTS; d++) {
		CAU::digitalWrite(ppCamShutter[d], LOW); 
		if (eepromDevicePrefocuses[d])
			CAU::digitalWrite(ppCamFocus[d], HIGH);
		else
			CAU::digitalWrite(ppCamFocus[d], LOW);
	}
	
	triggerCount = 0;
	referenceSensorVal = CAU::analogRead(ppLight);  // initialize reference base
	curTimeMS = millis();  // initialize the current time to start Reference and Display timers
	ReferenceUpdateTimeMS = curTimeMS + (uint32_t) gLightningData.updateRefPeriodMS ;  // initialize the update timer
	gLightningData.timeToDisplayMS = curTimeMS + DISPLAYFREQMS; // Update the display only once per second because it takes ~50-70 ms to do
	
	while (g_menuMode == MENUMODE_PHOTO)
	{
		// Loop checking for a strike (curval - ref > trigger)
		while ( !inStrikeCycle && g_menuMode == MENUMODE_PHOTO)
		{
			curTimeMS = millis();  // capture the current time
			gLightningData.sensorVal = CAU::analogRead(ppLight);
			currentDif = gLightningData.sensorVal - referenceSensorVal;
			if (currentDif > gLightningData.triggerDiffThreshold)
			{
			// Begin a new strike -- strike lasts until sensor1 goes back below threshold or 2 seconds (bail out)
				inStrikeCycle = true;
				//Trigger Cameras
				for(d = 0; d < EEPROM_LTG_NUM_CAM_PORTS; d++) {
					CAU::digitalWrite(ppCamFocus[d], HIGH);
					CAU::digitalWrite(ppCamShutter[d], HIGH); 
				}
				triggerCount = triggerCount + 1;
				peakOfStrike = gLightningData.sensorVal; // initialize the peak reading for this strike
				StrikeStartTimeUs = micros();  // initialize the strike start for duration measure
				StrikeStartTimeMS = curTimeMS;  // start clock for 2-second max check        refAtStrike = referenceSensorVal;  // save reference value at beginning of strike in index 4
				//Set up Cycle Times for Cameras
				for ( d = 0; d < EEPROM_LTG_NUM_CAM_PORTS; d++ )
				{
					if( eepromDeviceCycles[d] != 0 )
						shutterHoldTimeMS[d] = curTimeMS + (uint32_t) eepromDeviceCycles[d]*100; //eeprom value is integer tenths of sec.
					else
						shutterHoldTimeMS[d] = curTimeMS + g_minShutterHoldMs; // min hold time to activate shutter
						// known issue: If Prefocus is not set, some cameras will sometimes not trigger unless the pins are held high for >=150MS
				}
				break;
			}
			
			// Not yet in a strike event
			// Is it time to update the Reference Value?
			if( curTimeMS >= ReferenceUpdateTimeMS )
			{
				referenceSensorVal = gLightningData.sensorVal;          // Update the threshold reference base value to current value 
				ReferenceUpdateTimeMS = curTimeMS + (uint32_t) gLightningData.updateRefPeriodMS;  // Update Timer 
				if( (referenceSensorVal + gLightningData.triggerDiffThreshold) >= workingMaxSensorVal)
					// At top of Sensor range - trigVal too high - can't trigger
					TriggerTooHigh = true;  // Display a warning warning, but update ReferenceValue anyway, else continual trigger
				else
					TriggerTooHigh = false;
			}

			//Is it time to display current values?
			if( curTimeMS >= gLightningData.timeToDisplayMS)
			{
				LTGdisplayCurrentValsToAndroid( eepromPhotoMenuSimpleAdv, &gLightningData.sensorVal, &referenceSensorVal, &currentDif, &TriggerTooHigh, &triggerCount);
				gLightningData.timeToDisplayMS = curTimeMS + DISPLAYFREQMS;
			}
		}  // End of loop looking for start of a strike
		
		// Begin loop looking for end of strike and handling DeviceCycles
		while ( inStrikeCycle && g_menuMode == MENUMODE_PHOTO)
		{
			gLightningData.sensorVal = CAU::analogRead(ppLight);
			peakOfStrike = max( peakOfStrike, gLightningData.sensorVal);
			currentDif = gLightningData.sensorVal - referenceSensorVal;
			// Strike is done if current value is back below threshold or 2 seconds max to prevent lockups
			if (currentDif <= gLightningData.triggerDiffThreshold)
			// Lightning may often have multiple short flashes/pulses, so this might just be the end of the first pulse
			{
				inStrikeCycle = false;
				break;
			}
			curTimeMS = millis();
			if (curTimeMS > (StrikeStartTimeMS + 2000))
			{
				// Sensor value has been above threshold for >2 seconds - must be an ambient or sensor sensitivity change
				// Just end the strike and reset the Reference level
				inStrikeCycle = false;
				referenceSensorVal = gLightningData.sensorVal;          // Update the threshold reference base value to current value 
				ReferenceUpdateTimeMS = curTimeMS + (unsigned long) gLightningData.updateRefPeriodMS; // reset the Ref Update clock
				break;
			}
				
			// check each device for completion of cycle time
			for ( d = 0; d < EEPROM_LTG_NUM_CAM_PORTS; d++ )
			{
				if( shutterHoldTimeMS[d] && (shutterHoldTimeMS[d] < curTimeMS) )
				{
					shutterHoldTimeMS[d] = 0;
					// Reinitialize Prefocus and Shutter for Cam/Flash ports 0 & 1
					CAU::digitalWrite(ppCamShutter[d], LOW); 
					if (eepromDevicePrefocuses[d])
						CAU::digitalWrite(ppCamFocus[d], HIGH);
					else
						CAU::digitalWrite(ppCamFocus[d], LOW);
				}
			}
		}
		
		// Strike cycle just finished
		// Move the prior 3 strike details down in the arrays and store the details for this strike that just completed
		for (int irow = 2; irow >= 0; --irow)
		{
			strk_num[irow + 1] = strk_num[irow];
			ref[irow + 1] = ref[irow];
			peak[irow + 1] = peak[irow];
			strk_duration_us[irow + 1] = strk_duration_us[irow];
		}
		strk_num[0] = triggerCount;
		ref[0] = refAtStrike;   // use temp copy of reference value set at beginning of strike because it could have changed
		peak[0] = peakOfStrike;
		strk_duration_us[0] = micros() - StrikeStartTimeUs + 14; // Add 14 microseconds as 1/2 the estimated time between AnalogReads in strike detection
//    LTGdisplayStrikeDetailsToAndroid( &triggerCount, &refAtStrike, &peakOfStrike, &strk_duration_us);
		// Loop until DeviceCycles (Bulb TImer) is completed for both devices
		while ( shutterHoldTimeMS[0] || shutterHoldTimeMS[1] *********)
		{
			curTimeMS = millis();  // capture the current time
			gLightningData.sensorVal = CAU::analogRead(ppLight);  // Just to keep the current value up to date 
			currentDif = gLightningData.sensorVal - referenceSensorVal;
			// Check if light value has gone back up over threshold -- secondary pulses/flashes
			if (currentDif > gLightningData.triggerDiffThreshold)
			{
				strk_duration_us[0] = micros() - StrikeStartTimeUs + 14; // Extend the strike duration to cover secondary flashes during deviceCycles
				peak[0] = max( peak[0] , gLightningData.sensorVal);  // Update the peak if needed
			}

			// check each device for completion of cycle time
			for ( d = 0; d < EEPROM_LTG_NUM_CAM_PORTS; d++ )
			{
				if( shutterHoldTimeMS[d] && (shutterHoldTimeMS[d] < curTimeMS) )
				{
					shutterHoldTimeMS[d] = 0;
					// Reinitialize Prefocus and Shutter for Cam/Flash ports 0 & 1
					CAU::digitalWrite(ppCamShutter[d], LOW); 
					if (eepromDevicePrefocuses[d])
						CAU::digitalWrite(ppCamFocus[d], HIGH);
					else
						CAU::digitalWrite(ppCamFocus[d], LOW);
				}
			}
			delay( 1 ); // Since we are waiting for the BulbSec timer, no need to go any faster than 1 ms
		}
		// Recheck if current value is below threshold and if so reset Reference
		gLightningData.sensorVal = CAU::analogRead(ppLight);
		currentDif = gLightningData.sensorVal - referenceSensorVal;
		if (currentDif < gLightningData.triggerDiffThreshold)
		{
			referenceSensorVal = gLightningData.sensorVal;          // Update the threshold reference base value to current value 
			ReferenceUpdateTimeMS = curTimeMS + (unsigned long) gLightningData.updateRefPeriodMS; // reset the Ref Update clock
		}
		// If still above threshold, keep old Reference to allow a retrigger to capture more secondary flashes / pulses

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	//
	// Out of PHOTO mode
	//
	// clear camera/flash back to default (especially reset camera prefocus)
	for(d = 0; d < EEPROM_LTG_NUM_CAM_PORTS; d++) {
		CAU::digitalWrite(ppCamShutter[d], LOW); 
		CAU::digitalWrite(ppCamFocus[d], LOW);
	}
	peakOfStrike = 0;  // zero out the peak before going back to MENU
	// Now back to MENU mode
	}
}

//
//
void  getLTGMenuSettings(*********EEPROM_LTG_SENSOR_TRIG_VAL, EEPROM_LTG_SENSOR2_TRIG_NULL, gLightningData.updateRefPeriodMS,
		EEPROM_LTG_SENSOR2_TRIG_NULL, EEPROM_LTG_PHOTO_MENU_SIMPLE_ADV, EEPROM_LTG_SENSOR2_TRIG_NULL,
		EEPROM_LTG_DEVICE_PREFOCUS1, EEPROM_LTG_DEVICE_PREFOCUS2, EEPROM_LTG_DEVICE_CYCLE1, EEPROM_LTG_DEVICE_CYCLE2)
		{
		} 


//  LTGdisplayCurrentValsToAndroid
//    Editable Parameters should already be known to the Android device, so only pass the changable values
//
void  LTGdisplayCurrentValsToAndroid( int16_t eepromPhotoMenuSimpleAdv, int16_t *sensorVal, int16_t *referenceSensorVal, int16_t *currentDif,
				boolean *TriggerTooHigh, int16_t *triggerCount)
{
	CAU::log("sensorVal\t%d\treferenceSensorVal\t%d\tcurrentDif\t%d\tTriggerTooHigh\t%d\n", *sensorVal, *referenceSensorVal, *currentDif, *TriggerTooHigh);
	return;
}

//    LTGdisplayStrikeDetailsToAndroid( &triggerCount, &refAtStrike, &peakOfStrike, &strk_duration_us);
//      Editable Parameters should already be known to the Android device, so only pass the changable values

void  LTGdisplayDetailsToAndroid ( int16_t *triggerCount, int16_t *refAtStrike, int16_t *peakOfStrike, uint32_t *strk_duration_us)
{
	CAU::log("triggerCount\t%d\trefAtStrike\t%d\tpeakOfStrike\t%d\tstrk_duration_us\t%d\n", *triggerCount, *refAtStrike,
					*peakOfStrike, *strk_duration_us);
					
	return;
	}

*/