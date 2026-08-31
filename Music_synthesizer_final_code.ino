#include <Arduino.h>
#include <ESP_I2S.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

constexpr uint8_t I2S_LCK = 25;
constexpr uint8_t I2S_DIN = 27;
constexpr uint8_t I2S_BCK = 26;

constexpr uint8_t SHIFT_DATA = 22;
constexpr uint8_t SHIFT_CLOCK = 16;
constexpr uint8_t SHIFT_LATCH = 17;

constexpr uint8_t TFT_SCK = 18;
constexpr uint8_t TFT_MOSI = 23;
constexpr uint8_t TFT_CS = 21;
constexpr uint8_t TFT_DC = 19;
constexpr uint8_t TFT_RST = 32;

constexpr uint8_t MATRIX_ROWS = 6;
constexpr uint8_t MATRIX_COLS = 4;
constexpr uint8_t NUM_KEYS = 24;

constexpr uint8_t COL_PINS[MATRIX_COLS] = {
  36,
  39,
  34,
  35
};

constexpr uint8_t WAVE_OUTPUT = 6;
constexpr uint8_t TONE_OUTPUT = 7;

constexpr uint8_t WAVE_COUNT = 5;
constexpr uint8_t TONE_COUNT = 5;

constexpr uint32_t SAMPLE_RATE = 44100;
constexpr uint16_t AUDIO_FRAMES = 128;
constexpr uint16_t SCOPE_BUFFER_SIZE = 4096;

constexpr float TWO_PI_F = 6.28318530718f;
constexpr float MASTER_VOLUME = 0.05f;

const float notes[NUM_KEYS] = {
  261.63f,
  277.18f,
  293.66f,
  311.13f,
  329.63f,
  349.23f,
  369.99f,
  392.00f,
  415.30f,
  440.00f,
  466.16f,
  493.88f,
  523.25f,
  554.37f,
  587.33f,
  622.25f,
  659.25f,
  698.46f,
  739.99f,
  783.99f,
  830.61f,
  880.00f,
  932.33f,
  987.77f
};

const char* noteNames[NUM_KEYS] = {
  "C4",
  "C#4",
  "D4",
  "D#4",
  "E4",
  "F4",
  "F#4",
  "G4",
  "G#4",
  "A4",
  "A#4",
  "B4",
  "C5",
  "C#5",
  "D5",
  "D#5",
  "E5",
  "F5",
  "F#5",
  "G5",
  "G#5",
  "A5",
  "A#5",
  "B5"
};

const char* waveNames[WAVE_COUNT] = {
  "SINE",
  "SQUARE",
  "SAW",
  "TRIANGLE",
  "PULSE"
};

const char* toneNames[TONE_COUNT] = {
  "NORMAL",
  "BASS",
  "TREBLE",
  "MELLOW",
  "BRIGHT"
};

I2SClass I2S;

Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);

volatile uint32_t pressedMask = 0;
volatile uint8_t waveType = 0;
volatile uint8_t toneType = 0;

uint32_t debounceMask = 0;
uint32_t previousMask = 0;
uint32_t debounceStart = 0;

bool waveRaw = false;
bool waveStable = false;
uint32_t waveChangeTime = 0;

bool toneRaw = false;
bool toneStable = false;
uint32_t toneChangeTime = 0;

uint32_t lastDisplayUpdate = 0;

float phases[NUM_KEYS] = {};
float gains[NUM_KEYS] = {};

float bassLP = 0.0f;
float trebleLP = 0.0f;
float mellowLP = 0.0f;
float brightLP = 0.0f;

int16_t audioBuffer[AUDIO_FRAMES * 2];

volatile int16_t scopeBuffer[SCOPE_BUFFER_SIZE] = {};
volatile uint16_t scopeWriteIndex = 0;

constexpr int16_t SCREEN_W = 320;
constexpr int16_t SCREEN_H = 240;

constexpr int16_t SCOPE_X = 45;
constexpr int16_t SCOPE_Y = 31;
constexpr int16_t SCOPE_W = 270;
constexpr int16_t SCOPE_H = 155;

void write595(uint8_t value) {
  digitalWrite(SHIFT_LATCH, LOW);
  shiftOut(SHIFT_DATA, SHIFT_CLOCK, MSBFIRST, value);
  digitalWrite(SHIFT_LATCH, HIGH);
}

