#include <unity.h>

#include "transport.h"

#define CLOCK_DIVIDER 6

const float bpm = 120.0;
const uint32_t tickIntervalMicros = pulseIntervalMicros(bpm);

class TestTimeController : public TimeController {
public:
  void delayMicros(unsigned int delay) {}
};

class TestGateController : public GateController {
public:
  void openGate() {}
  void closeGate() {}
};

class TestDisplayController : public DisplayController {
public:
  void update() {}
};

TestTimeController timeController;
TestGateController gateController;
TestDisplayController displayController;

Sequencer createSequencer(State &state) {
  Sequencer sequencer(state, timeController, gateController);
  sequencer.setClockDivider(CLOCK_DIVIDER);
  return sequencer;
}

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

void testInternalClock() {
  State state;
  Sequencer sequencer = createSequencer(state);
  Transport transport(state, sequencer, displayController);
  TEST_ASSERT_FALSE(state.started);
  TEST_ASSERT_EQUAL(INTERNAL_CLOCK, state.syncMode);
  transport.toggleStartStop(0);
  TEST_ASSERT_TRUE(state.started);
  TEST_ASSERT_EQUAL(INTERNAL_CLOCK, state.syncMode);
}

void testExternalClockWithStart() {
  State state;
  Sequencer sequencer = createSequencer(state);
  Transport transport(state, sequencer, displayController);
  TEST_ASSERT_FALSE(state.started);
  TEST_ASSERT_EQUAL(INTERNAL_CLOCK, state.syncMode);
  transport.onMidiStart(0);
  TEST_ASSERT_TRUE(state.started);
  TEST_ASSERT_EQUAL(EXTERNAL_CLOCK, state.syncMode);
}

void testExternalClockWithoutStart() {
  State state;
  Sequencer sequencer = createSequencer(state);
  Transport transport(state, sequencer, displayController);
  TEST_ASSERT_FALSE(state.started);
  TEST_ASSERT_EQUAL(INTERNAL_CLOCK, state.syncMode);
  transport.onMidiClock(0);
  TEST_ASSERT_TRUE(state.started);
  TEST_ASSERT_EQUAL(EXTERNAL_CLOCK, state.syncMode);
}

void testExternalClockManualStop() {
  State state;
  Sequencer sequencer = createSequencer(state);
  Transport transport(state, sequencer, displayController);
  TEST_ASSERT_FALSE(state.started);
  TEST_ASSERT_EQUAL(INTERNAL_CLOCK, state.syncMode);
  transport.onMidiStart(0);
  TEST_ASSERT_TRUE(state.started);
  TEST_ASSERT_EQUAL(EXTERNAL_CLOCK, state.syncMode);
  transport.toggleStartStop(0);
  TEST_ASSERT_FALSE(state.started);
  TEST_ASSERT_EQUAL(EXTERNAL_CLOCK_FORCE_STOP, state.syncMode);
}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(testInternalClock);
  RUN_TEST(testExternalClockWithStart);
  RUN_TEST(testExternalClockWithoutStart);
  RUN_TEST(testExternalClockManualStop);
  UNITY_END();
}
