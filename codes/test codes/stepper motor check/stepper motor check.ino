#include <AccelStepper.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>

// Define stepper motor connections
#define STEP_PIN 10
#define DIR_PIN 11

#define STEP_PIN_2 12
#define DIR_PIN_2 13

#define STEPS_PER_REVOLUTION 200
#define ANGLE_INCREMENT 1.8

// Create a stepper object
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);
AccelStepper stepper2(AccelStepper::DRIVER, STEP_PIN_2, DIR_PIN_2);

void setup() {
  // Set up the speed and acceleration of the stepper motor
  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(500);

  stepper2.setMaxSpeed(1000);
  stepper2.setAcceleration(500);

  // Set the motor to run continuously
  stepper.moveTo(0);
  stepper2.moveTo(0);
}

void loop() {
  // Run the stepper motor
  stepper.run();
  stepper2.run();
}
