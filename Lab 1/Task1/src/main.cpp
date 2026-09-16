#include <Arduino.h>

// put function declarations here:
#define led 2

void setup()
{
  // put your setup code here, to run once:
  pinMode(led, OUTPUT);
}

void loop()
{
  digitalWrite(led, HIGH);
  delay(1000);
  digitalWrite(led, LOW);
  delay(1000);
  // put your main code here, to run repeatedly:
}
