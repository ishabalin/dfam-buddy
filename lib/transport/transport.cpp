#include "transport.h"

void Transport::onMidiStart(uint32_t micros) {
  switch (_state.syncMode) {
    case INTERNAL_CLOCK:
      _state.syncMode = EXTERNAL_CLOCK;
      clock.stop();
      _displayController.update();
      break;
    default:
      break;
  }
  _sequencer.reset();
  _state.started = true;
}

void Transport::onMidiStop(uint32_t micros) {
  if (_state.syncMode == EXTERNAL_CLOCK_FORCE_STOP) {
    // after being manually stopped and receiving
    // stop by MIDI, switch to internal clock mode
    _state.syncMode = INTERNAL_CLOCK;
    _displayController.update();
  }
  clock.stop();
  _state.started = false;
}

void Transport::onMidiClock(uint32_t micros) {
  switch (_state.syncMode) {
    case EXTERNAL_CLOCK_FORCE_STOP:
      // after being manually stopped ignore MIDI clock
      return;
      break;
    case INTERNAL_CLOCK:
      // switch to external clock mode
      _state.syncMode = EXTERNAL_CLOCK;
      clock.stop();
      _displayController.update();
      if (!_state.started) {
        _state.started = true;
      }
    default:
      _sequencer.handleTick(micros);
  }
}

void Transport::toggleStartStop(uint32_t micros) {

  if (_state.started) {
    if (_state.syncMode == EXTERNAL_CLOCK) {
      // manually stopped while in external clock mode
      _state.syncMode = EXTERNAL_CLOCK_FORCE_STOP;
      _displayController.update();
    }
    clock.stop();
    _state.started = false;
  } else {
    _sequencer.reset();
    clock.start();
    if (_state.syncMode != INTERNAL_CLOCK) {
      // after being manually started switch to internal clock mode
      _state.syncMode = INTERNAL_CLOCK;
      _displayController.update();
    }
    _state.started = true;
  }
}
