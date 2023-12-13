#include "state.h"

Forward forward;

void Steps::toggleStep(uint8_t i) {
  setStep(i, !getStep(i));
}

void Steps::setStep(uint8_t i, bool on) {
  if (on != steps[i]) {
    numStepsOn += on ? 1 : -1;
  }
  steps[i] = on;
}

bool Steps::getStep(uint8_t i) {
  return steps[i];
}

void Steps::advance(AdvanceDirection *direction) {
  if (numStepsOn < 1) {
    activeStep = -1;
    return;
  }
  do {
    if (direction == nullptr) {
      activeStep = forward.advance(activeStep);
    } else {
      activeStep = direction->advance(activeStep);
    }
  } while (!steps[activeStep]);
}
