#include "servoTesting.h"
#include <Arduino.h>

extern Servo s1; // Declare external variables
extern Servo s2;
extern Servo s3;
extern Servo s4;

void checkServos() {
  s1.write(0);
  s2.write(0);
  s3.write(0);
  s4.write(0);

  delay(1000);

  s1.write(180);
  s2.write(180);
  s3.write(180);
  s4.write(180);

  delay(1000);

  s1.write(0);
  s2.write(0);
  s3.write(0);
  s4.write(0);

  for (int pos = 0; pos < 180; pos++) {
    s1.write(pos);
    s2.write(pos);
    s3.write(pos);
    s4.write(pos);
    delay(10);
  }

  for (int pos = 180; pos > 0; pos--) {
    s1.write(pos);
    s2.write(pos);
    s3.write(pos);
    s4.write(pos);
    delay(10);
  }

  for (int pos = 0; pos < 180; pos++) {
    s1.write(180 - pos);
    s2.write(pos);
    s3.write(180 - pos);
    s4.write(pos);
    delay(10);
  }

  for (int pos = 0; pos < 180; pos++) {
    s1.write(pos);
    s2.write(180 - pos);
    s3.write(pos);
    s4.write(180 - pos);
    delay(10);
  }
}
