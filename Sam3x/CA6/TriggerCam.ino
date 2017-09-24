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

void triggerCamerasPhase2() {
  g_ctx.curCamElement = 0;

  // Trigger anything in this sorted list with 0 time
  if (g_ctx.camTimerElements[g_ctx.curCamElement].timeOffset == 0) {
    triggerCamerasPhase3();
  }
  else {
    g_ctx.camTimer.start(triggerCamerasPhase3, g_ctx.camTimerElements[g_ctx.curCamElement].timeOffset, true);
  }
}

void triggerCamerasPhase3() {
  do {
    CamTimerElement x = g_ctx.camTimerElements[g_ctx.curCamElement];
    if (x.sequencerVal & g_ctx.curSequencerBit) {
      CAU::digitalWrite(g_ctx.camPins[x.camOffset].focusPin, x.focusSig);
      CAU::digitalWrite(g_ctx.camPins[x.camOffset].shutterPin, x.shutterSig);
    }

    if (++g_ctx.curCamElement == NUM_CAM_TIMER_ELEMENTS) {
      triggerCamerasPhase4();
      return;
    }
  } while(g_ctx.camTimerElements[g_ctx.curCamElement].timeOffset == 0);
  g_ctx.camTimer.start(triggerCamerasPhase3, g_ctx.camTimerElements[g_ctx.curCamElement].timeOffset, true);
}

void triggerCamerasPhase4() {
  // Shift sequence bit one postion left with wrap around
  if (g_ctx.sequencerMask) {
    uint8_t b = g_ctx.curSequencerBit;
    do {  // Shift the bit until we will trigger at least one camera/flash by checking sequencer mask
      b = (b << 1) | (b >> 7); //Shift bit left with wrap around
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
  
  for(uint8_t i=0; i<NUM_CAMERAS; ++i) {
    uint64_t t0 = CATickTimer::convertTimeToTicks(g_ctx.camSettings[i].getDelaySeconds(), g_ctx.camSettings[i].getDelayNanoseconds());
    uint64_t t1 = CATickTimer::convertTimeToTicks(g_ctx.camSettings[i].getDurationSeconds(), g_ctx.camSettings[i].getDurationNanoseconds());
    uint64_t t2 = CATickTimer::convertTimeToTicks(g_ctx.camSettings[i].getPostDelaySeconds(), g_ctx.camSettings[i].getPostDelayNanoseconds());
    uint8_t mode = g_ctx.camSettings[i].getMode();
    uint8_t focusAfterDelay = (mode == CA_MODE_FLASH) ? LOW : HIGH;
    uint8_t focusAfterDuration = (mode == CA_MODE_PREFOCUS) ? HIGH : LOW;
    uint8_t sequencerVal = g_ctx.camSettings[i].getSequencer();

    sequencerMask |= sequencerVal;
    maxTotalTime = max(t0+t1+t2, maxTotalTime);

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
  }
  g_ctx.sequencerMask = sequencerMask;
  sortCamTiming(g_ctx.camTimerElements, j);
  // Don't need to sort the following because they are all inserted at the end with max time

  for(uint8_t i=0; i<NUM_CAMERAS; ++i) {
    uint8_t mode = g_ctx.camSettings[i].getMode();
    uint8_t focusAtEnd = (mode == CA_MODE_PREFOCUS || mode == CA_MODE_SMART_PREFOCUS) ? HIGH : LOW;
    
    // Set focus/shutter pins at very end of trigger
    g_ctx.camTimerElements[j].timeOffset = maxTotalTime;
    g_ctx.camTimerElements[j].camOffset = i;
    g_ctx.camTimerElements[j].focusSig = focusAtEnd;
    g_ctx.camTimerElements[j].shutterSig = LOW;
    g_ctx.camTimerElements[j++].sequencerVal = g_ctx.camSettings[i].getSequencer();
  }

  biasCamTiming(g_ctx.camTimerElements, NUM_CAM_TIMER_ELEMENTS);

  //CA_LOG("Sorted Camera List\n");
  //for(uint8_t i=0; i<NUM_CAM_TIMER_ELEMENTS; ++i) {
  //  CA_LOG("%d  %"PRId64"\n", g_ctx.camTimerElements[i].camOffset, g_ctx.camTimerElements[i].timeOffset);
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

