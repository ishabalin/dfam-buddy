#pragma once

#ifdef NATIVE_PLATFORM
#include <stdlib.h>
#else
#include <Arduino.h>
#endif // NATIVE_PLATFORM

#define STEPS 8

class Steps {
public:
  Steps(bool on) : steps{on, on, on, on, on, on, on, on}, numStepsOn(on ? STEPS : 0) {}

  int8_t activeStep = -1;

  void toggleStep(uint8_t);
  void setStep(uint8_t, bool);
  bool getStep(uint8_t);
  void advance();
  bool active() { return numStepsOn > 0; }

private:
  bool steps[STEPS];
  uint8_t numStepsOn;
};

enum SyncMode {
  INTERNAL_CLOCK,
  EXTERNAL_CLOCK,
  EXTERNAL_CLOCK_FORCE_STOP,
};

struct State {
  // transport
  bool started = false;
  SyncMode syncMode = INTERNAL_CLOCK;
  // steps
  Steps permSteps = Steps(true);
  Steps tempSteps = Steps(false);
  // actual step pointer, mirroring perm or temp step depending on current mode
  int8_t activeStep = -1;
  // current DFAM step
  int8_t dfamStep = 0;
};