uint32_t scanMatrix() {
  uint32_t result = 0;

  for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
    write595(1 << row);

    delayMicroseconds(10);

    for (uint8_t column = 0; column < MATRIX_COLS; column++) {
      if (digitalRead(COL_PINS[column]) == HIGH) {
        uint8_t key = row * MATRIX_COLS + column;
        result |= (1UL << key);
      }
    }
  }

  write595(0x00);

  return result;
}

void updateKeyboard() {
  uint32_t rawMask = scanMatrix();
  uint32_t now = millis();

  if (rawMask != debounceMask) {
    debounceMask = rawMask;
    debounceStart = now;
  }

  if (now - debounceStart >= 8) {
    pressedMask = debounceMask;
  }
}

bool readShiftButton(uint8_t output) {
  write595(1 << output);

  delayMicroseconds(10);

  bool pressed =
    digitalRead(COL_PINS[0]) == HIGH;

  write595(0x00);

  return pressed;
}

void updateControlButtons() {
  uint32_t now = millis();

  bool newWaveRaw =
    readShiftButton(WAVE_OUTPUT);

  if (newWaveRaw != waveRaw) {
    waveRaw = newWaveRaw;
    waveChangeTime = now;
  }

  if (
    now - waveChangeTime >= 25 &&
    waveStable != waveRaw
  ) {
    waveStable = waveRaw;

    if (waveStable) {
      waveType++;

      if (waveType >= WAVE_COUNT) {
        waveType = 0;
      }

      Serial.print("WAVE: ");
      Serial.println(waveNames[waveType]);
    }
  }

  bool newToneRaw =
    readShiftButton(TONE_OUTPUT);

  if (newToneRaw != toneRaw) {
    toneRaw = newToneRaw;
    toneChangeTime = now;
  }

  if (
    now - toneChangeTime >= 25 &&
    toneStable != toneRaw
  ) {
    toneStable = toneRaw;

    if (toneStable) {
      toneType++;

      if (toneType >= TONE_COUNT) {
        toneType = 0;
      }

      Serial.print("TONE: ");
      Serial.println(toneNames[toneType]);
    }
  }
}

float generateWave(uint8_t type, float phase) {
  switch (type) {
    case 0:
      return sinf(phase);

    case 1:
      return phase < PI
        ? 1.0f
        : -1.0f;

    case 2:
      return phase / PI - 1.0f;

    case 3:
      if (phase < PI) {
        return -1.0f +
          2.0f * phase / PI;
      }

      return 3.0f -
        2.0f * phase / PI;

    case 4:
      return phase <
        TWO_PI_F * 0.20f
        ? 1.0f
        : -1.0f;
  }

  return 0.0f;
}

float processTone(
  float sample,
  uint8_t type
) {
  switch (type) {
    case 0:
      return sample;

    case 1:
      bassLP +=
        0.035f *
        (sample - bassLP);

      return
        sample * 0.60f +
        bassLP * 0.70f;

    case 2: {
      trebleLP +=
        0.25f *
        (sample - trebleLP);

      float high =
        sample - trebleLP;

      return
        sample * 0.70f +
        high * 0.75f;
    }

    case 3:
      mellowLP +=
        0.15f *
        (sample - mellowLP);

      return mellowLP;

    case 4: {
      brightLP +=
        0.15f *
        (sample - brightLP);

      float high =
        sample - brightLP;

      return
        sample * 0.75f +
        high * 0.50f;
    }
  }

  return sample;
}

