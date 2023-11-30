#pragma once

#include <Arduino.h>

#define PIN_ENC_CLK 3
#define PIN_ENC_DT 2
#define PIN_ENC_SW 4

#define MUX_PIN_SG 14
#define MUX_PIN_S0 8
#define MUX_PIN_S1 7
#define MUX_PIN_S2 6
#define MUX_PIN_S3 5
#define MUX_PIN_EN 9

#define SHIFT_BTN_PIN 10
#define GATE_PIN 11

#define MUX_CHAN_LED1 14
#define MUX_CHAN_LED2 12
#define MUX_CHAN_LED3 10
#define MUX_CHAN_LED4 8
#define MUX_CHAN_LED5 6
#define MUX_CHAN_LED6 4
#define MUX_CHAN_LED7 2
#define MUX_CHAN_LED8 0
#define MUX_CHAN_BTN1 15
#define MUX_CHAN_BTN2 13
#define MUX_CHAN_BTN3 11
#define MUX_CHAN_BTN4 9
#define MUX_CHAN_BTN5 7
#define MUX_CHAN_BTN6 5
#define MUX_CHAN_BTN7 3
#define MUX_CHAN_BTN8 1

const uint8_t keyChannel[] = {
  MUX_CHAN_BTN1,
  MUX_CHAN_BTN2,
  MUX_CHAN_BTN3,
  MUX_CHAN_BTN4,
  MUX_CHAN_BTN5,
  MUX_CHAN_BTN6,
  MUX_CHAN_BTN7,
  MUX_CHAN_BTN8,
};

const uint8_t ledChannel[] = {
  MUX_CHAN_LED1,
  MUX_CHAN_LED2,
  MUX_CHAN_LED3,
  MUX_CHAN_LED4,
  MUX_CHAN_LED5,
  MUX_CHAN_LED6,
  MUX_CHAN_LED7,
  MUX_CHAN_LED8,
};

// micros
const uint32_t debounceDelay = 10;
const uint32_t keyReadDelay = 10;
const uint32_t encoderSwitchDebounceDelay = 500;

const float knobIncrement = 0.025;
const uint32_t ledUpdateDelay = 2000;

void updateDisplay();
bool debounceKey(uint8_t);
void startSequencer();
void stopSequencer();
