#ifndef TRIGGER_CAM_H
#define TRIGGER_CAM_H

extern void triggerCamerasPhase2();
extern void triggerCamerasPhase3();
extern void triggerCamerasPhase4();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// camTriggerRunning() - Checks to see if we are currently in a trigger
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool camTriggerRunning() {
  return g_ctx.camTriggerRunning;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// triggerCameras() - This is the first of a 4 phase triggering process.  This function handles mirror lockup and the
//  intervalometer start delay if it exists.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void triggerCameras() {
  if (camTriggerRunning()) {
    return;
  }

  g_ctx.camTriggerRunning = true;

  g_ctx.intervalometerCurRepeats = g_ctx.intervalometerRepeats;
  
  if (g_ctx.intervalometerEnable && (g_ctx.intervalometerStartTime!= 0)) {
    g_ctx.camTimer.start(triggerCamerasPhase2, g_ctx.intervalometerStartTime, true);
  }
  else {
    triggerCamerasPhase2();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// triggerCamerasPhase2() - This is the second of a 4 phase triggering process.  This function handles any initial delay
//  before triggering (this keeps the next phase as simple as possible).
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void triggerCamerasPhase2() {
  g_ctx.curCamElement = 0;

  if (g_ctx.numCamElements == 0) {
    CA_LOG("No cameras triggered.\n");
    triggerCamerasPhase4();
  }

  // Trigger anything in this sorted list with 0 time
  if (g_ctx.camElements[g_ctx.curCamElement].timeOffset == 0) {
    triggerCamerasPhase3();
  }
  else {
    g_ctx.camTimer.start(triggerCamerasPhase3, g_ctx.camElements[g_ctx.curCamElement].timeOffset, true);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// triggerCamerasPhase3() - This is the third of a 4 phase triggering process.  This function actually handles the triggering
//  of the cameras.  It walks through an a pre-generated array of camera elements.  Triggers one and then 
//  waits in a timer interrupt until the next camera element needs to be processed.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void triggerCamerasPhase3() {
  CamElement1& x = g_ctx.camElements[g_ctx.curCamElement];
  REG_PIOA_SODR = x.setMasks[0] & g_ctx.seqMask[g_ctx.sequencerValue][0];
  REG_PIOB_SODR = x.setMasks[1] & g_ctx.seqMask[g_ctx.sequencerValue][1];
  REG_PIOD_SODR = x.setMasks[3] & g_ctx.seqMask[g_ctx.sequencerValue][3];
  REG_PIOA_CODR = x.clearMasks[0] & g_ctx.seqMask[g_ctx.sequencerValue][0];
  REG_PIOB_CODR = x.clearMasks[1] & g_ctx.seqMask[g_ctx.sequencerValue][1];
  REG_PIOD_CODR = x.clearMasks[3] & g_ctx.seqMask[g_ctx.sequencerValue][3];

  if (++g_ctx.curCamElement == g_ctx.numCamElements) {
    triggerCamerasPhase4();
    return;
  }

  g_ctx.camTimer.start(triggerCamerasPhase3, g_ctx.camElements[g_ctx.curCamElement].timeOffset, true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// triggerCamerasPhase4() - This is the forth of a 4 phase triggering process.  This function just handles the book keeping
//  needed to move to the next trigger event.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void triggerCamerasPhase4() {
  if (g_ctx.curSeqMask) {
    uint8_t b = g_ctx.curSeqBit;
    
    do {  // Shift the bit until we will trigger at least one camera/flash by checking sequencer mask
      b = (b << 1) | (b >> 7); //Shift bit left with wrap around
      g_ctx.sequencerValue = (g_ctx.sequencerValue+1 > NUM_SEQUENCER_BITS-1) ? 0 : (g_ctx.sequencerValue+1);
    } while ((b & g_ctx.curSeqMask) == 0);
    g_ctx.curSeqBit = b;
  }

  if (g_ctx.intervalometerEnable) {
    bool repeat = false;
  
    if (g_ctx.intervalometerRepeats == 0) {
      repeat = true;  //repeat forever
    }
    else if ((--g_ctx.intervalometerCurRepeats) > 0) {
      repeat = true;  // repeat until current repeats is zero
    }

    if (repeat) {
      if (g_ctx.intervalometerIntervalTime) {
        g_ctx.camTimer.start(triggerCamerasPhase2, g_ctx.intervalometerIntervalTime, true);
      }
      else {
        triggerCamerasPhase2();
      }
      return;
    }
  }
  g_ctx.camTimer.stop();
  g_ctx.camTriggerRunning = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// handleMirrorLockup() - If mirror lockup is enabled this sets shutter and focus pins for a brief time when first
//  entering photo mode
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void handleMirrorLockup() {
  bool anyMirrorLockup = false;
  
  for(uint8_t i=0; i<NUM_CAMERAS; ++i) {
    uint8_t mirrorLockup = g_ctx.camSettings[i].getMirrorLockup();

    if (mirrorLockup) {
      hwPortPin focusPin = CAU::getCameraPin(i, FOCUS);
      hwPortPin shutterPin = CAU::getCameraPin(i, SHUTTER);
      CAU::digitalWrite(focusPin, HIGH);
      CAU::digitalWrite(shutterPin, HIGH);
      anyMirrorLockup = true;
    }
  }

  if (anyMirrorLockup) {
    delay(250);
    for(uint8_t i=0; i<NUM_CAMERAS; ++i) {
      uint8_t mode = g_ctx.camSettings[i].getMode();
      uint8_t focusVal = ((mode == CA_MODE_PREFOCUS) || (mode == CA_MODE_SMART_PREFOCUS)) ? HIGH : LOW;
      uint8_t mirrorLockup = g_ctx.camSettings[i].getMirrorLockup();
    
      if (mirrorLockup) {
        hwPortPin focusPin = CAU::getCameraPin(i, FOCUS);
        hwPortPin shutterPin = CAU::getCameraPin(i, SHUTTER);
        CAU::digitalWrite(focusPin, focusVal);
        CAU::digitalWrite(shutterPin, LOW);
      }
    }
    delay(250);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// startTriggerCameraState() - Set camera ports to correct levels when entering photo mode
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void startTriggerCameraState() {
  g_ctx.camTimer.stop();
  g_ctx.camTriggerRunning = false;
    
  for(uint8_t i=0; i<NUM_CAMERAS; ++i) {
    uint8_t mode = g_ctx.camSettings[i].getMode();
    uint8_t focusVal = ((mode == CA_MODE_PREFOCUS) || (mode == CA_MODE_SMART_PREFOCUS)) ? HIGH : LOW;
    hwPortPin focusPin = CAU::getCameraPin(i, FOCUS);
    hwPortPin shutterPin = CAU::getCameraPin(i, SHUTTER);
    CAU::digitalWrite(focusPin, focusVal);
    CAU::digitalWrite(shutterPin, LOW);
  }  
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// endTriggerCameraState() - Set camera ports to correct levels when exiting photo mode
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void endTriggerCameraState() {
  g_ctx.camTimer.stop();
  g_ctx.camTriggerRunning = false;

  for(uint8_t i=0; i<NUM_CAMERAS; ++i) {
    hwPortPin focusPin = CAU::getCameraPin(i, FOCUS);
    hwPortPin shutterPin = CAU::getCameraPin(i, SHUTTER);
    CAU::digitalWrite(focusPin, LOW);
    CAU::digitalWrite(shutterPin, LOW);
  }  
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// initCameraPins() - If mirror lockup is enabled this sets shutter and focus pins for a brief time when first
//  entering photo mode
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void initCameraPins() {
  for(uint8_t i=0; i<NUM_CAMERAS; ++i) {
    hwPortPin focusPin = CAU::getCameraPin(i, FOCUS);
    hwPortPin shutterPin = CAU::getCameraPin(i, SHUTTER);
    CAU::pinMode(focusPin, OUTPUT);
    CAU::pinMode(shutterPin, OUTPUT);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// sortCamTiming() - Sorts the list of element from smallest time to largest time
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void sortCamTiming(CamElement0* arry, int8_t sz) {
  int8_t i = 1;

  // Insertion Sort
  while (i < sz) {
    CamElement0 x = arry[i];
    int8_t j = i-1;
    while ((j >= 0) && (arry[j].timeOffset > x.timeOffset)) {
      arry[j+1] = arry[j];
      --j;
    }
    arry[j+1] = x;
    ++i;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// sortCamTiming() - Converts timing from a relative to start of trigger to relative to previous element.
//  By making it relative to the previous element it reduces compute time to set system timers during the triggering.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void biasCamTiming(CamElement0* arry, int8_t sz) {
  // Loop through all camera events and find difference in offsets and store those rather than the absolute offset stored before this was called
  // Note this must be called after sorting
  uint64_t x0 = arry[0].timeOffset;
  uint64_t x1;
  for (int8_t i=1; i<sz; ++i) {
    x1 = arry[i].timeOffset - arry[i-1].timeOffset;
    if (x1 == 1) {
      x1 = 0; // Because of the integer divide by clock we get some off by 1 results.  Any interrupt will take way more than 1 clock so skip this
    }
    arry[i-1].timeOffset = x0;
    x0 = x1;
  }
  arry[sz-1].timeOffset = x0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// calcMasks() - Helper function that adds bits to the set/clear register masks based on current focus/shutter values
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void calcMasks(uint32_t *setMasks, uint32_t *clearMasks, uint8_t focusSig, uint8_t shutterSig, uint8_t cam) {
  hwPortPin focus = CAU::getCameraPin(cam, FOCUS);
  if (focusSig) {
    setMasks[focus.port] |= (1<<focus.pin);
  }
  else {
    clearMasks[focus.port] |= (1<<focus.pin);
  }
  
  hwPortPin shutter = CAU::getCameraPin(cam, SHUTTER);
  if (shutterSig) {
    setMasks[shutter.port] |= (1<<shutter.pin);
  }
  else {
    clearMasks[shutter.port] |= (1<<shutter.pin);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// generateUnifiedElements() - This combines all the camElement0 elements that have exact same time into a CamElement1.
//  CamElement1 is just register set/clear masks and a time when to send them to send them to hw.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void generateUnifiedElements(CamElement0* camElements0, int8_t numCamElements0) {
  uint8_t i, j;
  CamElement1 *el = g_ctx.camElements;
  int8_t curEl = -1;

  memset(el, 0, sizeof(g_ctx.camElements));
  
  // Create a UnifiedElement with  each time generate an offsetTime, setMask (per port), clearMask (per port)
  // This is done by or-ing all the clear bits and all the set bits together
  // The basic idea here is we build masks for all the camera events happening at the same time and then set all those bits on the micro's ports in a single write instead of doing one bit at a time
  for(i=0; i<numCamElements0; ++i) {
    
    if ((i==0) || (camElements0[i].timeOffset!=0)) {
      // Increment the unified element in these cases because time has moved forward
      curEl++;
      el[curEl].timeOffset = camElements0[i].timeOffset;
    }
    calcMasks(el[curEl].setMasks, el[curEl].clearMasks, camElements0[i].focusSig, camElements0[i].shutterSig, camElements0[i].camOffset);
  }

  g_ctx.numCamElements = curEl+1;
  g_ctx.curCamElement = 0;

  //CA_LOG("Unified Camera List\n");
  //for(uint8_t i=0; i<g_ctx.numUnifiedCamTimerElements; ++i) {
  //  CA_LOG("%"PRId64" Set%#010x,%#010x,%#010x,%#010x Clear%#010x,%#010x,%#010x,%#010x\n", g_ctx.unifiedCamTimerElements[i].timeOffset,
  //    g_ctx.unifiedCamTimerElements[i].setMasks[0], g_ctx.unifiedCamTimerElements[i].setMasks[1], g_ctx.unifiedCamTimerElements[i].setMasks[2], g_ctx.unifiedCamTimerElements[i].setMasks[3],
  //    g_ctx.unifiedCamTimerElements[i].clearMasks[0], g_ctx.unifiedCamTimerElements[i].clearMasks[1], g_ctx.unifiedCamTimerElements[i].clearMasks[2], g_ctx.unifiedCamTimerElements[i].clearMasks[3]);
  //}

  memset(g_ctx.seqMask, 0, sizeof(g_ctx.seqMask));

  // Create a sequencer mask per sequencer bit (8 of them) which can be and-ed with the setMask/clearMask when trigger happens with the approriate sequencer value
  // The sequencer mask is basically any possible bit that could be set or cleared, but specifically excludes bits not allowed due to the sequencer settings from a camera port
  // This is done by walking through each each sequencer bit for each camera and or-ing its port bits into the sequencer mask for that sequencer bit
  for (i=0; i<NUM_SEQUENCER_BITS; ++i) {
    for(j=0; j<NUM_CAMERAS; ++j) {
      
      uint8_t seq = g_ctx.camSettings[j].getSequencer();
      
      // This check removes bits that shouldn't be in this sequencer mask
      if (seq & (1<<i)) {
        hwPortPin focus = CAU::getCameraPin(j, FOCUS);
        g_ctx.seqMask[i][focus.port] |= (1<<focus.pin);

        hwPortPin shutter = CAU::getCameraPin(j, SHUTTER);
        g_ctx.seqMask[i][shutter.port] |= (1<<shutter.pin);
      }
    }
  }

  //CA_LOG("Unified Sequencer masks\n");
  //for(uint8_t i=0; i<NUM_SEQUENCER_BITS; ++i) {
  //  CA_LOG("%#010x,%#010x,%#010x,%#010x\n", g_ctx.seqMask[i][0], g_ctx.seqMask[i][1], g_ctx.seqMask[i][2], g_ctx.seqMask[i][3]);
  //}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setupCamTiming() - This function is the main function that takes all camera timing values from the UI and converts
//  them into an efficent data structure that can be processed fast and efficently.  This is a multiphase process.
//  In the first phase we convert the UI data into an array of values that eliminates unneeded elements (cameras that are disabled)
//  and then it sorts all these elements.  The second phase we merge all the elements that occur at the exact same time
//  and cache off register masks to make it very quick to process these elements.
//  The whole point of this is that this can run slowly (outside the critical loop).  Then when we are triggering (in the critcal loop
//  we use these optimized data structures to run fast.
//  Requires a list of elements to be sorted and for time to be a relative offset from the previous element.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setupCamTiming() {
  uint8_t i;
  uint8_t j = 0;
  uint64_t maxTotalTime = 0;
  uint8_t sequencerMask = 0;
  bool processCam[NUM_CAMERAS] = {false, false, false, false, false, false, false, false};
  CamElement0 camElements0[NUM_CAM_TIMER_ELEMENTS];
  uint8_t numCamElements0;

  // Phase 1 (converting camera data into CamElement0 elements and throwing out unneeded elements.
  for(i=0; i<NUM_CAMERAS; ++i) {
    uint64_t t0 = CATickTimer::convertTimeToTicks(g_ctx.camSettings[i].getDelaySeconds(), g_ctx.camSettings[i].getDelayNanoseconds());
    uint64_t t1 = CATickTimer::convertTimeToTicks(g_ctx.camSettings[i].getDurationSeconds(), g_ctx.camSettings[i].getDurationNanoseconds());
    uint64_t t2 = CATickTimer::convertTimeToTicks(g_ctx.camSettings[i].getPostDelaySeconds(), g_ctx.camSettings[i].getPostDelayNanoseconds());
    uint64_t t;
    uint8_t mode = g_ctx.camSettings[i].getMode();
    uint8_t focusAfterDelay = (mode == CA_MODE_FLASH) ? LOW : HIGH;
    uint8_t focusAfterDuration = (mode == CA_MODE_PREFOCUS) ? HIGH : LOW;
    uint8_t sequencerVal = g_ctx.camSettings[i].getSequencer();

    sequencerMask |= sequencerVal;
    t = t0+t1+t2;
    maxTotalTime = max(t, maxTotalTime);

    // Don't add to list if all times are zero or if the sequencerVal is 0 since in both those cases nothing is happening
    if ((t!=0) && (sequencerVal!=0)) {

      // Set focus/shutter pins right after delay time
      camElements0[j].timeOffset = t0;
      camElements0[j].camOffset = i;
      camElements0[j].focusSig = focusAfterDelay;
      camElements0[j].shutterSig = HIGH;
      camElements0[j++].sequencerVal = sequencerVal;

      // Set focus/shutter pins right after duration time
      camElements0[j].timeOffset = t0+t1;
      camElements0[j].camOffset = i;
      camElements0[j].focusSig = focusAfterDuration;
      camElements0[j].shutterSig = LOW;
      camElements0[j++].sequencerVal = sequencerVal;

      processCam[i] = true;
    }
  }
  g_ctx.curSeqMask = sequencerMask;
  sortCamTiming(camElements0, j);
  // Don't need to sort the following because they are all inserted at the end with max time

  for(i=0; i<NUM_CAMERAS; ++i) {
    if (processCam[i]) {
      uint8_t mode = g_ctx.camSettings[i].getMode();
      uint8_t focusAtEnd = ((mode == CA_MODE_PREFOCUS) || (mode == CA_MODE_SMART_PREFOCUS)) ? HIGH : LOW;
      
      // Set focus/shutter pins at very end of trigger
      camElements0[j].timeOffset = maxTotalTime;
      camElements0[j].camOffset = i;
      camElements0[j].focusSig = focusAtEnd;
      camElements0[j].shutterSig = LOW;
      camElements0[j++].sequencerVal = g_ctx.camSettings[i].getSequencer();
    }
  }

  numCamElements0 = j;
  
  biasCamTiming(camElements0, numCamElements0);

  // Phase 2
  generateUnifiedElements(camElements0, numCamElements0);

  //CA_LOG("Sorted Camera List\n");
  //for(uint8_t i=0; i<g_ctx.numCamTimerElements; ++i) {
  //  CA_LOG("%d  %"PRId64"\n", g_ctx.camTimerElements[i].camOffset, g_ctx.camTimerElements[i].timeOffset);
  //}
}
#endif //TRIGGER_CAM_H

