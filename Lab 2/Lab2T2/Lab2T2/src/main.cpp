#include <Arduino.h>
#include <DHT.h>

#define DHT_PIN 15
#define DHTTYPE DHT22
#define LDR_PIN 39    
#define PIR_PIN 13    

#define COOLING_PWM_PIN 4
#define COOLING_PWM_CHAN 0

#define LIGHTING_PWM_PIN 2
#define LIGHTING_PWM_CHAN 1

DHT dht(DHT_PIN, DHTTYPE);
float tempThreshold = 25.0; 

void setupPWM(int pin, int channel) {
  ledcSetup(channel, 5000, 8); 
  ledcAttachPin(pin, channel);
}

void setActuatorPower(int channel, int powerPercent) {
  int dutyCycle = map(powerPercent, 0, 100, 0, 255);
  ledcWrite(channel, dutyCycle);
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(PIR_PIN, INPUT);

  setupPWM(COOLING_PWM_PIN, COOLING_PWM_CHAN);
  setupPWM(LIGHTING_PWM_PIN, LIGHTING_PWM_CHAN);
}

void loop() {
  float temp = dht.readTemperature();
  int ldrRaw = analogRead(LDR_PIN);
  
  float lightPercent = (1.0 - (ldrRaw / 4095.0)) * 100.0;
  bool occupied = (digitalRead(PIR_PIN) == HIGH);

  int coolingPower = 0;
  int lightingPower = 0;

  if (!occupied) {
    coolingPower = 0;
    lightingPower = 0;
  } else {
    if (temp >= tempThreshold) {
      coolingPower = 100;
    } else {
      coolingPower = 20;
    }

    if (lightPercent < 50.0) {
      lightingPower = 100;
    } else {
      lightingPower = 10;
    }
  }

  setActuatorPower(COOLING_PWM_CHAN, coolingPower);
  setActuatorPower(LIGHTING_PWM_CHAN, lightingPower);

  Serial.print("Occupancy: ");
  if (occupied) {
    Serial.println("YES");
  } else {
    Serial.println("NO");
  }

  Serial.print("Temperature (C): ");
  Serial.println(temp);

  Serial.print("Light Level (%): ");
  Serial.println(lightPercent);

  Serial.print("Cooling PWM (%): ");
  Serial.println(coolingPower);

  Serial.print("Lighting PWM (%): ");
  Serial.println(lightingPower);

  Serial.println("---------------------------------------------");
  delay(1000);
}