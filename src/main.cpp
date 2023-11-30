#include <Adafruit_LEDBackpack.h>
#include <Arduino.h>
#include <Encoder.h>
#include <MIDI.h>

#include "clock.h"
#include "main.h"
#include "sequencer.h"
#include "step_controller.h"

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

State state;
StepController stepController(state);
Clock clock;
SystemTimeController timeController;
GpioGateController gateController;
Sequencer sequencer(state, timeController, gateController);

long encoderPosition = 0;

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
  // TODO: stop internal clock
  // clock.stop();
  // TODO: keep track of when last received external clock
  sequencer.handleTick(micros());
  // incrementClock();
}

void handleStart() {
  // TODO: stop internal clock
  // clock.stop();
  // TODO: keep track of when last received external clock
  startSequencer();
}

void handleStop() {
  // TODO: stop internal clock
  // clock.stop();
  // TODO: keep track of when last received external clock
  stopSequencer();
}

void startSequencer() {
  state.started = true;
}

void stopSequencer() {
  state.started = false;
  sequencer.reset();
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
    long incr = newPosition - encoderPosition;
    encoderPosition = newPosition;
    if (shiftKeyState) {
      sequencer.setSwing(sequencer.swing() + knobIncrement * incr * 10);
    } else {
      float bpm = clock.bpm();
      bpm += knobIncrement * incr;
      clock.bpm(bpm);
    }
    // switch (theState) {
    //   case STOPPED | SHIFT:
    //   case STARTED | SHIFT:
    //     // float swing = sequencer.swing();
    //     // swing += knobIncrement * incr * 10;
    //     sequencer.setSwing(sequencer.swing() + knobIncrement * incr * 10);
    //     // swing = max(swing, 50.0);
    //     // swing = min(swing, 75.0);
    //     break;
    //   default:
    //     float bpm = clock.bpm();
    //     bpm += knobIncrement * incr;
    //     clock.bpm(bpm);
    //     // intervalMicros = pulseIntervalMicros(bpm);
    //     break;
    // }
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
        if (state.started) {
          stopSequencer();
          clock.stop();
        } else {
          startSequencer();
          clock.start();
        }
      }
    }
  }
}

void handleInternalClock(uint32_t nowMicros) {
  if (clock.update(nowMicros)) {
    // uint32_t nowMicros = micros();
    sequencer.handleTick(nowMicros);
    // handleClockTick();
  }
  // if (!started) {
  //   return;
  // }
  // if (nextPulseMicros == 0) {
  //   handleClock();
  //   nextPulseMicros = nowMicros + intervalMicros;
  // } else if (nowMicros > nextPulseMicros) {
  //   handleClock();
  //   nextPulseMicros += intervalMicros;
  // }
}

void updateDisplay() {
  if (shiftKeyState) {
    matrix.print(sequencer.swing(), 0);
    matrix.writeDisplay();
  } else {
    matrix.print(clock.bpm(), 1);
    matrix.writeDisplay();
  }
  // switch (theState) {
  //   case STOPPED | SHIFT:
  //   case STARTED | SHIFT:
  //     matrix.print(sequencer.swing(), 0);
  //     matrix.writeDisplay();
  //     break;
  //   default:
  //     // matrix.print(bpm, 1);
  //     matrix.print(clock.bpm(), 1);
  //     matrix.writeDisplay();
  //     break;
  // }
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

  updateDisplay();

  sequencer.setClockDivider(6);

  // clock.start();
  // startSequencer();
}

void loop() {
  uint32_t nowMicros = micros();
  MIDI.read();
  debounceShiftKey(nowMicros);
  handleEncoder();
  handleSwitch(nowMicros);
  handleInternalClock(nowMicros);
  // handleGate(nowMicros);
  sequencer.processGate(nowMicros);

  if (ledUpdateState) {
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
  } else {
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
  yield();
  // delayMicroseconds(1);
}
