#include <Servo.h>

// Define constants
static const int servoPin = 3;
static const int led1Pin = 20;
static const int led2Pin = 21;
static const int buttonPin = 19;

// Define classes
Servo radarServo;  

// Define variables
static int pos = 0;    // variable to store the servo position
static int led1Interval = 252; // Interval (in milliseconds) that LED 1 will toggle
static int led2Interval = 1000; // Interval (in milliseconds) that LED 2 will toggle
static int debounceInterval = 1000;  // the debounce time; increase if the output flickers
static int servoStepInterval = 15;  // time between servo steps
static uint8_t deviceState = 0;  // high when radar arm is down (on), low when radar arm is up (off)
static uint8_t led1State = 0;
static uint8_t led2State = 0;
static uint8_t buttonState = 0;
static unsigned long curMillis = 0;
static unsigned long servoStepTimer = 0;
static unsigned long Led1Timer = 0;
static unsigned long Led2Timer = 500;
static unsigned long debounceTimer = 0;  // the last time the output pin was toggled

void setup() {
  // Setup Servos
  radarServo.attach(servoPin);

  // Setup LEDs
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);

  // Setup button
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  // Get current program run time
  curMillis = millis();
  // Is button pushed?
  buttonState = digitalRead(buttonPin);

  if (curMillis - debounceTimer >= debounceInterval){
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // only toggle the device state if the new button state is HIGH
    if (buttonState == LOW){
      deviceState = !deviceState;
      debounceTimer = curMillis;
    }
  }

  if (deviceState == HIGH){
    if (curMillis - servoStepTimer >= servoStepInterval){
      if (pos != 90){
        // Sweep radar arm to down positon when device state is active (90 deg)
        for (pos = 0; pos <= 90; pos += 1) { // goes from 0 degrees to 90 degrees
          // in steps of 1 degree
          radarServo.write(pos);              // tell servo to go to position in variable 'pos'
          
        }
      }
    }

    // Toggle LED 1 if enough time passed
    if (curMillis - Led1Timer >= led1Interval){
      led1State = led1State ^ 1;
      Led1Timer = curMillis;
    }
    
    // Toggle LED 2 if enough time passed
    if (curMillis - Led2Timer >= led2Interval){
      led2State = led2State ^ 1;
      Led2Timer = curMillis;
    }

    // Set LED pins
    digitalWrite(led1Pin, led1State);
    digitalWrite(led2Pin, led2State);
  }
  
  else {
    digitalWrite(led1Pin, LOW);
    digitalWrite(led2Pin, LOW);
    if (curMillis - servoStepTimer >= servoStepInterval){
      if (pos != 0){
        for (pos = 90; pos >= 0; pos -= 1) { // goes from 90 degrees to 0 degrees
          radarServo.write(pos);              // tell servo to go to position in variable 'pos'
        }
      }
    }
  }
}