void generateAudio() {
  const float attackStep =
    1.0f /
    (0.008f * SAMPLE_RATE);

  const float releaseStep =
    1.0f /
    (0.030f * SAMPLE_RATE);

  uint32_t currentKeys =
    pressedMask;

  uint8_t currentWave =
    waveType;

  uint8_t currentTone =
    toneType;

  uint32_t newlyPressed =
    currentKeys &
    ~previousMask;

  for (
    uint8_t key = 0;
    key < NUM_KEYS;
    key++
  ) {
    if (
      newlyPressed &
      (1UL << key)
    ) {
      phases[key] = 0.0f;
    }
  }

  previousMask =
    currentKeys;

  for (
    uint16_t frame = 0;
    frame < AUDIO_FRAMES;
    frame++
  ) {
    float mix = 0.0f;
    uint8_t voices = 0;

    for (
      uint8_t key = 0;
      key < NUM_KEYS;
      key++
    ) {
      bool pressed =
        currentKeys &
        (1UL << key);

      if (pressed) {
        gains[key] +=
          attackStep;

        if (
          gains[key] >
          1.0f
        ) {
          gains[key] =
            1.0f;
        }
      } else {
        gains[key] -=
          releaseStep;

        if (
          gains[key] <
          0.0f
        ) {
          gains[key] =
            0.0f;
        }
      }

      if (
        gains[key] >
        0.0001f
      ) {
        float wave =
          generateWave(
            currentWave,
            phases[key]
          );

        mix +=
          wave *
          gains[key];

        voices++;

        phases[key] +=
          TWO_PI_F *
          notes[key] /
          SAMPLE_RATE;

        if (
          phases[key] >=
          TWO_PI_F
        ) {
          phases[key] -=
            TWO_PI_F;
        }
      }
    }

    if (voices > 1) {
      mix /= voices;
    }

    mix =
      processTone(
        mix,
        currentTone
      );

    mix =
      constrain(
        mix,
        -1.0f,
        1.0f
      );

    int16_t scopeSample =
      static_cast<int16_t>(
        mix *
        32767.0f
      );

    scopeBuffer[
      scopeWriteIndex
    ] =
      scopeSample;

    scopeWriteIndex++;

    if (
      scopeWriteIndex >=
      SCOPE_BUFFER_SIZE
    ) {
      scopeWriteIndex = 0;
    }

    int16_t sample =
      static_cast<int16_t>(
        mix *
        MASTER_VOLUME *
        32767.0f
      );

    audioBuffer[
      frame * 2
    ] =
      sample;

    audioBuffer[
      frame * 2 + 1
    ] =
      sample;
  }

  I2S.write(
    reinterpret_cast<uint8_t*>(
      audioBuffer
    ),
    sizeof(audioBuffer)
  );
}

void audioTask(
  void* parameter
) {
  while (true) {
    generateAudio();
  }
}

uint8_t countVoices(
  uint32_t mask
) {
  uint8_t voices = 0;

  for (
    uint8_t key = 0;
    key < NUM_KEYS;
    key++
  ) {
    if (
      mask &
      (1UL << key)
    ) {
      voices++;
    }
  }

  return voices;
}

float lowestFrequency(
  uint32_t mask
) {
  for (
    uint8_t key = 0;
    key < NUM_KEYS;
    key++
  ) {
    if (
      mask &
      (1UL << key)
    ) {
      return notes[key];
    }
  }

  return 0.0f;
}

void drawStaticScreen() {
  tft.fillScreen(
    ST77XX_BLACK
  );

  tft.setTextWrap(false);

  tft.drawRect(
    SCOPE_X,
    SCOPE_Y,
    SCOPE_W,
    SCOPE_H,
    ST77XX_WHITE
  );

  tft.setTextColor(
    ST77XX_WHITE
  );

  tft.setTextSize(1);

  tft.setCursor(
    3,
    SCOPE_Y - 3
  );

  tft.print("+1");

  tft.setCursor(
    12,
    SCOPE_Y +
    SCOPE_H / 2 -
    3
  );

  tft.print("0");

  tft.setCursor(
    3,
    SCOPE_Y +
    SCOPE_H -
    6
  );

  tft.print("-1");
}

uint16_t findTriggeredStart(
  uint16_t endIndex,
  uint16_t sampleSpan
) {
  int32_t start =
    static_cast<int32_t>(
      endIndex
    ) -
    sampleSpan -
    200;

  while (start < 0) {
    start +=
      SCOPE_BUFFER_SIZE;
  }

  uint16_t candidate =
    static_cast<uint16_t>(
      start
    );

  for (
    uint16_t i = 1;
    i < 200;
    i++
  ) {
    uint16_t previousIndex =
      (candidate + i - 1) %
      SCOPE_BUFFER_SIZE;

    uint16_t currentIndex =
      (candidate + i) %
      SCOPE_BUFFER_SIZE;

    int16_t previous =
      scopeBuffer[
        previousIndex
      ];

    int16_t current =
      scopeBuffer[
        currentIndex
      ];

    if (
      previous < 0 &&
      current >= 0
    ) {
      return currentIndex;
    }
  }

  return candidate;
}

