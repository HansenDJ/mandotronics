const uint8_t leftJetPins[2] = {5, 6};
const uint8_t rightJetPins[2] = {9, 10};

const uint8_t darkBrightness = 20;
const uint8_t lightBrightness = 240;
const uint8_t randomBrightnessVariance = 15;

const unsigned long flickerCycleMs = 2000;

struct JetState {
  unsigned long cycleStartMs;
  int brightnessOffset;
  unsigned long offsetChangeMs;
  unsigned long offsetChangeDurationMs;
};

JetState leftJet;
JetState rightJet;

float smoothStep(float t) {
  if (t <= 0.0f) return 0.0f;
  if (t >= 1.0f) return 1.0f;
  return t * t * (3.0f - 2.0f * t);
}

float triangleWave(float t) {
  // 0 -> 1 -> 0 over one cycle
  if (t < 0.5f) {
    return t * 2.0f;
  } else {
    return 2.0f - t * 2.0f;
  }
}

void initializeJet(JetState &jet, unsigned long nowMs) {
  jet.cycleStartMs = nowMs;
  jet.brightnessOffset = 0;
  jet.offsetChangeMs = nowMs;
  jet.offsetChangeDurationMs = (unsigned long)random(150, 400);
}

uint8_t updateJetFlicker(JetState &jet, unsigned long nowMs) {
  // Main 2-second cycle: dark-light-dark
  unsigned long cyclePhase = (nowMs - jet.cycleStartMs) % flickerCycleMs;
  float cycleT = (float)cyclePhase / (float)flickerCycleMs;
  float triangleBrightness = triangleWave(cycleT);
  
  // Apply smoothing to the triangle for gentler transitions
  float eased = smoothStep(triangleBrightness);
  
  // Calculate base brightness from the eased cycle
  float baseBrightness = darkBrightness + (lightBrightness - darkBrightness) * eased;
  
  // Add subtle random offset that changes slowly
  unsigned long offsetElapsed = nowMs - jet.offsetChangeMs;
  if (offsetElapsed >= jet.offsetChangeDurationMs) {
    jet.brightnessOffset = (int)random(-randomBrightnessVariance, randomBrightnessVariance + 1);
    jet.offsetChangeMs = nowMs;
    jet.offsetChangeDurationMs = (unsigned long)random(150, 400);
  }
  
  // Combine base brightness with offset
  int brightness = (int)baseBrightness + jet.brightnessOffset;
  
  // Clamp to valid PWM range
  if (brightness < 0) brightness = 0;
  if (brightness > 255) brightness = 255;
  
  return (uint8_t)brightness;
}

void setup() {
  Serial.begin(9600);
  
  for (uint8_t index = 0; index < 2; index++) {
    pinMode(leftJetPins[index], OUTPUT);
    pinMode(rightJetPins[index], OUTPUT);
  }

  randomSeed(analogRead(A0) + micros());

  unsigned long nowMs = millis();
  initializeJet(leftJet, nowMs);
  
  // Right jet starts slightly offset for visual variety
  initializeJet(rightJet, nowMs + random(300, 700));
}

void loop() {
  unsigned long nowMs = millis();
  
  uint8_t leftBrightness = updateJetFlicker(leftJet, nowMs);
  uint8_t rightBrightness = updateJetFlicker(rightJet, nowMs);
  
  for (uint8_t index = 0; index < 2; index++) {
    analogWrite(leftJetPins[index], leftBrightness);
    analogWrite(rightJetPins[index], rightBrightness);
  }
}
