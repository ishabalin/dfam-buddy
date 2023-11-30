#pragma once

#ifdef NATIVE_PLATFORM
#include <stdlib.h>
#else
#include <Arduino.h>
#endif // NATIVE_PLATFORM

#include "state.h"

// enum StepsMode {
//   DFAM_STEP,
//   ACTIVE_STEP,
//   EDIT_STEPS,
// };

class StepController {
public:
  StepController(State &state) : _state(state) {}

  // input
  void shiftKeyDown();
  void shiftKeyUp();
  void stepKeyDown(uint8_t);
  void stepKeyUp(uint8_t);

  // output
  bool getLed(uint8_t);

private:
  State &_state;
  bool shiftKeyState = false;
  bool stepKeyState[STEPS] = {false, false, false, false, false, false, false, false};
  //   StepsMode stepsMode = DFAM_STEP;
};
