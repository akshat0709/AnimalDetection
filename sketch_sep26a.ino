#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// -------------------- LCD --------------------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// -------------------- Pin Definitions --------------------

// Ultrasonic sensor
#define TRIG_PIN 9
#define ECHO_PIN 10

// LEDs
#define GREEN_LED 4
#define YELLOW_LED 5
#define RED_LED 6

// Buzzer
#define BUZZER_PIN 7

// PIR sensor (optional)
#define PIR_PIN 8

// Servo motor - used to simulate braking
#define SERVO_PIN 3

Servo brakeServo;

// -------------------- Variables --------------------
long duration;
float distance;

int safeDistance = 100;       // More than 100 cm = SAFE
int warningDistance = 50;     // 50-100 cm = WARNING
int dangerDistance = 50;      // Less than 50 cm = DANGER

bool objectDetected = false;
bool pirDetected = false;

// -------------------- Setup --------------------
void setup() {

  Serial.begin(9600);

  // Ultrasonic
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // LEDs
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);

  // PIR
  pinMode(PIR_PIN, INPUT);

  // Servo
  brakeServo.attach(SERVO_PIN);
  brakeServo.write(0);

  // LCD
  lcd.init();
  lcd.backlight();

  // Startup screen
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Animal Detection");
  lcd.setCursor(0, 1);
  lcd.print("System Starting");

  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  delay(1000);

  lcd.clear();
}

// -------------------- Main Loop --------------------
void loop() {

  // Get distance
  distance = getDistance();

  // Read PIR
  pirDetected = digitalRead(PIR_PIN);

  // Print distance to Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // -------------------- SAFE --------------------
  if (distance > safeDistance) {

    objectDetected = false;

    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);

    noTone(BUZZER_PIN);

    // Release brake
    brakeServo.write(0);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("TRACK: CLEAR");

    lcd.setCursor(0, 1);
    lcd.print("Dist:");
    lcd.print(distance, 0);
    lcd.print("cm");

    Serial.println("STATUS: TRACK CLEAR");
  }

  // -------------------- WARNING --------------------
  else if (distance > dangerDistance &&
           distance <= warningDistance + 50) {

    objectDetected = true;

    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(RED_LED, LOW);

    // Intermittent warning sound
    tone(BUZZER_PIN, 1000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WARNING!");

    lcd.setCursor(0, 1);
    lcd.print("Object:");
    lcd.print(distance, 0);
    lcd.print("cm");

    Serial.println("STATUS: WARNING");

    // Slight braking
    brakeServo.write(45);

    delay(300);

    noTone(BUZZER_PIN);
  }

  // -------------------- DANGER --------------------
  else {

    objectDetected = true;

    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, HIGH);

    // Strong alarm
    tone(BUZZER_PIN, 2000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("!!! DANGER !!!");

    lcd.setCursor(0, 1);
    lcd.print("STOP ");
    lcd.print(distance, 0);
    lcd.print("cm");

    Serial.println("STATUS: DANGER - STOP");

    // Full braking simulation
    brakeServo.write(90);

    delay(500);

    noTone(BUZZER_PIN);
  }

  // -------------------- PIR Detection --------------------
  if (pirDetected) {

    Serial.println("PIR: MOVEMENT DETECTED");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("MOVEMENT");
    lcd.setCursor(0, 1);
    lcd.print("DETECTED!");

    digitalWrite(RED_LED, HIGH);

    tone(BUZZER_PIN, 2000);

    brakeServo.write(90);

    delay(1000);

    noTone(BUZZER_PIN);
  }

  delay(200);
}

// -------------------- Ultrasonic Function --------------------
float getDistance() {

  // Make sure trigger is LOW
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // Send 10 microsecond pulse
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read echo
  duration = pulseIn(ECHO_PIN, HIGH, 30000);

  // If no echo
  if (duration == 0) {
    return 999;
  }

  // Calculate distance
  float calculatedDistance = duration * 0.0343 / 2;

  return calculatedDistance;
}