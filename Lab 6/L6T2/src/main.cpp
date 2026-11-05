#include <WiFi.h>
#include <PubSubClient.h>

#define STATUS_LED 2 // Onboard LED on ESP32 Dev Module

const char *ssid = "laptop";       // Replace with your local Wi-Fi SSID
const char *password = "12345678"; // Replace with your local Wi-Fi Password
const char *mqtt_server = "test.mosquitto.org";

const char *status_topic = "lab6/esp32_recovery/status";
const char *telemetry_topic = "lab6/esp32_recovery/telemetry";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastPublish = 0;
unsigned long lastReconnectAttempt = 0;
unsigned long packet_count = 0;

// Non-blocking LED indicator state control
void update_led_indicator(bool connected)
{
  digitalWrite(STATUS_LED, connected ? HIGH : LOW);
}

// Wi-Fi Connection & Recovery Handler
void check_wifi_connection()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    update_led_indicator(false);
    Serial.println("\n[WARN] Wi-Fi connection lost! Initiating recovery...");

    WiFi.disconnect();
    WiFi.begin(ssid, password);

    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000)
    {
      delay(500);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("\n[Wi-Fi] RESTORED! IP Address: " + WiFi.localIP().toString());
    }
    else
    {
      Serial.println("\n[Wi-Fi] Reconnection attempt timed out. Will retry in main loop.");
    }
  }
}

// Non-blocking MQTT Broker Recovery Handler
bool reconnect_mqtt()
{
  Serial.println("[MQTT] Attempting broker connection to test.mosquitto.org...");
  String clientId = "ESP32Rec-" + String(random(0xffff), HEX);

  if (client.connect(clientId.c_str()))
  {
    Serial.println("[MQTT] SUCCESS - Connected to broker!");

    // Publish reconnection event to alert subscribers
    client.publish(status_topic, "SYSTEM_RECOVERED_AND_ACTIVE");
    update_led_indicator(true);
    return true;
  }

  Serial.print("[MQTT] FAILED - Return Code (rc) = ");
  Serial.print(client.state());
  Serial.println(". Will retry in 5 seconds...");
  update_led_indicator(false);
  return false;
}

void setup()
{
  Serial.begin(115200);
  pinMode(STATUS_LED, OUTPUT);
  update_led_indicator(false);

  Serial.println("\n--- Task 3 Auto-Recovery System Starting ---");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  check_wifi_connection();
  client.setServer(mqtt_server, 1883);
}

void loop()
{
  // Step 1: Monitor and recover Wi-Fi layer
  if (WiFi.status() != WL_CONNECTED)
  {
    check_wifi_connection();
  }

  // Step 2: Monitor and recover MQTT layer if Wi-Fi is active
  if (WiFi.status() == WL_CONNECTED)
  {
    if (!client.connected())
    {
      unsigned long now = millis();
      if (now - lastReconnectAttempt > 5000)
      { // Non-blocking 5-second interval retry
        lastReconnectAttempt = now;
        if (reconnect_mqtt())
        {
          lastReconnectAttempt = 0;
        }
      }
    }
    else
    {
      // Step 3: Service active MQTT client loop & publish telemetry
      client.loop();

      unsigned long now = millis();
      if (now - lastPublish > 3000)
      { // Publish telemetry packet every 3 seconds
        lastPublish = now;
        packet_count++;

        String payload = "{\"status\":\"RUNNING\",\"packet_id\":" + String(packet_count) + "}";
        Serial.print("[TELEMETRY PUBLISH] Transmitting packet #");
        Serial.print(packet_count);
        Serial.print(": ");
        Serial.println(payload);

        client.publish(telemetry_topic, payload.c_str());
      }
    }
  }
}