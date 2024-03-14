#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Servo.h>
#include "ServoTesting.h"

Adafruit_MPU6050 mpu;
Servo s1;
Servo s2;
Servo s3;
Servo s4;

sensors_event_t a, g, temp;
int servoPorts[4] = {12, 11, 10, 9}; // ports used by the servos
int ledPorts[2] = {2, 3};            // ports used by status-LED
bool performServoCheck = true;
int servoMap[4];
int commandRateLimiter = 1; // in ms
int buttonState = HIGH;     // Variable to hold the button state
int lastButtonState = HIGH; // Variable to hold the previous button state
int count = 0;              // Variable to hold the count
bool buttonPressed = false; // Flag to indicate button press
int sensorMode = 1;         // default mode
bool displayInfo = true;    // show all info

int mapper(float value)
{
  return map(value, -10, 10, 0, 180);
}

void blinkingLed(int port)
{
  digitalWrite(port, HIGH);
  delay(200);
  digitalWrite(port, LOW);
  delay(200);
  digitalWrite(port, HIGH);
  delay(200);
  digitalWrite(port, LOW);
}

void auditoryFeedback(auditoryCodes code)
{
  switch (code)
  {
  case success:
    tone(4, 880);
    delay(200);
    noTone(4);
    tone(4, 1318);
    delay(200);
    noTone(4);
    tone(4, 1760);
    delay(200);
    noTone(4);
    break;

  case error:
    tone(4, 1200);
    delay(200);
    noTone(4);
    tone(4, 200);
    delay(200);
    noTone(4);
    tone(4, 200);
    delay(200);
    noTone(4);
    break;
  }
}

void servoSteer(int servoMap[4])
{
  s1.write(mapper(servoMap[0]));
  s2.write(mapper(servoMap[1]));
  s3.write(mapper(servoMap[2]));
  s4.write(mapper(servoMap[3]));
}

void ledInfo(ledColor color)
{
  switch (color)
  {
  case green:
    blinkingLed(2);
    auditoryFeedback(success);
    break;

  case red:
    while (1)
    {
      auditoryFeedback(error);
      blinkingLed(3);
    }
  }
}

void info(String value)
{
  if (displayInfo)
    Serial.println(value);
}

int getSensorMode()
{
  buttonState = digitalRead(5);
  if (buttonState == LOW && lastButtonState == HIGH)
  {
    buttonPressed = true;
  }
  else
  {
    buttonPressed = false;
  }

  lastButtonState = buttonState;

  if (buttonPressed)
  {
    count++;

    if (count == 6)
    {
      count = 1;
    }

    for (int i = 0; i < count; i++)
    {
      tone(4, 880);
      delay(50);
      noTone(4);
      delay(100);
    }

    info(String("Mode: ") + count);
    return count;
  }
}

void setup(void)
{
  Serial.begin(115200);
  pinMode(5, INPUT);            // button
  pinMode(ledPorts[0], OUTPUT); // red-led
  pinMode(ledPorts[1], OUTPUT); // green-led

  s1.attach(servoPorts[0]); // servo-1
  s2.attach(servoPorts[1]); // servo-2
  s3.attach(servoPorts[2]); // servo-3
  s4.attach(servoPorts[3]); // servo-4

  if (!mpu.begin())
  {
    ledInfo(red);
    info("Failed to find MPU6050 chip");
    while (1)
    {
      delay(10);
    }
  }
  else
  {
    ledInfo(green);
    info("Successfuly found MPU6050 chip!");
  }

  if (performServoCheck)
  {
    checkServos();
    info("Successfuly tested servos!");
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_44_HZ);

  delay(1000);
}

void loop()
{
  mpu.getEvent(&a, &g, &temp);

  sensorMode = getSensorMode();

  switch (sensorMode)
  {
  case 1:
    servoMap[0] = a.acceleration.y;
    servoMap[1] = a.acceleration.x * -1;
    servoMap[2] = a.acceleration.y * -1;
    servoMap[3] = a.acceleration.x;
    break;
  case 2:
    servoMap[0] = g.gyro.z;
    servoMap[1] = g.gyro.z;
    servoMap[2] = g.gyro.z;
    servoMap[3] = g.gyro.z;
    break;

  case 3:
    servoMap[0] = (g.gyro.x) + g.gyro.z;
    servoMap[1] = (g.gyro.y * -1) + g.gyro.z;
    servoMap[2] = (g.gyro.x * -1) + g.gyro.z;
    servoMap[3] = (g.gyro.y) + g.gyro.z;
    break;

  case 4:
    servoMap[0] = (a.acceleration.y) + g.gyro.z;
    servoMap[1] = (a.acceleration.x * -1) + g.gyro.z;
    servoMap[2] = (a.acceleration.y * -1) + g.gyro.z;
    servoMap[3] = (a.acceleration.x) + g.gyro.z;
    break;

  case 5:
    servoMap[0] = 0;
    servoMap[1] = 0;
    servoMap[2] = 0;
    servoMap[3] = 0;
    break;
  }

  servoSteer(servoMap);
  delay(commandRateLimiter);
}