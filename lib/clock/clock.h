#pragma once

#ifdef NATIVE_PLATFORM
#include <stdlib.h>
#else
#include <Arduino.h>
#endif // NATIVE_PLATFORM

uint32_t beatIntervalMicros(float);
uint32_t pulseIntervalMicros(float);

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
