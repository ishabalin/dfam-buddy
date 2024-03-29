#include "clock.h"

uint32_t beatIntervalMicros(float bpm) {
  // 1 pulse per beat, interval between pulses in micros
  // 1 beat is a quarter note
  // 1 / (BPM / 60.) * 1,000,000 == 60,000,000 / BPM
  float intervalMicros = 60000000 / bpm;
  return (uint32_t)intervalMicros;
}

uint32_t pulseIntervalMicros(float bpm) {
  // 24 pulses per beat, interval between pulses in micros
  // 1 / ((BPM * 24) / 60.) * 1,000,000 == 1,000,000 / (BPM / 2.5) == 2,500,000 / BPM
  float intervalMicros = 2500000.0 / bpm;
  // uint32_t intervalMicros = 250000000.0 / bpm;
  return (uint32_t)intervalMicros;
}

uint8_t getClockDivider(uint8_t index) {
  if (index >= clockDividersCount) {
    // fall back
    index = 3;
  }
  return clockDividers[index];
}

uint8_t getClockDividerStepsPerNote(uint8_t index) {
  uint8_t divider = getClockDivider(index);
  uint8_t stepsPerBar = ((24 * 4) / divider);
  bool triplet = ((1 < divider) && (divider < 96) && (stepsPerBar % 3 == 0));
  if (triplet) {
    stepsPerBar = TRIPLET | (stepsPerBar / 3 * 2);
  }
  return stepsPerBar;
}

void Clock::start() {
  started = true;
  nextPulseMicros = 0;
  intervalMicros = pulseIntervalMicros(bpm_);
}

void Clock::stop() {
  started = false;
}

void Clock::bpm(float value) {
  bpm_ = value;
  intervalMicros = pulseIntervalMicros(bpm_);
}

bool Clock::update(uint32_t nowMicros) {
  if (!started) {
    return false;
  }
  bool tick = false;
  if (nextPulseMicros == 0) {
    tick = true;
    nextPulseMicros = nowMicros + intervalMicros;
  } else if (nowMicros > nextPulseMicros) {
    tick = true;
    nextPulseMicros += intervalMicros;
  }
  return tick;
}
