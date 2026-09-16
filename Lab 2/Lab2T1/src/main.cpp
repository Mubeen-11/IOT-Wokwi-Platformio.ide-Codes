#include <Arduino.h>
#include <DHT.h>
#include <ESP32Servo.h>

#define adc 36
#define led 23
#define alarm 22
float percentage = 68.0;

float readvoltage(int pin)
{
    int rawvalue = analogRead(pin);
    return rawvalue * (3.3 / 4095.0);
}

float readTank(int pin)
{
    int rawvalue = analogRead(pin);
    return (rawvalue / 4095.0) * 100.0;
}

void setup()
{
    Serial.begin(115200);
    pinMode(led, OUTPUT);
    pinMode(alarm, OUTPUT);
    Serial.println("Task 1 Loaded");
}

void loop()
{
    float level = readTank(adc);
    float volt = readvoltage(adc);
    if (level > percentage)
    {
        digitalWrite(led, HIGH);
        digitalWrite(alarm, HIGH);
        Serial.println(level);
    }
    else
    {
        digitalWrite(led, LOW);
        digitalWrite(alarm, LOW);
        Serial.println(level);
    }
    delay(500);
}
