#include <WiFi.h>
#include <HTTPClient.h>
#include "ThingSpeak.h"

// --- Hardware Wi-Fi Credentials ---
const char *ssid = "Wokwi-GUEST";
const char *password = "";

// --- ThingSpeak Configuration ---
unsigned long channelID = 3505876;
const char *writeApiKey = "";
const char *readApiKey = "";

// --- Pin Definitions ---
#define POT_PIN 34 // Potentiometer Analog Input Pin (GPIO 34)
#define LED_PIN 18 // PWM Output Pin for LED (GPIO 18)

// --- Global Objects ---
WiFiClient client;

// --- PWM Parameters ---
const int pwmFreq = 5000;
const int pwmResolution = 8;

void setup()
{
  Serial.begin(115200);
  delay(100);

  pinMode(POT_PIN, INPUT);

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 1, 0)
  ledcAttach(LED_PIN, pwmFreq, pwmResolution);
#else
  ledcSetup(0, pwmFreq, pwmResolution);
  ledcAttachPin(LED_PIN, 0);
#endif

  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  ThingSpeak.begin(client);
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Wi-Fi disconnected! Reconnecting...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
    }
  }

  int rawADC = analogRead(POT_PIN);
  float voltage = (rawADC / 4095.0) * 3.3;

  Serial.println("----------------------------------------");
  Serial.printf("Measured Potentiometer ADC: %d | Voltage: %.2f V\n", rawADC, voltage);

  WiFiClient writeClient; // Fresh TCP socket for writing
  ThingSpeak.begin(writeClient);
  ThingSpeak.setField(2, voltage);

  int writeStatus = ThingSpeak.writeFields(channelID, writeApiKey);
  if (writeStatus == 200)
  {
    Serial.println(">> Cloud Write Success: Voltage sent to Field 2!");
  }
  else
  {
    Serial.printf(">> Cloud Write Failed, HTTP Error Code: %d\n", writeStatus);
  }

  writeClient.stop(); // Close TCP socket cleanly

  delay(15000);

  WiFiClient readClient; // Fresh TCP socket for reading
  HTTPClient http;

  String url = "http://api.thingspeak.com/channels/" + String(channelID) +
               "/fields/2/last.txt?api_key=" + String(readApiKey);

  http.setTimeout(10000);
  http.begin(readClient, url);

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK)
  {
    String payload = http.getString();
    float fetchedVoltage = payload.toFloat();

    Serial.printf("<< Cloud Read Success: Retrieved Field 2 Voltage = %.2f V\n", fetchedVoltage);

    int pwmDuty = map(constrain(fetchedVoltage * 100, 0, 330), 0, 330, 0, 255);

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 1, 0)
    ledcWrite(LED_PIN, pwmDuty);
#else
    ledcWrite(0, pwmDuty);
#endif

    Serial.printf("<< Applied PWM Duty Cycle to LED: %d / 255\n", pwmDuty);
  }
  else
  {
    Serial.printf("<< Cloud Read Failed, HTTP Error Code: %d\n", httpCode);
  }

  http.end();
  readClient.stop(); // Close TCP socket cleanly

  Serial.println("----------------------------------------\n");
  delay(5000);
}
