#include <Servo.h>

const int servoPin = 3;
const int buttonPin = 19;

const int led1Pin = 20;
const int led2Pin = 21;

Servo myServo;

// ---- Debounce ----
const unsigned long debounceMicros = 20000;

// ---- Motion ----
const unsigned long moveDuration = 1500000; // 1.5s

// ---- LED blink intervals ----
const unsigned long led1Interval = 250000; // 250 ms
const unsigned long led2Interval = 1000000; // 1s

// ---- State ----
volatile bool moveRequested = false;
volatile bool requestedDirection = true; 
// true = 0 → -90 (CCW) i.e. 90 → 0
// false = -90 → 0 (CW) i.e. 0 → 90

volatile bool cycleCompleteFlag = true;
volatile unsigned long lastButtonInterruptTime = 0;

bool motionActive = false;
unsigned long motionStartTime = 0;

// LED state
bool ledBlinkActive = false;
unsigned long lastLed1Toggle = 0;
unsigned long lastLed2Toggle = 0;
bool led1State = false;
bool led2State = false;

// -------- Button ISR --------
void handleButton() {
  unsigned long now = micros();

  if (now - lastButtonInterruptTime < debounceMicros) return;
  lastButtonInterruptTime = now;

  if (!cycleCompleteFlag) return;

  requestedDirection = !requestedDirection;

  if (!moveRequested) {
    moveRequested = true;
  }
}

// -------- Motion complete --------
void onMotionComplete() {
  cycleCompleteFlag = true;
  motionActive = false;

  // If we just did 0 → -90 (CCW), start LEDs
  if (requestedDirection) {
    ledBlinkActive = true;
  }
}

// -------- Start motion --------
void startMove(bool forward) {
  // If going -90 → 0 (CW), stop LEDs first
  if (!forward) {
    ledBlinkActive = false;
    digitalWrite(led1Pin, LOW);
    digitalWrite(led2Pin, LOW);
  }

  motionStartTime = micros();
  motionActive = true;
  cycleCompleteFlag = false;
}

// -------- S-curve --------
float sCurve(float t) {
  return (1 - cos(t * PI)) * 0.5;
}

// -------- Servo update --------
void updateServo() {
  if (!motionActive) return;

  unsigned long now = micros();
  float t = (float)(now - motionStartTime) / moveDuration;

  if (t >= 1.0) t = 1.0;

  float s = sCurve(t);

  int angle;

  if (requestedDirection) {
    // 0 → -90 (CCW) = 90 → 0
    angle = 90 - (90 * s);
  } else {
    // -90 → 0 (CW) = 0 → 90
    angle = 0 + (90 * s);
  }

  myServo.write(angle);

  if (t >= 1.0) {
    onMotionComplete();
  }
}

// -------- LED update --------
void updateLEDs() {
  if (!ledBlinkActive) return;

  unsigned long now = micros();

  if (now - lastLed1Toggle >= led1Interval) {
    lastLed1Toggle = now;
    led1State = !led1State;
    digitalWrite(led1Pin, led1State);
  }

  if (now - lastLed2Toggle >= led2Interval) {
    lastLed2Toggle = now;
    led2State = !led2State;
    digitalWrite(led2Pin, led2State);
  }
}

// -------- Setup --------
void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);

  myServo.attach(servoPin);

  // Start at "90°" (center = 90)
  myServo.write(90);

  attachInterrupt(digitalPinToInterrupt(buttonPin), handleButton, FALLING);
}

// -------- Loop --------
void loop() {
  updateServo();
  updateLEDs();

  if (moveRequested && cycleCompleteFlag) {
    moveRequested = false;
    startMove(requestedDirection);
  }
}