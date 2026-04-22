#include <Arduino.h>

#define DAC_PIN 25      
#define WAVE_BTN 33   

const int notePins[4] = {15, 18, 19, 21}; 
const float frequencies[4] = {261.63, 293.66, 329.63, 349.23};

// --- SYNTH SETTINGS ---
float masterVolume = 0.1; 
float currentGain = 0.0;    // Current volume (0.0 to 1.0)
float releaseSpeed = 0.005; // How fast it fades out (smaller = longer)
float activeFreq = 0;
float phase = 0;
unsigned long lastMicros = 0;

enum WaveType { SINE, SQUARE, TRIANGLE, SAWTOOTH, PULSE, COUNT };
WaveType currentWave = SINE;

void setup() {
  Serial.begin(115200);
  pinMode(WAVE_BTN, INPUT_PULLUP);
  for (int i = 0; i < 4; i++) pinMode(notePins[i], INPUT_PULLUP);
  dacWrite(DAC_PIN, 0);
  lastMicros = micros();
}

void loop() {
  // 1. Wave Selection
  static bool lastWaveBtnState = HIGH;
  bool waveBtnState = digitalRead(WAVE_BTN);
  if (lastWaveBtnState == HIGH && waveBtnState == LOW) {
    currentWave = (WaveType)((currentWave + 1) % COUNT);
    delay(150);
  }
  lastWaveBtnState = waveBtnState;

  // 2. Scan Buttons
  int pressedIndex = -1;
  for (int i = 0; i < 4; i++) {
    if (digitalRead(notePins[i]) == LOW) {
      pressedIndex = i;
      break;
    }
  }

  // 3. Envelope Logic (The "Fade")
  if (pressedIndex != -1) {
    activeFreq = frequencies[pressedIndex];
    currentGain = 1.0; // Instant "Attack"
  } else {
    // Release: Slowly drop the gain to 0
    if (currentGain > 0) {
      currentGain -= releaseSpeed; 
      if (currentGain < 0) currentGain = 0;
    }
  }

  // 4. Audio Generation
  unsigned long currentMicros = micros();
  float delta = (float)(currentMicros - lastMicros);
  lastMicros = currentMicros;

  if (currentGain > 0) {
    float phaseInc = (2.0 * PI * activeFreq) / 1000000.0;
    phase += phaseInc * delta;
    if (phase >= 2.0 * PI) phase -= 2.0 * PI;

    float rawVal = 0;
    switch (currentWave) {
      case SINE:     rawVal = sin(phase); break;
      case SQUARE:   rawVal = (phase < PI) ? 0.5 : -0.5; break;
      case TRIANGLE: rawVal = (phase < PI) ? (2.0 * (phase / PI) - 1.0) : (1.0 - 2.0 * ((phase - PI) / PI)); break;
      case SAWTOOTH: rawVal = ((phase / PI) - 1.0) * 0.7; break;
      case PULSE:    rawVal = (phase < (PI * 0.3)) ? 0.5 : -0.5; break; // Thin 15% pulse
    }
    
    // Apply the Envelope (currentGain)
    int dacOutput = 127 + (int)(rawVal * 127.0 * masterVolume * currentGain);
    dacWrite(DAC_PIN, dacOutput);
  } else {
    dacWrite(DAC_PIN, 127); // Stay at center when silent to avoid DC "thump"
    phase = 0;
  }
}
