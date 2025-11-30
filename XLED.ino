#include <Wire.h>

struct VibePacket {
  uint8_t db_percent;
  uint8_t bass_percent;
  uint8_t mids_percent;
  uint8_t highs_percent;
  uint8_t vibe_state;
} __attribute__((packed));

volatile VibePacket pkt;

// LED pins for 5-level meter
const int ledPins[5] = {8, 9, 10, 11, 12};

void setup() {
  Serial.begin(9600);

  Wire.begin(0x0B);         // <-- LED SLAVE ADDRESS
  Wire.onReceive(receiveEvent);

  // Initialize LED pins
  for (int i = 0; i < 5; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }
}

void loop() {
  unsigned long now = millis();

  int value = pkt.db_percent;

  // map 50–100 → 0–4
  int level = map(value, 60, 100, 0, 4);
  level = constrain(level, 0, 4);

  // update LEDs
  for (int i = 0; i < 5; i++) {
    digitalWrite(ledPins[i], (i <= level) ? HIGH : LOW);
  }

}

void receiveEvent(int howMany) {
  if (howMany == sizeof(VibePacket)) {
    Wire.readBytes((char*)&pkt, sizeof(VibePacket));
  } else {
    while (Wire.available()) Wire.read();
  }
}
