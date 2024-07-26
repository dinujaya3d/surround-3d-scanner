#include <Wire.h>
#include <Adafruit_VL53L0X.h>

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

void setup() {
  Serial.begin(115200);

  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while (1);
  }

  Serial.println(F("VL53L0X test"));
}

void loop() {
  VL53L0X_RangingMeasurementData_t measure;

  lox.rangingTest(&measure, false);

  if (measure.RangeStatus != 4) {  // phase failures have incorrect data
    Serial.print(F("Distance (mm): "));
    Serial.println(measure.RangeMilliMeter);
  } else {
    Serial.println(F("Out of range"));
  }

  delay(100);
}
