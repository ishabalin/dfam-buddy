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

private:
  bool steps[STEPS];
  uint8_t numStepsOn;
};

struct State {
  // transport
  bool started = false;
  // steps
  Steps permSteps = Steps(true);
  Steps tempSteps = Steps(false);
  // actual step pointer, mirroring perm or temp step depending on current mode
  int8_t activeStep = -1;
  // current DFAM step
  int8_t dfamStep = 0;
  //   // global step counter, that doesn't wrap around
  //   int8_t globalStep = -1;
};
