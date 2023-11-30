#include "state.h"

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

void Steps::advance() {
  if (numStepsOn < 1) {
    activeStep = -1;
    return;
  }
  do {
    activeStep = (activeStep + 1) % STEPS;
  } while (!steps[activeStep]);
}
