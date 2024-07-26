#include <Adafruit_VL53L0X.h>
#include <Adafruit_VL53L0X.h>

#define STEP_PIN_1 24
#define DIR_PIN_1 23
#define ENA_PIN_1 22

#define STEP_PIN_2 27
#define DIR_PIN_2 26
#define ENA_PIN_2 25

#define XY_REV 200
#define XZ_REV 50

#define ANGLE_INCREMENT 1.8


Adafruit_VL53L0X lox1 = Adafruit_VL53L0X();

float measurements[XZ_REV][3];

void tof(uint8_t bus) {
  Wire.beginTransmission(0x70);  // TCA9548A address is 0x70
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
}

void setup() {
  Serial.begin(9600);  // Initialize the serial monitor
  Wire.begin();

  tof(1);

  if (!lox1.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while (1);
  }
  pinMode(STEP_PIN_1,OUTPUT);
  pinMode(DIR_PIN_1,OUTPUT);
  pinMode(ENA_PIN_1,OUTPUT);

  pinMode(STEP_PIN_2,OUTPUT);
  pinMode(DIR_PIN_2,OUTPUT);
  pinMode(ENA_PIN_2,OUTPUT);

  digitalWrite(ENA_PIN_1, LOW);
  digitalWrite(ENA_PIN_2, LOW);

  digitalWrite(DIR_PIN_2, LOW);
  for (int i=0; i<XZ_REV/2; i++) {
    digitalWrite(STEP_PIN_2, HIGH);
    digitalWrite(STEP_PIN_2, LOW);
    delayMicroseconds(50);
  }

  Serial.println(F("VL53L0X test with Stepper Motor"));
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i=0; i < XY_REV; i++) {
    if (i%2 == 0) {
      digitalWrite(DIR_PIN_2,HIGH);
    } else {
      digitalWrite(DIR_PIN_2,LOW);
    }
    for (int j=0; j < XZ_REV; j++) {
      VL53L0X_RangingMeasurementData_t measure;
      lox1.rangingTest(&measure, false);

      if (i%2 == 0) {
        if (measure.RangeStatus != 4) {  // phase failures have incorrect data
        measurements[j][0] = j * ANGLE_INCREMENT - 45;
        measurements[j][1] = i * ANGLE_INCREMENT;
        measurements[j][2] = measure.RangeMilliMeter;
      } else {
        measurements[j][0] = j * ANGLE_INCREMENT - 45;
        measurements[j][1] = i * ANGLE_INCREMENT;
        measurements[j][2] = 10000;
      }
      } else {
        if (measure.RangeStatus != 4) {  // phase failures have incorrect data
        measurements[XZ_REV - j -1][0] = j * ANGLE_INCREMENT - 45;
        measurements[XZ_REV - j - 1][1] = i * ANGLE_INCREMENT;
        measurements[XZ_REV - j - 1][2] = measure.RangeMilliMeter;
      } else {
        measurements[XZ_REV - j - 1][0] = j * ANGLE_INCREMENT - 45;
        measurements[XZ_REV - j - 1][1] = i * ANGLE_INCREMENT;
        measurements[XZ_REV - j - 1][2] = 10000;
      }
      }
      digitalWrite(STEP_PIN_2, HIGH);
      digitalWrite(STEP_PIN_2, LOW);
    }
    Serial.print("aaa[");
  for (int k = 0; k < XZ_REV; k++) {
    Serial.print("[");
    Serial.print(measurements[k][0]);
    Serial.print(",");
    Serial.print(measurements[k][1]);
    Serial.print(",");
    Serial.print(measurements[k][2]);
    Serial.print("],");
    }
  Serial.println("]");
  digitalWrite(STEP_PIN_1, HIGH);
  digitalWrite(STEP_PIN_1, LOW);
  }

}