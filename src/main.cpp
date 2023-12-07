#include <Adafruit_LEDBackpack.h>
#include <Arduino.h>
#include <Encoder.h>
#include <MIDI.h>

#include "main.h"
#include "sequencer.h"
#include "step_controller.h"
#include "transport.h"

// MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);
MIDI_CREATE_DEFAULT_INSTANCE();
Adafruit_7segment matrix = Adafruit_7segment();
Encoder knob(PIN_ENC_CLK, PIN_ENC_DT);

uint32_t keyDebounceTime[] = {0, 0, 0, 0, 0, 0, 0, 0};
bool keyLastReading[] = {false, false, false, false, false, false, false, false};
bool keyState[] = {false, false, false, false, false, false, false, false};

class SystemTimeController : public TimeController {
public:
  void delayMicros(unsigned int delay) { delayMicroseconds(delay); }
};

class GpioGateController : public GateController {
public:
  void openGate() { digitalWrite(GATE_PIN, HIGH); }
  void closeGate() { digitalWrite(GATE_PIN, LOW); }
};

class SevenSegmentDisplayController : public DisplayController {
public:
  void update() { updateDisplay(); }
};

State state;
StepController stepController(state);
SystemTimeController timeController;
GpioGateController gateController;
Sequencer sequencer(state, timeController, gateController);
SevenSegmentDisplayController displayController;
Transport transport(state, sequencer, displayController);

float clockDividerIndex = 3.0;

long encoderPosition = 0;
uint32_t encoderUpdateTime = 0;

int8_t ledUpdateIndex = 0;
uint32_t nextLedUpdateTime = 0;

int8_t keyReadIndex = 0;
uint32_t nextKeyReadTime = 0;

bool ledUpdateState = true;

uint32_t shiftKeyDebounceTime = 0;
bool shiftKeyLastReading = false;
bool shiftKeyState = false;

void selectMuxChannel(byte channel) {
  digitalWrite(MUX_PIN_S0, channel >> 0 & 1);
  digitalWrite(MUX_PIN_S1, channel >> 1 & 1);
  digitalWrite(MUX_PIN_S2, channel >> 2 & 1);
  digitalWrite(MUX_PIN_S3, channel >> 3 & 1);
}

void handleClock() {
  uint32_t nowMicros = micros();
  transport.onMidiClock(nowMicros);
}

void handleStart() {
  uint32_t nowMicros = micros();
  transport.onMidiStart(nowMicros);
}

void handleStop() {
  uint32_t nowMicros = micros();
  transport.onMidiStop(nowMicros);
}

void updateLed(uint8_t index) {
  selectMuxChannel(ledChannel[index]);
  bool on = stepController.getLed(index);
  if (on) {
    digitalWrite(MUX_PIN_EN, LOW);
    digitalWrite(MUX_PIN_SG, HIGH);
  } else {
    digitalWrite(MUX_PIN_EN, HIGH);
    digitalWrite(MUX_PIN_SG, LOW);
  }
}

void readKey(uint8_t index) {
  selectMuxChannel(keyChannel[index]);
  if (!debounceKey(index)) {
    return;
  }
  if (keyState[index]) {
    // key down
    stepController.stepKeyDown(index);
  } else {
    // key up
    stepController.stepKeyUp(index);
  }
}

bool debounceKey(uint8_t index) {
  uint32_t nowMicros = micros();
  bool value = digitalRead(MUX_PIN_SG) == LOW;
  if (value != keyLastReading[index]) {
    keyDebounceTime[index] = nowMicros;
  }
  keyLastReading[index] = value;
  if ((nowMicros - keyDebounceTime[index]) > debounceDelay) {
    if (value != keyState[index]) {
      keyState[index] = value;
      return true;
    }
  }
  return false;
}

bool debounceShiftKey(uint32_t nowMicros) {
  // uint32_t nowMicros = micros();
  bool value = digitalRead(SHIFT_BTN_PIN) == LOW;
  if (value != shiftKeyLastReading) {
    shiftKeyDebounceTime = nowMicros;
  }
  shiftKeyLastReading = value;
  if ((nowMicros - shiftKeyDebounceTime) > debounceDelay) {
    if (value != shiftKeyState) {
      shiftKeyState = value;
      if (shiftKeyState) {
        stepController.shiftKeyDown();
      } else {
        stepController.shiftKeyUp();
      }
      updateDisplay();
      return true;
    }
  }
  return false;
}

void handleEncoder() {
  long newPosition = knob.read();
  if (newPosition != encoderPosition) {
    uint32_t nowMicros = micros();
    float accel = 1.0;
    if (encoderUpdateTime > 0) {
      uint32_t delta = nowMicros - encoderUpdateTime;
      if (delta < 2500) {
        accel = 10.0;
      }
    }
    encoderUpdateTime = nowMicros;
    long incr = newPosition - encoderPosition;
    encoderPosition = newPosition;
    if (shiftKeyState) {
      sequencer.setSwing(sequencer.swing() + knobIncrement * incr * 10);
    } else if (state.syncMode == INTERNAL_CLOCK || state.syncMode == EXTERNAL_CLOCK_FORCE_STOP) {
      float bpm = transport.clock.bpm();
      bpm += knobIncrement * incr * accel;
      if (bpm < 50.0) {
        bpm = 50.0;
      }
      if (bpm > 1000.0) {
        bpm = 1000.0;
      }
      transport.clock.bpm(bpm);
    } else {
      clockDividerIndex += knobIncrement * incr * 10;
      if (clockDividerIndex < 0) {
        clockDividerIndex = 0;
      }
      if (clockDividerIndex > clockDividersCount - 1) {
        clockDividerIndex = clockDividersCount - 1;
      }
      sequencer.setClockDivider(clockDividers[int(clockDividerIndex)]);
    }
    updateDisplay();
  }
}

