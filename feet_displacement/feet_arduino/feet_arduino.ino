#include <Wire.h>

// MPU9250 registers and constants
#define MPU9250_ADDRESS 0x68
#define ACCEL_XOUT_H 0x3B
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40
#define GYRO_XOUT_H 0x43
#define GYRO_XOUT_L 0x44
#define GYRO_YOUT_H 0x45
#define GYRO_YOUT_L 0x46
#define GYRO_ZOUT_H 0x47
#define GYRO_ZOUT_L 0x48

int16_t accel_x, accel_y, accel_z;
int16_t gyro_x, gyro_y, gyro_z;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  // Initialize MPU9250
  Wire.beginTransmission(MPU9250_ADDRESS);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0);    // Wake up the MPU-9250
  Wire.endTransmission(true);
}

void loop() {
  // Read accelerometer data
  Wire.beginTransmission(MPU9250_ADDRESS);
  Wire.write(ACCEL_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU9250_ADDRESS, 6, true);
  
  accel_x = Wire.read() << 8 | Wire.read();
  accel_y = Wire.read() << 8 | Wire.read();
  accel_z = Wire.read() << 8 | Wire.read();
  
  // Read gyroscope data
  Wire.beginTransmission(MPU9250_ADDRESS);
  Wire.write(GYRO_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU9250_ADDRESS, 6, true);
  
  gyro_x = Wire.read() << 8 | Wire.read();
  gyro_y = Wire.read() << 8 | Wire.read();
  gyro_z = Wire.read() << 8 | Wire.read();

  // Calculate 3D displacement
  float dispX = accel_x * pow(millis(), 2) / 2.0;
  float dispY = accel_y * pow(millis(), 2) / 2.0;
  float dispZ = accel_z * pow(millis(), 2) / 2.0;

  // Print the displacement values
  Serial.print("Displacement (X, Y, Z): ");
  Serial.print(dispX);
  Serial.print(", ");
  Serial.print(dispY);
  Serial.print(", ");
  Serial.println(dispZ);

  delay(100); // Adjust the delay as needed
}
