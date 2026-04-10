#include "BluetoothSerial.h"
BluetoothSerial SerialBT;

// Motor direction pins
#define IN1 26
#define IN2 27
#define IN3 14
#define IN4 12

// Motor speed control pins
#define ENA 25
#define ENB 33

// LED and Buzzer pins
#define FRONT_LED 4
#define BACK_LED 5
#define HAZARD_LED 18
#define BUZZER 2

// PWM channels
#define LEFT_CHANNEL 0
#define RIGHT_CHANNEL 1
#define FREQ 1000
#define RESOLUTION 8  // 8-bit: 0–255

// State variables
bool manualFrontLedState = false;   // Stores the manual LED toggle state
bool hazardActive = false;
bool hazardState = false;
bool isMoving = false;
bool isReversing = false;
bool backLedManual = false;
bool buzzerState = false;
int motorSpeed = 255;

unsigned long previousMillis = 0;
const long blinkInterval = 500;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("RC CAR");

  // Motor pin modes
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // LED and Buzzer pin modes
  pinMode(FRONT_LED, OUTPUT);
  pinMode(BACK_LED, OUTPUT);
  pinMode(HAZARD_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  // PWM setup
  ledcSetup(LEFT_CHANNEL, FREQ, RESOLUTION);
  ledcSetup(RIGHT_CHANNEL, FREQ, RESOLUTION);
  ledcAttachPin(ENA, LEFT_CHANNEL);
  ledcAttachPin(ENB, RIGHT_CHANNEL);

  // Initial LED states
  digitalWrite(FRONT_LED, LOW);
  digitalWrite(BACK_LED, LOW);
  digitalWrite(HAZARD_LED, LOW);
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  ledcWrite(LEFT_CHANNEL, 0);
  ledcWrite(RIGHT_CHANNEL, 0);
}

void applyFrontLedLogic() {
  if (isMoving && !isReversing) {
    // Auto turn ON when moving forward
    digitalWrite(FRONT_LED, HIGH);
  } else {
    // Restore manual state when stopped or reversing
    digitalWrite(FRONT_LED, manualFrontLedState);
  }
}

void loop() {
  if (SerialBT.available()) {
    char command = SerialBT.read();
    Serial.println(command);

    switch (command) {
      case 'F': // Forward
        isMoving = true;
        isReversing = false;
        digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
        digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
        ledcWrite(LEFT_CHANNEL, motorSpeed);
        ledcWrite(RIGHT_CHANNEL, motorSpeed);
        applyFrontLedLogic();
        break;

      case 'B': // Backward
        isMoving = true;
        isReversing = true;
        digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
        digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
        ledcWrite(LEFT_CHANNEL, motorSpeed);
        ledcWrite(RIGHT_CHANNEL, motorSpeed);
        applyFrontLedLogic();
        break;

      case 'L': // Left
        isMoving = true;
        isReversing = false;
        digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
        digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
        ledcWrite(LEFT_CHANNEL, motorSpeed);
        ledcWrite(RIGHT_CHANNEL, motorSpeed);
        applyFrontLedLogic();
        break;

      case 'R': // Right
        isMoving = true;
        isReversing = false;
        digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
        digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
        ledcWrite(LEFT_CHANNEL, motorSpeed);
        ledcWrite(RIGHT_CHANNEL, motorSpeed);
        applyFrontLedLogic();
        break;

      case 'G': // Forward Right
        isMoving = true;
        isReversing = false;
        digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
        digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
        ledcWrite(LEFT_CHANNEL, 0);
        ledcWrite(RIGHT_CHANNEL, motorSpeed);
        applyFrontLedLogic();
        break;

      case 'I': // Forward Left
        isMoving = true;
        isReversing = false;
        digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
        digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
        ledcWrite(LEFT_CHANNEL, motorSpeed);
        ledcWrite(RIGHT_CHANNEL, 0);
        applyFrontLedLogic();
        break;

      case 'H': // Backward Right
        isMoving = true;
        isReversing = true;
        digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
        digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
        ledcWrite(LEFT_CHANNEL, 0);
        ledcWrite(RIGHT_CHANNEL, motorSpeed);
        applyFrontLedLogic();
        break;

      case 'J': // Backward Left
        isMoving = true;
        isReversing = true;
        digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
        digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
        ledcWrite(LEFT_CHANNEL, motorSpeed);
        ledcWrite(RIGHT_CHANNEL, 0);
        applyFrontLedLogic();
        break;

      case 'S': // Stop
        stopMotors();
        isMoving = false;
        isReversing = false;
        digitalWrite(BACK_LED, LOW);
        applyFrontLedLogic();
        break;

      case 'D': // Stop all and reset speed
        stopMotors();
        isMoving = false;
        isReversing = false;
        motorSpeed = 0;
        digitalWrite(BACK_LED, LOW);
        applyFrontLedLogic();
        break;

      case 'W': // Toggle Front LED manually
        manualFrontLedState = !manualFrontLedState;
        applyFrontLedLogic();
        break;

      case 'U': // Toggle Back LED
        backLedManual = !backLedManual;
        break;

      case 'X': // Toggle Hazard
        hazardActive = !hazardActive;
        hazardState = false;
        digitalWrite(HAZARD_LED, LOW);
        break;

      case 'V': // Toggle Buzzer
        buzzerState = !buzzerState;
        digitalWrite(BUZZER, buzzerState);
        break;

      // Speed control: 0 to 9, ':' = 100%
      case '0'...'9':
        motorSpeed = (command - '0') * 25.5;
        Serial.print("Speed Set: "); Serial.println(motorSpeed);
        break;

      case ':':
        motorSpeed = 255;
        Serial.println("Speed Set: 255 (100%)");
        break;

      default:
        Serial.println("Invalid Command");
        break;
    }
  }

  // Hazard blinking logic
  if (hazardActive && !isMoving) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= blinkInterval) {
      previousMillis = currentMillis;
      hazardState = !hazardState;
      digitalWrite(HAZARD_LED, hazardState);
    }
  }

  if (hazardActive && isMoving) {
    digitalWrite(HAZARD_LED, LOW);
  }

  // Back LED logic
  if (isReversing) {
    digitalWrite(BACK_LED, HIGH);
  } else {
    digitalWrite(BACK_LED, backLedManual ? HIGH : LOW);
  }
}