void drawHeader(
  uint32_t keys,
  uint8_t currentWave,
  uint8_t currentTone
) {
  tft.fillRect(
    0,
    0,
    SCREEN_W,
    29,
    ST77XX_BLACK
  );

  tft.setTextSize(1);

  uint8_t voices =
    countVoices(keys);

  if (voices == 0) {
    tft.setTextColor(
      ST77XX_YELLOW
    );

    tft.setCursor(4, 5);
    tft.print("NO NOTE");
  }

  if (voices == 1) {
    for (
      uint8_t key = 0;
      key < NUM_KEYS;
      key++
    ) {
      if (
        keys &
        (1UL << key)
      ) {
        tft.setTextColor(
          ST77XX_YELLOW
        );

        tft.setCursor(
          4,
          5
        );

        tft.print(
          noteNames[key]
        );

        tft.print(" ");

        tft.print(
          notes[key],
          2
        );

        tft.print("Hz");

        break;
      }
    }
  }

  if (voices > 1) {
    tft.setTextColor(
      ST77XX_YELLOW
    );

    tft.setCursor(4, 5);

    tft.print(voices);
    tft.print(" NOTES");
  }

  tft.setTextColor(
    ST77XX_CYAN
  );

  tft.setCursor(
    245,
    4
  );

  tft.print(
    waveNames[
      currentWave
    ]
  );

  tft.setTextColor(
    ST77XX_MAGENTA
  );

  tft.setCursor(
    245,
    16
  );

  tft.print(
    toneNames[
      currentTone
    ]
  );
}

void drawFrequencyList(
  uint32_t keys
) {
  tft.fillRect(
    0,
    190,
    SCREEN_W,
    50,
    ST77XX_BLACK
  );

  tft.setTextSize(1);

  tft.setTextColor(
    ST77XX_WHITE
  );

  uint8_t active = 0;

  for (
    uint8_t key = 0;
    key < NUM_KEYS;
    key++
  ) {
    if (
      keys &
      (1UL << key)
    ) {
      if (active < 8) {
        uint8_t column =
          active % 4;

        uint8_t row =
          active / 4;

        int16_t x =
          3 +
          column * 79;

        int16_t y =
          194 +
          row * 14;

        tft.setCursor(
          x,
          y
        );

        tft.print(
          notes[key],
          1
        );
      }

      active++;
    }
  }

  if (active == 0) {
    tft.setCursor(
      3,
      194
    );

    tft.print(
      "Frequency: --"
    );
  }

  if (active > 8) {
    tft.setCursor(
      3,
      222
    );

    tft.print("+");
    tft.print(active - 8);
    tft.print(" more");
  }
}

