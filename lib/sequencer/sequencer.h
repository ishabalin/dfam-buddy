#pragma once

#ifdef NATIVE_PLATFORM
#include <stdlib.h>
#else
#include <Arduino.h>
#endif // NATIVE_PLATFORM

#include "state.h"

#define GATE_LENGTH_MICROS 1000

class GateController {
public:
  virtual void openGate() = 0;
  virtual void closeGate() = 0;
};

class TimeController {
public:
  virtual void delayMicros(unsigned int) = 0;
};

class Sequencer {
public:
  Sequencer(State &state, TimeController &timeController, GateController &gateController)
      : _state(state), _timeController(timeController), _gateController(gateController) {}

  void reset();
  void handleTick(uint32_t);
  void processGate(uint32_t);
  void setClockDivider(uint8_t);
  float swing() { return swing_; }
  void setSwing(float value);

protected:
  // transport
  uint32_t ticks = 0;
  uint8_t clockDivider = 6;
  uint8_t pendingClockDivider = 6;
  float swing_ = 50.0;

  // global step counter, that doesn't wrap around
  int8_t globalStep = -1;

  // gate
  uint32_t gateRequestTime = 0;
  bool gateOpen = false;

  // keep track of step length for swing
  uint32_t lastStepLength = 0;
  uint32_t lastStepTime = 0;

protected:
  void advance(uint32_t);

private:
  State &_state;
  TimeController &_timeController;
  GateController &_gateController;

  void delayMicros(unsigned int);
  void openGate();
  void closeGate();
  void advanceDfamStep();
};
