#pragma once

#ifdef NATIVE_PLATFORM
#include <stdlib.h>
#else
#include <Arduino.h>
#endif // NATIVE_PLATFORM

#include "clock.h"
#include "sequencer.h"
#include "state.h"

class DisplayController {
public:
  virtual void update() = 0;
};

class Transport {
public:
  Transport(State &state, Sequencer &sequencer, DisplayController &displayController)
      : _state(state), _sequencer(sequencer), _displayController(displayController) {}

  Clock clock;

  void onMidiStart(uint32_t);
  void onMidiStop(uint32_t);
  void onMidiClock(uint32_t);
  void toggleStartStop(uint32_t);

private:
  State &_state;
  Sequencer &_sequencer;
  DisplayController &_displayController;
};