void drawScope(
  uint32_t keys
) {
  tft.fillRect(
    SCOPE_X + 1,
    SCOPE_Y + 1,
    SCOPE_W - 2,
    SCOPE_H - 2,
    ST77XX_BLACK
  );

  uint16_t grid =
    tft.color565(
      40,
      40,
      40
    );

  for (
    uint8_t i = 1;
    i < 4;
    i++
  ) {
    int16_t x =
      SCOPE_X +
      (SCOPE_W * i) /
      4;

    tft.drawFastVLine(
      x,
      SCOPE_Y + 1,
      SCOPE_H - 2,
      grid
    );
  }

  for (
    uint8_t i = 1;
    i < 4;
    i++
  ) {
    int16_t y =
      SCOPE_Y +
      (SCOPE_H * i) /
      4;

    tft.drawFastHLine(
      SCOPE_X + 1,
      y,
      SCOPE_W - 2,
      grid
    );
  }

  int16_t centerY =
    SCOPE_Y +
    SCOPE_H / 2;

  tft.drawFastHLine(
    SCOPE_X + 1,
    centerY,
    SCOPE_W - 2,
    tft.color565(
      90,
      90,
      90
    )
  );

  float frequency =
    lowestFrequency(keys);

  uint16_t sampleSpan =
    700;

  if (
    frequency >
    0.0f
  ) {
    sampleSpan =
      static_cast<uint16_t>(
        SAMPLE_RATE *
        3.0f /
        frequency
      );

    if (
      sampleSpan <
      100
    ) {
      sampleSpan =
        100;
    }

    if (
      sampleSpan >
      1800
    ) {
      sampleSpan =
        1800;
    }
  }

  uint16_t endIndex =
    scopeWriteIndex;

  uint16_t startIndex =
    findTriggeredStart(
      endIndex,
      sampleSpan
    );

  int16_t previousX =
    SCOPE_X + 1;

  int16_t firstSample =
    scopeBuffer[
      startIndex
    ];

  int16_t previousY =
    centerY -
    static_cast<int32_t>(
      firstSample
    ) *
    (SCOPE_H / 2 - 6) /
    32768;

  for (
    int16_t x = 1;
    x < SCOPE_W - 2;
    x++
  ) {
    uint16_t offset =
      static_cast<uint32_t>(
        x
      ) *
      sampleSpan /
      (SCOPE_W - 3);

    uint16_t index =
      (
        startIndex +
        offset
      ) %
      SCOPE_BUFFER_SIZE;

    int16_t value =
      scopeBuffer[
        index
      ];

    int16_t currentX =
      SCOPE_X +
      1 +
      x;

    int16_t currentY =
      centerY -
      static_cast<int32_t>(
        value
      ) *
      (SCOPE_H / 2 - 6) /
      32768;

    tft.drawLine(
      previousX,
      previousY,
      currentX,
      currentY,
      ST77XX_GREEN
    );

    previousX =
      currentX;

    previousY =
      currentY;
  }
}

void drawDisplay() {
  uint32_t now =
    millis();

  if (
    now -
    lastDisplayUpdate <
    50
  ) {
    return;
  }

  lastDisplayUpdate =
    now;

  uint32_t keys =
    pressedMask;

  uint8_t currentWave =
    waveType;

  uint8_t currentTone =
    toneType;

  drawHeader(
    keys,
    currentWave,
    currentTone
  );

  drawScope(keys);

  drawFrequencyList(
    keys
  );
}

void setup() {
  Serial.begin(115200);

  delay(500);

  pinMode(
    SHIFT_DATA,
    OUTPUT
  );

  pinMode(
    SHIFT_CLOCK,
    OUTPUT
  );

  pinMode(
    SHIFT_LATCH,
    OUTPUT
  );

  for (
    uint8_t i = 0;
    i < MATRIX_COLS;
    i++
  ) {
    pinMode(
      COL_PINS[i],
      INPUT
    );
  }

  write595(0x00);

  SPI.begin(
    TFT_SCK,
    -1,
    TFT_MOSI,
    TFT_CS
  );

  tft.init(
    240,
    320
  );

  tft.setRotation(1);

  tft.fillScreen(
    ST77XX_BLACK
  );

  tft.setTextColor(
    ST77XX_WHITE
  );

  tft.setTextSize(2);

  tft.setCursor(
    70,
    100
  );

  tft.print(
    "STARTING..."
  );

  I2S.setPins(
    I2S_BCK,
    I2S_LCK,
    I2S_DIN,
    -1,
    -1
  );

  bool i2sOK =
    I2S.begin(
      I2S_MODE_STD,
      SAMPLE_RATE,
      I2S_DATA_BIT_WIDTH_16BIT,
      I2S_SLOT_MODE_STEREO
    );

  if (!i2sOK) {
    Serial.println(
      "I2S FAILED"
    );

    tft.fillScreen(
      ST77XX_BLACK
    );

    tft.setTextColor(
      ST77XX_RED
    );

    tft.setCursor(
      60,
      100
    );

    tft.print(
      "I2S FAILED"
    );

    while (true) {
      delay(1000);
    }
  }

  drawStaticScreen();

  xTaskCreatePinnedToCore(
    audioTask,
    "Audio",
    4096,
    nullptr,
    3,
    nullptr,
    0
  );

  Serial.println(
    "I2S OK"
  );

  Serial.println(
    "SYNTH READY"
  );

  Serial.println(
    "WAVE: SINE"
  );

  Serial.println(
    "TONE: NORMAL"
  );
}

void loop() {
  updateKeyboard();
  updateControlButtons();
  drawDisplay();

  delay(1);
}