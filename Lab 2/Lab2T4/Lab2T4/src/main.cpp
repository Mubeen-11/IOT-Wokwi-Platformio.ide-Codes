#include <Arduino.h>
#define LDR_PIN 39
#define PIR_PIN 13
#define TRIG_PIN 5
#define ECHO_PIN 18

float readUltrasonicDistance(int trigPin, int echoPin)
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0)
  {
    return 400.0;
  }
  return (duration * 0.0343) / 2.0;
}

void setup()
{
}

void loop()
{
}
