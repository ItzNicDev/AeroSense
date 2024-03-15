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
bool performServoCheck = false;

float servoMap[4];
float oldServoMap[4];

int commandRateLimiter = 1; // in ms
int buttonState = HIGH;     // Variable to hold the button state
int oldButtonState = HIGH;  // Variable to hold the previous button state
int count = 0;              // Variable to hold the count
bool buttonPressed = false; // Flag to indicate button press
int sensorMode = 1;         // default mode
bool displayInfo = true;    // show all info

long _map(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int mapper(float value)
{
  return _map(value, -10.0, 10.0, 0.0, 180.0);
}

void blinkingLed(int port)
{
  for (int i = 0; i < 2; i++)
  {
    digitalWrite(port, HIGH);
    delay(200);
    digitalWrite(port, LOW);
    delay(200);
  }
}

void auditoryFeedback(auditoryCodes code)
{
  switch (code)
  {
  case SUCCESS:
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

  case ERROR:
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

void servoSteer(float servoMap[4])
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
  case GREEN:
    blinkingLed(2);
    auditoryFeedback(SUCCESS);
    break;

  case RED:
    while (1)
    {
      auditoryFeedback(ERROR);
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
  if (buttonState == LOW && oldButtonState == HIGH)
  {
    buttonPressed = true;
  }
  else
  {
    buttonPressed = false;
  }

  oldButtonState = buttonState;

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

bool stateHasChanged()
{
  return (oldServoMap[0] != servoMap[0] || oldServoMap[1] != servoMap[1] || oldServoMap[2] != servoMap[2] || oldServoMap[3] || servoMap[3]);
}

void attachPorts()
{
  pinMode(5, INPUT);            // button
  pinMode(ledPorts[0], OUTPUT); // red-led
  pinMode(ledPorts[1], OUTPUT); // green-led

  s1.attach(servoPorts[0]); // servo-1
  s2.attach(servoPorts[1]); // servo-2
  s3.attach(servoPorts[2]); // servo-3
  s4.attach(servoPorts[3]); // servo-4
}

void setup(void)
{
  Serial.begin(115200);

  attachPorts();

  if (!mpu.begin())
  {
    ledInfo(RED);
    info("Failed to find MPU6050 chip");
    while (1)
    {
      delay(10);
    }
  }
  else
  {
    ledInfo(GREEN);
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

  if (sensorMode == 1)
  {
    servoMap[0] = a.acceleration.y;
    servoMap[1] = a.acceleration.x * -1;
    servoMap[2] = a.acceleration.y * -1;
    servoMap[3] = a.acceleration.x;
  }
  else if (sensorMode == 2)
  {
    servoMap[0] = g.gyro.z;
    servoMap[1] = g.gyro.z;
    servoMap[2] = g.gyro.z;
    servoMap[3] = g.gyro.z;
  }
  else if (sensorMode == 3)
  {
    servoMap[0] = (g.gyro.x) + g.gyro.z;
    servoMap[1] = (g.gyro.y * -1) + g.gyro.z;
    servoMap[2] = (g.gyro.x * -1) + g.gyro.z;
    servoMap[3] = (g.gyro.y) + g.gyro.z;
  }

  else if (sensorMode == 4)
  {
    servoMap[0] = (a.acceleration.y) + g.gyro.z;
    servoMap[1] = (a.acceleration.x * -1) + g.gyro.z;
    servoMap[2] = (a.acceleration.y * -1) + g.gyro.z;
    servoMap[3] = (a.acceleration.x) + g.gyro.z;
  }

  if (stateHasChanged())
  {
    servoSteer(servoMap);
  }

  oldServoMap[0] = servoMap[0];
  oldServoMap[1] = servoMap[1];
  oldServoMap[2] = servoMap[2];
  oldServoMap[3] = servoMap[3];

  delay(commandRateLimiter);
}