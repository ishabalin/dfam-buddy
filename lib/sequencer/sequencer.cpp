#include "sequencer.h"

void Sequencer::reset() {
  ticks = 0;
  _state.activeStep = -1;
  globalStep = -1;

  _state.permSteps.activeStep = -1;
  _state.tempSteps.activeStep = -1;
}

void Sequencer::setSwing(float value) {
  if (value > 75.0) {
    value = 75.0;
  }
  if (value < 50.0) {
    value = 50.0;
  }
  swing_ = value;
}

void Sequencer::setClockDivider(uint8_t value) {
  pendingClockDivider = value;
}

void Sequencer::handleTick(uint32_t micros) {
  if (ticks == 0) {
    advance(micros);
  }
  ticks += 1;
  _state.offStep = (ticks >= clockDivider / 2);
  if (ticks == clockDivider) {
    ticks = 0;
    clockDivider = pendingClockDivider;
  }
}

void Sequencer::advanceDfamStep() {
  _state.dfamStep = (_state.dfamStep + 1) % STEPS;
}

void Sequencer::delayMicros(unsigned int micros) {
  _timeController.delayMicros(micros);
}

void Sequencer::openGate() {
  _gateController.openGate();
}

void Sequencer::closeGate() {
  _gateController.closeGate();
}

void Sequencer::advance(uint32_t micros) {
  globalStep++;
  if (lastStepTime > 0) {
    lastStepLength = micros - lastStepTime;
  }
  lastStepTime = micros;
  _state.activeStep = -1;

  // keep advancing permanent steps even when
  // temporary steps are active, for "step jump"
  _state.permSteps.advance();
  _state.activeStep = _state.permSteps.activeStep;
  _state.tempSteps.advance();
  if (_state.tempSteps.activeStep >= 0) {
    _state.activeStep = _state.tempSteps.activeStep;
  }
  if (_state.activeStep >= 0) {
    gateRequestTime = micros;
    if (globalStep % 2 == 1) {
      // for globalStep >= 1, lastStepLength is guaranteed to be set
      gateRequestTime += lastStepLength * (swing_ * 2 - 100) / 100.0;
    }
  }
}

void Sequencer::processGate(uint32_t micros) {
  if (gateRequestTime > 0) {
    if (gateOpen && (micros > (gateRequestTime + GATE_LENGTH_MICROS))) {
      closeGate();
      gateOpen = false;
      gateRequestTime = 0;
      advanceDfamStep();
      return;
    }
    if (_state.activeStep < 0 || gateOpen) {
      return;
    }
    int numSkipSteps = (_state.activeStep - _state.dfamStep + 7) % STEPS;
    // inline skip
    if (numSkipSteps > 0) {
      for (int i = 0; i < numSkipSteps; i++) {
        openGate();
        delayMicros(3);
        closeGate();
        delayMicros(3);
        advanceDfamStep();
      }
    }
    if (micros >= gateRequestTime) {
      openGate();
      gateOpen = true;
    }
  }
}
