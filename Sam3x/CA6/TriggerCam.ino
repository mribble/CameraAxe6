bool camTriggerRunning() {
  return g_ctx.camTriggerRunning;
}

void triggerCameras() {
  if (camTriggerRunning()) {
    return;
  }
  g_ctx.camTriggerRunning = true;
  handleMirrorLockup();

  g_ctx.intervalometerCurRepeats = g_ctx.intervalometerRepeats;
  
  if (g_ctx.intervalometerEnable && (g_ctx.intervalometerStartTime!= 0)) {
    g_ctx.camTimer.start(triggerCamerasPhase2, g_ctx.intervalometerStartTime, true);
  }
  else {
    triggerCamerasPhase2();
  }
}

//#define USE_OLD_TRIGGER_METHOD

void triggerCamerasPhase2() {
  g_ctx.curCamElement = 0;
  g_ctx.curUnifiedCamElement = 0;

  // Trigger anything in this sorted list with 0 time
#ifdef USE_OLD_TRIGGER_METHOD
  if (g_ctx.camTimerElements[g_ctx.curCamElement].timeOffset == 0) {
#else
  if (g_ctx.unifiedCamTimerElements[g_ctx.curUnifiedCamElement].timeOffset == 0) {
#endif
    triggerCamerasPhase3();
  }
  else {
#ifdef USE_OLD_TRIGGER_METHOD
    g_ctx.camTimer.start(triggerCamerasPhase3, g_ctx.camTimerElements[g_ctx.curCamElement].timeOffset, true);
#else
    g_ctx.camTimer.start(triggerCamerasPhase3, g_ctx.unifiedCamTimerElements[g_ctx.curUnifiedCamElement].timeOffset, true);
#endif
  }
}

void triggerCamerasPhase3() {
#ifdef USE_OLD_TRIGGER_METHOD
  uint8_t startElement = g_ctx.curCamElement;
  do {
    CamTimerElement x = g_ctx.camTimerElements[g_ctx.curCamElement];
    if (x.sequencerVal & g_ctx.curSequencerBit) {
      CAU::digitalWrite(g_ctx.camPins[x.camOffset].focusPin, x.focusSig);
      CAU::digitalWrite(g_ctx.camPins[x.camOffset].shutterPin, x.shutterSig);
    }

    if (++g_ctx.curCamElement == g_ctx.numCamTimerElements) {
      triggerCamerasPhase4();
      return;
    }
  } while(g_ctx.camTimerElements[g_ctx.curCamElement].timeOffset == 0);

  g_ctx.camTimer.start(triggerCamerasPhase3, g_ctx.camTimerElements[g_ctx.curCamElement].timeOffset, true);
#else
  uint8_t startElement = g_ctx.curUnifiedCamElement;
  do {
    CamTimerUnifiedElement& x = g_ctx.unifiedCamTimerElements[g_ctx.curUnifiedCamElement];
    REG_PIOA_SODR = x.setMasks[0] & g_ctx.unifiedSequencerMask[g_ctx.sequencerValue][0];    //todo figure out why sequencer not working
    REG_PIOB_SODR = x.setMasks[1] & g_ctx.unifiedSequencerMask[g_ctx.sequencerValue][1];
    REG_PIOD_SODR = x.setMasks[3] & g_ctx.unifiedSequencerMask[g_ctx.sequencerValue][3];
    REG_PIOA_CODR = x.clearMasks[0] & g_ctx.unifiedSequencerMask[g_ctx.sequencerValue][0];
    REG_PIOB_CODR = x.clearMasks[1] & g_ctx.unifiedSequencerMask[g_ctx.sequencerValue][1];
    REG_PIOD_CODR = x.clearMasks[3] & g_ctx.unifiedSequencerMask[g_ctx.sequencerValue][3];

    if (++g_ctx.curUnifiedCamElement == g_ctx.numUnifiedCamTimerElements) {
      triggerCamerasPhase4();
      return;
    }
  } while(g_ctx.unifiedCamTimerElements[g_ctx.curUnifiedCamElement].timeOffset == 0);

  g_ctx.camTimer.start(triggerCamerasPhase3, g_ctx.unifiedCamTimerElements[g_ctx.curUnifiedCamElement].timeOffset, true);
#endif
}

void triggerCamerasPhase4() {
  // Shift sequence bit one postion left with wrap around
  if (g_ctx.sequencerMask) {
    uint8_t b = g_ctx.curSequencerBit;
    do {  // Shift the bit until we will trigger at least one camera/flash by checking sequencer mask
      b = (b << 1) | (b >> 7); //Shift bit left with wrap around
      g_ctx.sequencerValue = (g_ctx.sequencerValue+1 > NUM_SEQUENCER_BITS-1) ? 0 : (g_ctx.sequencerValue+1);
    } while ((b & g_ctx.sequencerMask) == 0);
    g_ctx.curSequencerBit = b;
  }

  if (g_ctx.intervalometerEnable) {
    bool repeat = false;
  
    if (g_ctx.intervalometerRepeats == 0) {
      //repeat forever
      repeat = true;
    }
    else if ((--g_ctx.intervalometerCurRepeats) > 0) {
      // repeat until current repeats is zero
      repeat = true;
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

void handleMirrorLockup() {
  bool anyMirrorLockup = false;
  
  for(uint8_t i=0; i<NUM_CAMERAS; ++i) {
    uint8_t mirrorLockup = g_ctx.camSettings[i].getMirrorLockup();

    if (mirrorLockup) {
      CAU::digitalWrite(g_ctx.camPins[i].focusPin, HIGH);
      CAU::digitalWrite(g_ctx.camPins[i].shutterPin, HIGH);
      anyMirrorLockup = true;
    }
  }

  if (anyMirrorLockup) {
    delay(250);
    for(uint8_t i=0; i<NUM_CAMERAS; ++i) {
      uint8_t mode = g_ctx.camSettings[i].getMode();
      uint8_t focusPin = (mode == CA_MODE_PREFOCUS) ? HIGH : LOW;
      uint8_t mirrorLockup = g_ctx.camSettings[i].getMirrorLockup();
    
      if (mirrorLockup) {
        CAU::digitalWrite(g_ctx.camPins[i].focusPin, focusPin);
        CAU::digitalWrite(g_ctx.camPins[i].shutterPin, LOW);
      }
    }
    delay(250);
  }
}

void initCameraPins() {
  for(uint8_t i=0; i<NUM_CAMERAS; ++i) {
    g_ctx.camPins[i].focusPin = CAU::getCameraPin(i, FOCUS);
    g_ctx.camPins[i].shutterPin = CAU::getCameraPin(i, SHUTTER);
    CAU::pinMode(g_ctx.camPins[i].focusPin, OUTPUT);
    CAU::pinMode(g_ctx.camPins[i].shutterPin, OUTPUT);
  }
}

void setupCamTiming() {
  uint8_t j = 0;
  uint64_t maxTotalTime = 0;
  uint8_t sequencerMask = 0;
  bool processCam[NUM_CAMERAS] = {false, false, false, false, false, false, false, false};
  
  for(uint8_t i=0; i<NUM_CAMERAS; ++i) {
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
      g_ctx.camTimerElements[j].timeOffset = t0;
      g_ctx.camTimerElements[j].camOffset = i;
      g_ctx.camTimerElements[j].focusSig = focusAfterDelay;
      g_ctx.camTimerElements[j].shutterSig = HIGH;
      g_ctx.camTimerElements[j++].sequencerVal = sequencerVal;

      // Set focus/shutter pins right after duration time
      g_ctx.camTimerElements[j].timeOffset = t0+t1;
      g_ctx.camTimerElements[j].camOffset = i;
      g_ctx.camTimerElements[j].focusSig = focusAfterDuration;
      g_ctx.camTimerElements[j].shutterSig = LOW;
      g_ctx.camTimerElements[j++].sequencerVal = sequencerVal;

      processCam[i] = true;
    }
  }
  g_ctx.sequencerMask = sequencerMask;
  sortCamTiming(g_ctx.camTimerElements, j);
  // Don't need to sort the following because they are all inserted at the end with max time

  for(uint8_t i=0; i<NUM_CAMERAS; ++i) {
    if (processCam[i]) {
      uint8_t mode = g_ctx.camSettings[i].getMode();
      uint8_t focusAtEnd = (mode == CA_MODE_PREFOCUS || mode == CA_MODE_SMART_PREFOCUS) ? HIGH : LOW;
      
      // Set focus/shutter pins at very end of trigger
      g_ctx.camTimerElements[j].timeOffset = maxTotalTime;
      g_ctx.camTimerElements[j].camOffset = i;
      g_ctx.camTimerElements[j].focusSig = focusAtEnd;
      g_ctx.camTimerElements[j].shutterSig = LOW;
      g_ctx.camTimerElements[j++].sequencerVal = g_ctx.camSettings[i].getSequencer();
    }
  }

  g_ctx.numCamTimerElements = j;

  biasCamTiming(g_ctx.camTimerElements, g_ctx.numCamTimerElements);
  generateUnifiedElements();

  //CA_LOG("Sorted Camera List\n");
  //for(uint8_t i=0; i<g_ctx.numCamTimerElements; ++i) {
  //  CA_LOG("%d  %"PRId64"\n", g_ctx.camTimerElements[i].camOffset, g_ctx.camTimerElements[i].timeOffset);
  //}
}

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

void generateUnifiedElements() {
  uint8_t i, j;
  CamTimerElement *el = g_ctx.camTimerElements;
  CamTimerUnifiedElement *uEl = g_ctx.unifiedCamTimerElements;
  uint8_t numEl = g_ctx.numCamTimerElements;
  int8_t curUEl = -1;

  memset(uEl, 0, sizeof(g_ctx.unifiedCamTimerElements));
  
  // Create a UnifiedElement with  each time generate an offsetTime, setMask (per port), clearMask (per port)
  // This is done by or-ing all the clear bits and all the set bits together
  // The basic idea here is we build masks for all the camera events happening at the same time and then set all those bits on the micro's ports in a single write instead of doing one bit at a time
  for(i=0; i<numEl; ++i) {
    
    if ((i==0) || (el[i].timeOffset!=0)) {
      // Increment the unified element in these cases because time has moved forward
      curUEl++;
      uEl[curUEl].timeOffset = el[i].timeOffset;
    }
    calcMasks(uEl[curUEl].setMasks, uEl[curUEl].clearMasks, el[i].focusSig, el[i].shutterSig, el[i].camOffset);
  }

  g_ctx.numUnifiedCamTimerElements = curUEl+1;
  g_ctx.curUnifiedCamElement = 0;

  //CA_LOG("Unified Camera List\n");
  //for(uint8_t i=0; i<g_ctx.numUnifiedCamTimerElements; ++i) {
  //  CA_LOG("%"PRId64" Set%#010x,%#010x,%#010x,%#010x Clear%#010x,%#010x,%#010x,%#010x\n", g_ctx.unifiedCamTimerElements[i].timeOffset,
  //    g_ctx.unifiedCamTimerElements[i].setMasks[0], g_ctx.unifiedCamTimerElements[i].setMasks[1], g_ctx.unifiedCamTimerElements[i].setMasks[2], g_ctx.unifiedCamTimerElements[i].setMasks[3],
  //    g_ctx.unifiedCamTimerElements[i].clearMasks[0], g_ctx.unifiedCamTimerElements[i].clearMasks[1], g_ctx.unifiedCamTimerElements[i].clearMasks[2], g_ctx.unifiedCamTimerElements[i].clearMasks[3]);
  //}

  memset(g_ctx.unifiedSequencerMask, 0, sizeof(g_ctx.unifiedSequencerMask));

  // Create a sequencer mask per sequencer bit (8 of them) which can be and-ed with the setMask/clearMask when trigger happens with the approriate sequencer value
  // The sequencer mask is basically any possible bit that could be set or cleared, but specifically excludes bits not allowed due to the sequencer settings from a camera port
  // This is done by walking through each each sequencer bit for each camera and or-ing its port bits into the sequencer mask for that sequencer bit
  for (i=0; i<NUM_SEQUENCER_BITS; ++i) {
    for(j=0; j<NUM_CAMERAS; ++j) {
      
      uint8_t seq = g_ctx.camSettings[j].getSequencer();
      
      // This check removes bits that shouldn't be in this sequencer mask
      if (seq & (1<<i)) {
        hwPortPin focus = CAU::getCameraPin(j, FOCUS);
        g_ctx.unifiedSequencerMask[i][focus.port] |= (1<<focus.pin);

        hwPortPin shutter = CAU::getCameraPin(j, SHUTTER);
        g_ctx.unifiedSequencerMask[i][shutter.port] |= (1<<shutter.pin);
      }
    }
  }

  //CA_LOG("Unified Sequencer masks\n");
  //for(uint8_t i=0; i<NUM_SEQUENCER_BITS; ++i) {
  //  CA_LOG("%#010x,%#010x,%#010x,%#010x\n", g_ctx.unifiedSequencerMask[i][0], g_ctx.unifiedSequencerMask[i][1], g_ctx.unifiedSequencerMask[i][2], g_ctx.unifiedSequencerMask[i][3]);
  //}

}

void sortCamTiming(CamTimerElement* arry, int8_t sz) {
  int8_t i = 1;

  // Insertion Sort
  while (i < sz) {
    CamTimerElement x = arry[i];
    int8_t j = i-1;
    while ((j >= 0) && (arry[j].timeOffset > x.timeOffset)) {
      arry[j+1] = arry[j];
      --j;
    }
    arry[j+1] = x;
    ++i;
  }
}

void biasCamTiming(CamTimerElement* arry, int8_t sz) {
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