uint32_t encoderSwitchDebounceTime = 0;
bool encoderSwitchLastReading = false;
bool encoderSwitchState = false;

void handleSwitch(uint32_t nowMicros) {
  bool value = digitalRead(PIN_ENC_SW) == LOW;
  if (value != encoderSwitchLastReading) {
    encoderSwitchDebounceTime = nowMicros;
  }
  encoderSwitchLastReading = value;
  if ((nowMicros - encoderSwitchDebounceTime) > encoderSwitchDebounceDelay) {
    if (value != encoderSwitchState) {
      encoderSwitchState = value;
      encoderSwitchState = value;

      if (encoderSwitchState) {
        transport.toggleStartStop(nowMicros);
      }
    }
  }
}

void handleInternalClock(uint32_t nowMicros) {
  // TODO: move to transport?
  if (transport.clock.update(nowMicros)) {
    sequencer.handleTick(nowMicros);
  }
}

void showBpm(float bpm) {
  matrix.print(bpm, 1);
  matrix.writeDisplay();
}

void showSwing(float swing) {
  matrix.print(swing, 0);
  matrix.writeDigitRaw(1, 0b10000000); // dot
  matrix.writeDisplay();
}

void showClockDivider(uint8_t index) {
  matrix.writeDigitRaw(0, 0); // blank
  // matrix.writeDigitRaw(0, 0b01001001); // "X"
  // matrix.writeDigitRaw(0, 0b01010010); // "/" slash
  uint8_t stepsPerNote = getClockDividerStepsPerNote(index);
  bool triplet = (stepsPerNote & TRIPLET) != 0;
  stepsPerNote = stepsPerNote & ~TRIPLET;
  if (stepsPerNote > 9) {
    matrix.writeDigitNum(1, stepsPerNote / 10, false);
    matrix.writeDigitNum(3, stepsPerNote % 10, false);
    matrix.writeDigitRaw(4, triplet ? 0b01111000 : 0);
  } else {
    matrix.writeDigitRaw(1, 0);
    matrix.writeDigitNum(3, stepsPerNote, false);
    matrix.writeDigitRaw(4, triplet ? 0b01111000 : 0);
  }
  matrix.writeDisplay();
}

void updateDisplay() {
  if (shiftKeyState) {
    showSwing(sequencer.swing());
  } else if (state.syncMode == INTERNAL_CLOCK || state.syncMode == EXTERNAL_CLOCK_FORCE_STOP) {
    showBpm(transport.clock.bpm());
  } else {
    showClockDivider(int(clockDividerIndex));
  }
}

void processLeds(uint32_t nowMicros) {
  if (nextLedUpdateTime == 0) {
    updateLed(ledUpdateIndex);
    nextLedUpdateTime = nowMicros + ledUpdateDelay;
  } else if (nowMicros > nextLedUpdateTime) {
    digitalWrite(MUX_PIN_EN, HIGH);
    digitalWrite(MUX_PIN_SG, LOW);
    ledUpdateIndex++;
    if (ledUpdateIndex < STEPS) {
      updateLed(ledUpdateIndex);
      nextLedUpdateTime += ledUpdateDelay;
    } else {
      ledUpdateIndex = 0;
      ledUpdateState = false;
      nextLedUpdateTime = 0;
      digitalWrite(MUX_PIN_EN, LOW);
      pinMode(MUX_PIN_SG, INPUT_PULLUP);
    }
  }
}

void processKeys(uint32_t nowMicros) {
  readKey(keyReadIndex);
  if (nextKeyReadTime == 0) {
    nextKeyReadTime = nowMicros + keyReadDelay;
  } else if (nowMicros > nextKeyReadTime) {
    keyReadIndex++;
    nextKeyReadTime += keyReadDelay;
    if (keyReadIndex > 7) {
      keyReadIndex = 0;
      nextKeyReadTime = 0;
      ledUpdateState = true;
      digitalWrite(MUX_PIN_EN, HIGH);
      pinMode(MUX_PIN_SG, OUTPUT);
    }
  }
}

void setup() {
  pinMode(PIN_ENC_SW, INPUT_PULLUP);

  pinMode(MUX_PIN_SG, OUTPUT);
  pinMode(MUX_PIN_S0, OUTPUT);
  pinMode(MUX_PIN_S1, OUTPUT);
  pinMode(MUX_PIN_S2, OUTPUT);
  pinMode(MUX_PIN_S3, OUTPUT);
  pinMode(MUX_PIN_EN, OUTPUT);

  pinMode(SHIFT_BTN_PIN, INPUT_PULLUP);

  pinMode(GATE_PIN, OUTPUT);
  digitalWrite(GATE_PIN, LOW);

  // disconnect outputs
  digitalWrite(MUX_PIN_EN, HIGH);

  MIDI.setHandleClock(handleClock);
  MIDI.setHandleStart(handleStart);
  MIDI.setHandleStop(handleStop);
  MIDI.begin();

  matrix.begin(0x70);
  matrix.setBrightness(1);

  sequencer.setClockDivider(clockDividers[int(clockDividerIndex)]);

  updateDisplay();
}

void loop() {
  uint32_t nowMicros = micros();
  MIDI.read();
  debounceShiftKey(nowMicros);
  handleEncoder();
  handleSwitch(nowMicros);
  handleInternalClock(nowMicros);
  sequencer.processGate(nowMicros);

  if (ledUpdateState) {
    processLeds(nowMicros);
  } else {
    processKeys(nowMicros);
  }
  yield();
}
