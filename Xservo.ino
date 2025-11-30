#include <Wire.h>
#include <Servo.h>

Servo micServo;

struct VibePacket {
  uint8_t db_percent;
  uint8_t bass_percent;
  uint8_t mids_percent;
  uint8_t highs_percent;
  uint8_t vibe_state;
} __attribute__((packed));

volatile VibePacket pkt;   // received packet from master

unsigned long lastUpdate = 0;
const unsigned long updateInterval = 200;  // servo update rate (ms)

void setup() {
  Serial.begin(9600);

  Wire.begin(0x09);  // slave address
  Wire.onReceive(receiveEvent);

  micServo.attach(9);
  micServo.write(0);
}

void loop() {
  unsigned long currentTime = millis();

  if (currentTime - lastUpdate >= updateInterval) {
    lastUpdate = currentTime;

    int value = pkt.db_percent;   // 0–100

    Serial.print("Received dB%: ");
    Serial.println(value);

    // Map 50–100 → 0–4 level
    int level = map(value, 60, 100, 0, 4);
    level = constrain(level, 0, 4);

    // Convert level (0–4) into servo angle
    // Each step = 45°, reversed
    int angle = 180 - (level * 45);

    Serial.print("Level: ");
    Serial.print(level);
    Serial.print("  → Angle: ");
    Serial.println(angle);

    micServo.write(angle);
  }
}

// I²C handler — uses your real packet structure
void receiveEvent(int howMany) {
  if (howMany == sizeof(VibePacket)) {
    Wire.readBytes((char*)&pkt, sizeof(VibePacket));
  } else {
    while (Wire.available()) Wire.read();  // flush garbage
  }
}
