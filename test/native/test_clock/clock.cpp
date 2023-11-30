#include <unity.h>

#include "clock.h"

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

void testBeatIntervalMicros() {
  TEST_ASSERT_EQUAL(1000000, beatIntervalMicros(60));
  TEST_ASSERT_EQUAL(500000, beatIntervalMicros(120));
}

void testPulseIntervalMicros() {
  TEST_ASSERT_EQUAL(41666, pulseIntervalMicros(60));
  TEST_ASSERT_EQUAL(20833, pulseIntervalMicros(120));
}

void testClockCallback() {
  Clock clock;
  // not started
  TEST_ASSERT_FALSE(clock.update(0));
  // start
  clock.start();
  TEST_ASSERT_TRUE(clock.update(0));
  // next tick is not due yet
  TEST_ASSERT_FALSE(clock.update(0));
  // one second later
  TEST_ASSERT_TRUE(clock.update(1000000));
}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(testBeatIntervalMicros);
  RUN_TEST(testPulseIntervalMicros);
  RUN_TEST(testClockCallback);
  UNITY_END();
}
