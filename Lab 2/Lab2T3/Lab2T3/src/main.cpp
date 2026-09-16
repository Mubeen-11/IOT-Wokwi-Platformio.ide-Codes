#include <Arduino.h>

#define ROLL_NUMBER 48

#define TRIG_PIN 5
#define ECHO_PIN 18

#define LED_GREEN 2
#define LED_YELLOW 4
#define LED_RED 16
#define BUZZER_PIN 17

float dStop = 15.0 + (ROLL_NUMBER % 10);

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

void resetAlerts()
{
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
  noTone(BUZZER_PIN);
}

void setup()
{
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
}

void loop()
{
  float distance = readUltrasonicDistance(TRIG_PIN, ECHO_PIN);
  resetAlerts();

  String parkingState = "";

  if (distance > (dStop + 30.0))
  {
    parkingState = "SAFE";
    digitalWrite(LED_GREEN, HIGH);
    delay(500);
  }
  else if ((distance > (dStop + 15.0)) && (distance <= (dStop + 30.0)))
  {
    parkingState = "CAUTION";
    digitalWrite(LED_YELLOW, HIGH);
    tone(BUZZER_PIN, 1000, 100);
    delay(900);
  }
  else if ((distance > dStop) && (distance <= (dStop + 15.0)))
  {
    parkingState = "WARNING";
    digitalWrite(LED_RED, HIGH);
    tone(BUZZER_PIN, 1000, 100);
    delay(100);
  }
  else
  {
    parkingState = "CRITICAL STOP";
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_YELLOW, HIGH);
    digitalWrite(LED_RED, HIGH);
    tone(BUZZER_PIN, 2000);
    delay(200);
  }

  Serial.print("Distance (cm): ");
  Serial.println(distance);

  Serial.print("Stopping Limit (cm): ");
  Serial.println(dStop);

  Serial.print("Parking State: ");
  Serial.println(parkingState);

  Serial.println("---------------------------------------------");
}