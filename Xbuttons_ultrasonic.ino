#include <Wire.h>

#define MY_I2C_ADDRESS 0x08
#define ENABLE_ULTRASONIC true

// ========== PIN DEFINITIONS ==========
const int buttonPins[5] = {7, 6, 5, 4, 3};  // Buttons 1-5
const int trigPin = 8;
const int echoPin = 9;

// ======= GLOBAL VARIABLES =======
volatile uint8_t lastButtonPressed = 0; // 1-5 or 0
volatile int lastDistance = 0;          // cm

// ======= BUTTON DEBOUNCE =======
unsigned long lastDebounceTime[5] = {0,0,0,0,0};
const unsigned long debounceDelay = 50;
int buttonState[5] = {HIGH,HIGH,HIGH,HIGH,HIGH};
int lastButtonState[5] = {HIGH,HIGH,HIGH,HIGH,HIGH};

// ======= ULTRASONIC =======
#if ENABLE_ULTRASONIC
unsigned long lastUltrasonicRead = 0;
long duration;
int distance;
#endif

// ======= I2C COMMAND =======
volatile uint8_t i2cCommand = 0; // store last command from master

// ======= SETUP =======
void setup() {
  Serial.begin(115200);
  
  // Initialize I2C as SLAVE
  Wire.begin(MY_I2C_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  
  // Buttons
  for (int i=0;i<5;i++) pinMode(buttonPins[i], INPUT_PULLUP);
  
  // Ultrasonic
  #if ENABLE_ULTRASONIC
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  #endif
  
  Serial.print("Arduino I2C Slave Ready @ 0x");
  Serial.println(MY_I2C_ADDRESS, HEX);
}

// ======= LOOP =======
void loop() {
  handleButtons();
  
  #if ENABLE_ULTRASONIC
  handleUltrasonic();
  #endif
  
  delay(10); // Small delay to avoid flooding Serial
}

// ======= BUTTON HANDLER =======
void handleButtons() {
  for(int i=0;i<5;i++){
    int reading = digitalRead(buttonPins[i]);
    if(reading != lastButtonState[i]) lastDebounceTime[i] = millis();
    
    if((millis() - lastDebounceTime[i]) > debounceDelay){
      if(reading == LOW && buttonState[i] == HIGH){
        lastButtonPressed = i+1;
        Serial.print("Button ");
        Serial.print(i+1);
        Serial.println(" pressed");
      }
      buttonState[i] = reading;
    }
    lastButtonState[i] = reading;
  }
}

// ======= ULTRASONIC =======
#if ENABLE_ULTRASONIC
void handleUltrasonic(){
  unsigned long now = millis();
  if(now - lastUltrasonicRead >= 200){
    lastUltrasonicRead = now;
    
    digitalWrite(trigPin, LOW); delayMicroseconds(2);
    digitalWrite(trigPin, HIGH); delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    duration = pulseIn(echoPin,HIGH);
    distance = duration*0.0343/2;
    lastDistance = distance;
    
    Serial.print("Distance: "); Serial.print(distance); Serial.println(" cm");
  }
}
#endif

// Called when ESP32 sends a byte (command)
void receiveEvent(int howMany){
  if(howMany >= 1){
    i2cCommand = Wire.read(); // 1 = button, 2 = distance
    while(Wire.available()) Wire.read(); // flush extra
  }
}

// Called when ESP32 requests data
void requestEvent(){
  if(i2cCommand == 1){
    Wire.write(lastButtonPressed);
    lastButtonPressed = 0;
  }
  else if(i2cCommand == 2){
    Wire.write((uint8_t)(lastDistance & 0xFF));        // LSB
    Wire.write((uint8_t)((lastDistance >> 8) & 0xFF)); // MSB
  }
  i2cCommand = 0; // reset
}
