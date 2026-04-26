/*
// #include <ESP32Servo.h>
#include <Arduino.h>

#define ENCODER_A 35
#define ENCODER_B 34
#define SERVO_PIN 25

volatile long encoderCount = 0;
volatile uint8_t lastEncoded = 0;

void IRAM_ATTR updateEncoder() {

  uint8_t MSB = digitalRead(ENCODER_A);
  uint8_t LSB = digitalRead(ENCODER_B);

  uint8_t encoded = (MSB << 1) | LSB;
  uint8_t sum = (lastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderCount++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderCount--;

  lastEncoded = encoded;
}

void setup() {

  Serial.begin(9600);

  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENCODER_A), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_B), updateEncoder, CHANGE);

  Serial.println("System Ready");
  Serial.println("Commands: S<number>  (example: S90)");
}

void loop() {

  static long lastCount = 0;

  // --- Encoder Output ---
  if (lastCount != encoderCount) {

    long count;
    noInterrupts();
    count = encoderCount;
    interrupts();

    float revolutions = count / 4096.0;
    float degrees = revolutions * 360.0;

    Serial.print("Count: ");
    Serial.print(count);
    Serial.print("  Rev: ");
    Serial.print(revolutions, 4);
    Serial.print("  Deg: ");
    Serial.println(degrees, 2);

    lastCount = count;
  }
}
*/