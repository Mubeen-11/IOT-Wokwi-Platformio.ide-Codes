#include <WiFi.h>
#include <PubSubClient.h>

#define LED_PIN 2 // Onboard LED on ESP32 Dev Module (Active-HIGH)

const char *ssid = "laptop";       // Use "Wokwi-GUEST" for Wokwi simulation
const char *password = "12345678"; // Use "" for Wokwi simulation
const char *mqtt_server = "test.mosquitto.org";

const char *cmd_topic = "lab6/esp32/control";
const char *state_topic = "lab6/esp32/state";

WiFiClient espClient;
PubSubClient client(espClient);

bool led_state = false; // System state memory: false = OFF, true = ON

// Helper function to broadcast current LED state to MQTT
void publish_led_state()
{
  const char *payload = led_state ? "STATE: ON" : "STATE: OFF";
  client.publish(state_topic, payload);
  Serial.print("[MQTT PUBLISH] Sent to state topic: ");
  Serial.println(payload);
}

// MQTT Callback Function - Executes automatically on incoming payload
void callback(char *topic, byte *payload, unsigned int length)
{
  char message[32];
  if (length >= sizeof(message))
    length = sizeof(message) - 1;

  for (unsigned int i = 0; i < length; i++)
  {
    message[i] = (char)payload[i];
  }
  message[length] = '\0'; // Null-terminate string

  Serial.print("\n[MQTT RECEIVED] Topic [");
  Serial.print(topic);
  Serial.print("] Payload: ");
  Serial.println(message);

  // Command Evaluation Logic
  if (strcmp(message, "ON") == 0)
  {
    led_state = true;
    digitalWrite(LED_PIN, HIGH); // Turn LED ON
    Serial.println(" -> Action Executed: LED turned ON");
    publish_led_state();
  }
  else if (strcmp(message, "OFF") == 0)
  {
    led_state = false;
    digitalWrite(LED_PIN, LOW); // Turn LED OFF
    Serial.println(" -> Action Executed: LED turned OFF");
    publish_led_state();
  }
  else if (strcmp(message, "STATUS") == 0)
  {
    Serial.println(" -> Action Executed: STATUS requested");
    publish_led_state();
  }
  else
  {
    // Command Error Guard: Rejects invalid payloads without altering hardware state
    Serial.println(" -> ERROR: Invalid command received! State unchanged.");
  }
}

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

  Serial.println("\nWi-Fi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Connecting to MQTT Broker (test.mosquitto.org)...");
    String clientId = "ESP32Control-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str()))
    {
      Serial.println(" Connected!");
      // Subscribe to control topic upon establishing connection
      client.subscribe(cmd_topic);
      Serial.print("Subscribed to command topic: ");
      Serial.println(cmd_topic);
    }
    else
    {
      Serial.print(" Failed, rc=");
      Serial.print(client.state());
      Serial.println(". Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Initialize with LED OFF

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback); // Attach sub event callback
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop(); // Keeps MQTT client state machine responsive
}

// mosquitto_pub -h test.mosquitto.org -t "lab6/esp32/control" -m "ON"
// mosquitto_pub -h test.mosquitto.org -t "lab6/esp32/control" -m "STATUS"
// mosquitto_pub -h test.mosquitto.org -t "lab6/esp32/control" -m "TOGGLE"
// mosquitto_pub -h test.mosquitto.org -t "lab6/esp32/control" -m "OFF"