#pragma once

#ifdef NATIVE_PLATFORM
#include <stdlib.h>
#else
#include <Arduino.h>
#endif // NATIVE_PLATFORM

#define STEPS 8

class AdvanceDirection {
public:
  virtual int8_t advance(int8_t) = 0;
};

class Forward : public AdvanceDirection {
public:
  int8_t advance(int8_t step) {
    step++;
    if (step >= STEPS) {
      step = 0;
    }
    return step;
  }
};

class Backward : public AdvanceDirection {
  int8_t advance(int8_t step) {
    step--;
    if (step < 0) {
      step = STEPS - 1;
    }
    return step;
  }
};

class PingPong : public AdvanceDirection {
private:
  int8_t direction = 1;

public:
  int8_t advance(int8_t step) {
    step += direction;
    if (step < 0 || step > STEPS - 1) {
      direction = -direction;
      step += direction;
    }
    return step;
  }
};

extern Forward forward;

class Steps {
public:
  Steps(bool on) : steps{on, on, on, on, on, on, on, on}, numStepsOn(on ? STEPS : 0) {}

  int8_t activeStep = -1;

  void toggleStep(uint8_t);
  void setStep(uint8_t, bool);
  bool getStep(uint8_t);
  void advance(AdvanceDirection *);
  uint8_t length() { return numStepsOn; }

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
  bool offStep = false;
};
