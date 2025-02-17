#include "TRSensors.h"
 
const int LEFT_IN1 = A0;
const int LEFT_IN2 = A1;
const int LEFT_ENA = 5;
const int RIGHT_IN1 = A2;
const int RIGHT_IN2 = A3;
const int RIGHT_ENB = 6;
 
#define NUM_SENSORS 5
TRSensors trs = TRSensors();
unsigned int sensorValues[NUM_SENSORS];
 
int baseSpeed = 120; 
int turnAdjust = 50; 
bool lineLost = false;
 
#define LOOP_DELAY 10 
unsigned long lastLoopTime = 0;
unsigned long lineLostTime = 0; 
const unsigned long lineLostThreshold = 30; 
 
bool lastSeenLeft = true; 
 
void setup() {
  Serial.begin(9600);
  pinMode(LEFT_IN1, OUTPUT);
  pinMode(LEFT_IN2, OUTPUT);
  pinMode(LEFT_ENA, OUTPUT);
  pinMode(RIGHT_IN1, OUTPUT);
  pinMode(RIGHT_IN2, OUTPUT);
  pinMode(RIGHT_ENB, OUTPUT);
 
  delay(3000); 
  calibrateSensors();
}
 
void loop() {
  if (millis() - lastLoopTime >= LOOP_DELAY) {
    lastLoopTime = millis();
 
    int position = trs.readLine(sensorValues);
 
    bool lineDetected = false;
    for (int i = 0; i < NUM_SENSORS; i++) {
      if (sensorValues[i] > 200) { 
        lineDetected = true;
        break;
      }
    }
 
    if (lineDetected) {
      lineLost = false;
      lineLostTime = 0; 
 
      if (position < 2500) {
        lastSeenLeft = true; 
      } else if (position > 2500) {
        lastSeenLeft = false; 
      }
    } else {
      if (lineLostTime == 0) {
        lineLostTime = millis(); 
      }
 
      if (millis() - lineLostTime >= lineLostThreshold) {
        lineLost = true; 
      }
    }
 
    if (lineLost) {
      searchForLine();
    } else {
      int motorSpeedLeft = baseSpeed;
      int motorSpeedRight = baseSpeed;
 
      if (position < 1500) {
        motorSpeedLeft -= (sensorValues[0] < 200) ? turnAdjust * 2 : turnAdjust;
      } else if (position > 2500) {
        motorSpeedRight -= (sensorValues[4] < 200) ? turnAdjust * 2 : turnAdjust;
      }
 
      motorSpeedLeft = constrain(motorSpeedLeft, 0, 255);
      motorSpeedRight = constrain(motorSpeedRight, 0, 255);
 
      controlLeftMotor(motorSpeedLeft);
      controlRightMotor(motorSpeedRight);
    }
  }
}
 
void searchForLine() {
  const int searchSpeed = 100; 
  const int maxSearchTime = 5000; 
  unsigned long startTime = millis();

  while (millis() - startTime < maxSearchTime) {
    int position = trs.readLine(sensorValues);
    bool lineDetected = false;
 
    for (int i = 0; i < NUM_SENSORS; i++) {
      if (sensorValues[i] > 200) { 
        lineDetected = true;
        break;
      }
    }
 
    if (lineDetected) {
      lineLost = false; 
      return;          
    }
 
    if (lastSeenLeft) {
      controlLeftMotor(-searchSpeed);
      controlRightMotor(searchSpeed);
    } else {
      controlLeftMotor(searchSpeed);
      controlRightMotor(-searchSpeed);
    }
 
    delay(100); 
  }
 
  controlLeftMotor(0);
  controlRightMotor(0);
}
 
void calibrateSensors() {
  for (int i = 0; i < 400; i++) {
    trs.calibrate();
    delay(25);
  }
}
 
void controlLeftMotor(int speed) {
  if (speed > 0) {
    digitalWrite(LEFT_IN1, HIGH);
    digitalWrite(LEFT_IN2, LOW);
    analogWrite(LEFT_ENA, speed);
  } else {
    digitalWrite(LEFT_IN1, LOW);
    digitalWrite(LEFT_IN2, HIGH);
    analogWrite(LEFT_ENA, -speed);
  }
}
 
void controlRightMotor(int speed) {
  if (speed > 0) {
    digitalWrite(RIGHT_IN1, LOW);
    digitalWrite(RIGHT_IN2, HIGH);
    analogWrite(RIGHT_ENB, speed);
  } else {
    digitalWrite(RIGHT_IN1, HIGH);
    digitalWrite(RIGHT_IN2, LOW);
    analogWrite(RIGHT_ENB, -speed);
  }
}

//kod ok