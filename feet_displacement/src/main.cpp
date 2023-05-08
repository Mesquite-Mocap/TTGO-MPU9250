#include <Wire.h>
#include <MPU9250.h>

MPU9250 mpu;

void setup() {
  Serial.begin(115200);
  
  Wire.begin();
  mpu.initialize();
  
  // Calibrate the sensor
  mpu.calibrateAccelGyro();
  mpu.setThreshold(3); // Adjust the motion detection threshold if needed
}

void loop() {
  // Read accelerometer and gyroscope data
  mpu.update();

  // Get 3D displacement
  float dispX = mpu.getAccX_mss() * pow(mpu.getDeltaTime(), 2) / 2.0;
  float dispY = mpu.getAccY_mss() * pow(mpu.getDeltaTime(), 2) / 2.0;
  float dispZ = mpu.getAccZ_mss() * pow(mpu.getDeltaTime(), 2) / 2.0;

  // Print the displacement values
  Serial.print("Displacement (X, Y, Z): ");
  Serial.print(dispX);
  Serial.print(", ");
  Serial.print(dispY);
  Serial.print(", ");
  Serial.println(dispZ);
  
  delay(100); // Adjust the delay as needed
}
