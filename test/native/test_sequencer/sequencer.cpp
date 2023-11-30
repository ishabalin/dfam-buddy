#include <unity.h>

#include "clock.h"
#include "sequencer.h"

#define CLOCK_DIVIDER 6

const float bpm = 120.0;
const uint32_t tickIntervalMicros = pulseIntervalMicros(bpm);

class TestTimeController : public TimeController {
public:
  unsigned int micros = 1000;
  unsigned int ticks = 0;

  void delayMicros(unsigned int delay) { micros += delay; }

  void skipToNextTick() {
    ticks++;
    micros = 1000 + ticks * tickIntervalMicros;
  }

  void reset() {
    micros = 1000;
    ticks = 0;
  }
};

class TestGateController : public GateController {
public:
  bool gateOpen = false;
  unsigned int gateCount = 0;

  void openGate() {
    gateCount++;
    gateOpen = true;
  }

  void closeGate() { gateOpen = false; }

  void reset() {
    gateOpen = false;
    gateCount = 0;
  }
};

TestTimeController timeController;
TestGateController gateController;

void setUp(void) {
  timeController.reset();
  gateController.reset();
}

void tearDown(void) {
  // clean stuff up here
}

Sequencer createSequencer(State &state) {
  Sequencer sequencer(state, timeController, gateController);
  sequencer.setClockDivider(CLOCK_DIVIDER);
  return sequencer;
}

void testPermSteps() {
  State state;
  Sequencer sequencer = createSequencer(state);
  // start, send first tick
  sequencer.handleTick(timeController.micros);
  sequencer.processGate(timeController.micros);
  // on initial start should send 7 skip gates + 1 step gate
  TEST_ASSERT_EQUAL(7 + 1, gateController.gateCount);
  // send remaining ticks
  for (int i = 0; i < CLOCK_DIVIDER; i++) {
    timeController.skipToNextTick();
    sequencer.handleTick(timeController.micros);
    sequencer.processGate(timeController.micros);
  }
  // another step gate must have been sent
  TEST_ASSERT_EQUAL(7 + 2, gateController.gateCount);
}

void testTempSteps() {
  State state;
  Sequencer sequencer = createSequencer(state);
  // enable temp steps
  for (int i = 0; i < 3; i++) {
    state.tempSteps.setStep(i, true);
  }
  // start, send first tick
  sequencer.handleTick(timeController.micros);
  sequencer.processGate(timeController.micros);

  // on initial start should send 7 skip gates + 1 step gate
  TEST_ASSERT_EQUAL(7 + 1, gateController.gateCount);
  // send 2 more steps, CLOCK_DIVIDER x 2 ticks
  for (int i = 0; i < CLOCK_DIVIDER * 2; i++) {
    timeController.skipToNextTick();
    sequencer.handleTick(timeController.micros);
    sequencer.processGate(timeController.micros);
  }
  TEST_ASSERT_EQUAL(7 + 3, gateController.gateCount);
  // send another step, 6 ticks
  for (int i = 0; i < CLOCK_DIVIDER; i++) {
    timeController.skipToNextTick();
    sequencer.handleTick(timeController.micros);
    sequencer.processGate(timeController.micros);
  }
  // should send 5 skip gates + 1 step gate
  TEST_ASSERT_EQUAL(7 + 3 + 5 + 1, gateController.gateCount);
}

void testDisabledPermSteps() {
  State state;
  Sequencer sequencer = createSequencer(state);
  // disable steps 4 through 8
  for (int i = 3; i < 8; i++) {
    state.permSteps.setStep(i, false);
  }
  // start, send first tick
  sequencer.handleTick(timeController.micros);
  sequencer.processGate(timeController.micros);
  // on initial start should send 7 skip gates + 1 step gate
  TEST_ASSERT_EQUAL(7 + 1, gateController.gateCount);
  // send 2 more steps, 6 x 2 ticks
  for (int i = 0; i < CLOCK_DIVIDER * 2; i++) {
    timeController.skipToNextTick();
    sequencer.handleTick(timeController.micros);
    sequencer.processGate(timeController.micros);
  }
  TEST_ASSERT_EQUAL(7 + 3, gateController.gateCount);
  // send another step, 6 ticks
  for (int i = 0; i < CLOCK_DIVIDER; i++) {
    timeController.skipToNextTick();
    sequencer.handleTick(timeController.micros);
    sequencer.processGate(timeController.micros);
  }
  // should send 5 skip gates + 1 step gate
  TEST_ASSERT_EQUAL(7 + 3 + 5 + 1, gateController.gateCount);
}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(testPermSteps);
  RUN_TEST(testDisabledPermSteps);
  UNITY_END();
}
