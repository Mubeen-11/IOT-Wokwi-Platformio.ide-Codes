#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

#define DHTPIN 15 // GPIO15
#define DHTTYPE DHT22
#define IRPIN 4 // GPIO4

const char *ssid = "laptop";       // Use "Wokwi-GUEST" for Wokwi
const char *password = "12345678"; // Use "" for Wokwi
const char *mqtt_server = "test.mosquitto.org";

const char *device_id = "ESP32_WROOM_01";
// Topic structured for single (+) or multi (#) level wildcards: lab6/devices/ESP32_WROOM_01/sensors
const char *mqtt_topic = "lab6/devices/ESP32_WROOM_01/sensors";

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

unsigned long lastMsg = 0;
unsigned long reading_count = 0;

void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to Wi-Fi network: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi Connected successfully!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Connecting to MQTT Broker...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str()))
    {
      Serial.println(" Connected!");
    }
    else
    {
      Serial.print(" Connection Failed! Return Code (rc)=");
      Serial.print(client.state());
      Serial.println(". Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(IRPIN, INPUT);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 4000)
  { // Publish every 4 seconds
    lastMsg = now;
    reading_count++;

    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    int ir_raw = digitalRead(IRPIN);
    bool obstacle_detected = (ir_raw == LOW); // LOW indicates object presence

    if (isnan(temp) || isnan(hum))
    {
      Serial.println("Error reading temperature/humidity from DHT11!");
      return;
    }

    // Allocate JSON Document
    StaticJsonDocument<256> doc;
    doc["device_id"] = device_id;
    doc["reading_no"] = reading_count;
    doc["temperature"] = temp;
    doc["humidity"] = hum;
    doc["obstacle"] = obstacle_detected;

    char jsonBuffer[256];
    serializeJson(doc, jsonBuffer);

    Serial.print("Publishing JSON payload to ");
    Serial.print(mqtt_topic);
    Serial.print(": ");
    Serial.println(jsonBuffer);

    client.publish(mqtt_topic, jsonBuffer);
  }
}