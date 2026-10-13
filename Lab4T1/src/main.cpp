#include <Arduino.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

void setup()
{
  Serial.begin(115200);
  dht.begin();

  Serial.println("\n--- ESP32 Woke Up: Active Sensing Mode ---");

  // Read temperature and humidity
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  // Validate readings
  if (isnan(temp) || isnan(hum))
  {
    Serial.println("Error: Failed to read from DHT11 sensor!");
  }
  else
  {
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.print(" °C | Humidity: ");
    Serial.print(hum);
    Serial.println(" %");
  }

  // Active duration requirement (10 seconds total)
  Serial.println("Active window running (10 seconds total)...");
  delay(10000);

  // Enter Deep Sleep for 20 seconds
  Serial.println("Entering Deep Sleep for 20 seconds...");
  esp_sleep_enable_timer_wakeup(20 * 1000000ULL);
  esp_deep_sleep_start();
}

void loop()
{
  // Unused: Execution reboots into setup() upon RTC timer expiration
}