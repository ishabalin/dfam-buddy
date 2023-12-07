#include <unity.h>

// #include "state.h"
#include "step_controller.h"

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

void testEditPermStep() {
  State state;
  StepController stepController = StepController(state);
  TEST_ASSERT_TRUE(state.permSteps.getStep(0));
  stepController.shiftKeyDown();
  stepController.stepKeyDown(0);
  TEST_ASSERT_FALSE(state.permSteps.getStep(0));
}

void testEditTempStep() {
  State state;
  state.started = true;
  StepController stepController = StepController(state);
  TEST_ASSERT_FALSE(state.tempSteps.getStep(0));
  stepController.stepKeyDown(0);
  TEST_ASSERT_TRUE(state.tempSteps.getStep(0));
  stepController.stepKeyUp(0);
  TEST_ASSERT_FALSE(state.tempSteps.getStep(0));
}

void testSetDfamStep() {
  State state;
  state.started = false;
  StepController stepController = StepController(state);
  TEST_ASSERT_EQUAL(0, state.dfamStep);
  stepController.stepKeyDown(5);
  TEST_ASSERT_EQUAL(5, state.dfamStep);
}

void testTempStepsOnShiftUp() {
  State state;
  state.started = true;
  StepController stepController = StepController(state);
  // set temp steps
  for (int i = 0; i < 3; i++) {
    stepController.stepKeyDown(i);
    TEST_ASSERT_TRUE(state.tempSteps.getStep(i));
  }
  // press shift and release keys
  // temp steps should hold
  stepController.shiftKeyDown();
  for (int i = 0; i < 3; i++) {
    stepController.stepKeyUp(i);
    TEST_ASSERT_TRUE(state.tempSteps.getStep(i));
  }
  // press keys again and release shift
  for (int i = 0; i < 3; i++) {
    stepController.stepKeyDown(i);
  }
  stepController.shiftKeyUp();
  // temp steps should hold
  for (int i = 0; i < 3; i++) {
    TEST_ASSERT_TRUE(state.tempSteps.getStep(i));
  }
  // press shift, release keys, then release shift
  stepController.shiftKeyDown();
  for (int i = 0; i < 3; i++) {
    stepController.stepKeyUp(i);
  }
  stepController.shiftKeyUp();
  // temp steps should go away
  for (int i = 0; i < STEPS; i++) {
    TEST_ASSERT_FALSE(state.tempSteps.getStep(i));
  }
}

void testTempStepsSync() {
  // when entering step jump mode, temp steps must sync with perm steps

  State state;
  state.started = true;
  StepController stepController = StepController(state);
  // "advance" perm steps
  state.permSteps.activeStep = 5;

  TEST_ASSERT_EQUAL(-1, state.tempSteps.activeStep);
  // set temp steps
  for (int i = 5; i < 7; i++) {
    stepController.stepKeyDown(i);
  }
  TEST_ASSERT_EQUAL(5, state.tempSteps.activeStep);

  // release keys
  for (int i = 5; i < 7; i++) {
    stepController.stepKeyUp(i);
  }
  TEST_ASSERT_EQUAL(5, state.tempSteps.activeStep);

  // "advance" perm steps
  state.permSteps.activeStep = 3;

  // set temp steps
  for (int i = 5; i < 7; i++) {
    stepController.stepKeyDown(i);
  }
  TEST_ASSERT_EQUAL(3, state.tempSteps.activeStep);
}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(testEditPermStep);
  RUN_TEST(testEditTempStep);
  RUN_TEST(testSetDfamStep);
  RUN_TEST(testTempStepsOnShiftUp);
  RUN_TEST(testTempStepsSync);
  UNITY_END();
}
