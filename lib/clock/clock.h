#pragma once

#ifdef NATIVE_PLATFORM
#include <stdlib.h>
#else
#include <Arduino.h>
#endif // NATIVE_PLATFORM

uint32_t beatIntervalMicros(float);
uint32_t pulseIntervalMicros(float);
uint8_t getClockDivider(uint8_t);
uint8_t getClockDividerStepsPerNote(uint8_t);

const uint8_t clockDividers[] = {2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 96};
const uint8_t clockDividersCount = sizeof(clockDividers);

#define TRIPLET 0b10000000

class Clock {
public:
  void start();
  void stop();
  bool update(uint32_t);
  void bpm(float);
  float bpm() const { return bpm_; }

private:
  float bpm_ = 120.0;
  bool started = false;
  uint32_t intervalMicros = 0;
  uint32_t nextPulseMicros = 0;
};
