#include <Arduino.h>

#define DAC_PIN 25      // ESP32 Internal DAC Pin
#define BUTTON_PIN 33   // Cycle through waves

#define WAVE_FREQ 440   
float phase = 0;
float phaseIncrement = (2.0 * PI * WAVE_FREQ) / 1000000.0; 
unsigned long lastMicros = 0;

enum WaveType { SINE, SQUARE, TRIANGLE, SAWTOOTH, COUNT };
WaveType currentWave = SINE;

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  lastMicros = micros();
}

void loop() {
  // 1. Button Logic
  static bool lastBtn = HIGH;
  bool currentBtn = digitalRead(BUTTON_PIN);
  if (lastBtn == HIGH && currentBtn == LOW) {
    delay(200); // Debounce
    currentWave = (WaveType)((currentWave + 1) % COUNT);
  }
  lastBtn = currentBtn;

  // 2. Timing
  unsigned long currentMicros = micros();
  phase += phaseIncrement * (currentMicros - lastMicros);
  lastMicros = currentMicros;
  if (phase >= 2.0 * PI) phase -= 2.0 * PI;

  // 3. Waveform Math (Output 0 to 255)
  int dacVal = 0;
  switch (currentWave) {
    case SINE:     
      dacVal = (int)(127.5 * (sin(phase) + 1.0)); 
      break;
    case SQUARE:   
      dacVal = (phase < PI) ? 255 : 0; 
      break;
    case TRIANGLE: 
      dacVal = (int)(255.0 * (phase < PI ? (phase / PI) : (2.0 - phase / PI))); 
      break;
    case SAWTOOTH: 
      dacVal = (int)(255.0 * (phase / (2.0 * PI))); 
      break;
  }

  dacWrite(DAC_PIN, dacVal);
}
