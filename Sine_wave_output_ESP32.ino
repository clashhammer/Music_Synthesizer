#include <Arduino.h>
#include <driver/dac.h> // The ESP32 library for Analog Output

// 1. GLOBAL SETTINGS
const int dacPin = 25;          // GPIO 25 is DAC Channel 1
const float frequency = 440.0;  // Frequency in Hz (440Hz is Note A4)
const float amplitude = 127;    // Half of 255 (The "height" of the wave)
const float offset = 128;       // The "middle" point (The "Zero" of our wave)

// 2. TIMING VARIABLES
unsigned long lastMicros = 0;
float phase = 0;

void setup() {
  // Enable the DAC on the specific pin
  dac_output_enable(DAC_CHANNEL_1); 
}

void loop() {
  // 3. THE MATH ENGINE
  // We calculate how much time has passed since the last loop
  unsigned long currentMicros = micros();
  float deltaTime = (currentMicros - lastMicros) / 1000000.0; // Convert to seconds
  lastMicros = currentMicros;

  // 4. PHASE CALCULATION
  // Phase moves from 0 to 2*PI to complete one full circle (one wave)
  phase += 2.0 * PI * frequency * deltaTime;
  
  // Keep phase between 0 and 2*PI to prevent number overflow
  if (phase >= 2.0 * PI) {
    phase -= 2.0 * PI;
  }

  // 5. GENERATING THE VOLTAGE
  // sin(phase) gives a value between -1 and 1
  // We multiply by amplitude and add offset to get a value between 0 and 255
  int dacValue = (int)(offset + amplitude * sin(phase));

  // 6. OUTPUT TO PIN
  dac_output_voltage(DAC_CHANNEL_1, dacValue);
}