#include "step_controller.h"

void StepController::shiftKeyDown() {
  shiftKeyState = true;
}

void StepController::shiftKeyUp() {
  shiftKeyState = false;
  for (int i = 0; i < STEPS; i++) {
    _state.tempSteps.setStep(i, stepKeyState[i]);
  }
}

void StepController::stepKeyDown(uint8_t i) {
  stepKeyState[i] = true;
  if (shiftKeyState) {
    _state.permSteps.toggleStep(i);
  } else if (_state.started) {
    if (_state.tempSteps.length() < 1) {
      // entering step jump mode, sync steps
      _state.tempSteps.activeStep = _state.permSteps.activeStep;
    }
    _state.tempSteps.setStep(i, true);
  } else {
    _state.dfamStep = i;
  }
}

void StepController::stepKeyUp(uint8_t i) {
  stepKeyState[i] = false;
  if (_state.started && !shiftKeyState) {
    _state.tempSteps.setStep(i, false);
  }
}

bool StepController::getLed(uint8_t i) {
  if (shiftKeyState) {
    return _state.permSteps.getStep(i);
  } else if (_state.started) {
    if (((_state.tempSteps.length() == 1) || (_state.permSteps.length() == 1)) && (_state.offStep)) {
      return false;
    }
    return (_state.activeStep == i);
  } else {
    return (_state.dfamStep == i);
  }
}
