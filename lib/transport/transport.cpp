#include "transport.h"

void Transport::onMidiStart(uint32_t micros) {
  switch (_state.syncMode) {
    case INT:
      _state.syncMode = EXT;
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
  if (_state.syncMode == EXT_STOPPED) {
    _state.syncMode = INT;
    _displayController.update();
  }
  clock.stop();
  _state.started = false;
}

void Transport::onMidiClock(uint32_t micros) {
  switch (_state.syncMode) {
    case EXT_STOPPED:
      return;
      break;
    case INT:
      _state.syncMode = EXT;
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
    if (_state.syncMode == EXT) {
      _state.syncMode = EXT_STOPPED;
      _displayController.update();
    }
    clock.stop();
    _state.started = false;
  } else {
    _sequencer.reset();
    clock.start();
    if (_state.syncMode != INT) {
      _state.syncMode = INT;
      _displayController.update();
    }
    _state.started = true;
  }
}
