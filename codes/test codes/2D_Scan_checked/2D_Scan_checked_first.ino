#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <Arduino.h>
#include "A4988.h"

#define STEPS_PER_REVOLUTION 200
#define ANGLE_INCREMENT 1.8

int Step = 10; //GPIO14---D5 of Nodemcu--Step of stepper motor driver
int Dire  = 11; //GPIO2---D4 of Nodemcu--Direction of stepper motor driver
int Sleep = 14; //GPIO12---D6 of Nodemcu-Control Sleep Mode on A4988
int MS1 = 13; //GPIO13---D7 of Nodemcu--MS1 for A4988
int MS2 = 16; //GPIO16---D0 of Nodemcu--MS2 for A4988
int MS3 = 15; //GPIO15---D8 of Nodemcu--MS3 for A4988

//Motor Specs
const int spr = 200; //Steps per revolution
int RPM = 100; //Motor Speed in revolutions per minute
int Microsteps = 1; //Stepsize (1 for full steps, 2 for half steps, 4 for quarter steps, etc)
 
//Providing parameters for motor control
A4988 stepper(spr, Dire, Step, MS1, MS2, MS3);

Adafruit_VL53L0X lox = Adafruit_VL53L0X();
//Stepper stepper(STEPS_PER_REVOLUTION, 2, 3, 4, 5);  // Adjust pin numbers accordingly

const int num_measurements = STEPS_PER_REVOLUTION;
float measurements[num_measurements][2];

String arrayToString(int arr[], int size) {
  String result = "{";
  for (int i = 0; i < size; i++) {
    result += String(arr[i]);
    if (i < size - 1) {
      result += ", ";
    }
  }
  result += "}";
  return result;
}

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600); // Use Serial1 for communication on Arduino Mega

  pinMode(Step, OUTPUT); //Step pin as output
  pinMode(Dire,  OUTPUT); //Direcction pin as output
  pinMode(Sleep,  OUTPUT); //Set Sleep OUTPUT Control button as output
  digitalWrite(Step, LOW); // Currently no stepper motor movement
  digitalWrite(Dire, LOW);
  
  // Set target motor RPM to and microstepping setting
  //stepper.begin(RPM, Microsteps);

  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while (1);
  }
  pinMode(13, OUTPUT);  // Set pin 13 as an output

//  stepper.setSpeed(500);  // Adjust the speed as needed

  Serial.println(F("VL53L0X test with Stepper Motor"));
}

void loop() {
  digitalWrite(12, HIGH);
  digitalWrite(Sleep, HIGH); //A logic high allows normal operation of the A4988 by removing from sleep
  stepper.rotate(360);

  for (int i = 0; i < num_measurements; i++) {
    // Rotate stepper motor by ANGLE_INCREMENT degrees
  //  stepper.step(ANGLE_INCREMENT);

    // Take distance measurement
    VL53L0X_RangingMeasurementData_t measure;
    lox.rangingTest(&measure, false);

    if (measure.RangeStatus != 4) {  // phase failures have incorrect data
      measurements[i][0] = i * ANGLE_INCREMENT;
      measurements[i][1] = measure.RangeMilliMeter;
    } else {
      measurements[i][0] = i * ANGLE_INCREMENT;
      measurements[i][1] = 10000;
    }

    // No delay or minimal delay between steps
  }

  // Print the 2D array after 360 degrees rotation
  Serial.print("aaa[");
  for (int i = 0; i < num_measurements; i++) {
    Serial.print("[");
    Serial.print(measurements[i][0]);
    Serial.print(",");
    Serial.print(measurements[i][1]);
    Serial.print("],");
  }
  Serial.println("]");
  //Serial.println(arrayToString());

  // Reset the stepper motor to its initial position
//  stepper.step(-360 * STEPS_PER_REVOLUTION / 360);
  digitalWrite(13, LOW);
  
  // Delay before starting a new measurement
  delay(5000);
}