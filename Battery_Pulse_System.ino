#define BLYNK_TEMPLATE_ID "TMPL3JNW14-Ui"
#define BLYNK_TEMPLATE_NAME "Battery Pulse System"
#define BLYNK_FIRMWARE_VERSION "0.1.0"
#define BLYNK_PRINT Serial
#include "BlynkEdgent.h"
#include <DHT.h>
#define DHTTYPE DHT22
#define DHTPIN D4
DHT dht(DHTPIN, DHTTYPE);

// Battery Monitoring
const int analogInPin = A0;
int sensorValue;
float voltage, bat_percentage;
const float calibration = 0.40;
const int CHARGE_EN_PIN = D5;

// Li-ion Battery Parameters
const float MIN_VOLTAGE = 3.0;
const float MAX_VOLTAGE = 4.2;
const float OVERHEAT_TEMP = 45;

void setup() {
  Serial.begin(115200);
  delay(3000);
  
  pinMode(CHARGE_EN_PIN, OUTPUT);
  digitalWrite(CHARGE_EN_PIN, HIGH);
  
  BlynkEdgent.begin();
  dht.begin();
}

void loop() {
  BlynkEdgent.run();
  
  // Read and send temperature/humidity
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  
  if (!isnan(t) && !isnan(h)) {
    Serial.print("\nTemperature: ");
    Serial.print(t);
    Serial.println("°C");
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.println("%");
    Blynk.virtualWrite(V1, t);
    Blynk.virtualWrite(V2, h);
  } else {
    Serial.println("Failed to read DHT sensor");
  }

  // Read battery voltage
  sensorValue = analogRead(analogInPin);
  voltage = (((sensorValue * 3.3) / 1024) * 2 + calibration);
  
  bat_percentage = mapfloat(voltage, MIN_VOLTAGE, MAX_VOLTAGE, 0, 100);
  bat_percentage = constrain(bat_percentage, 0, 100);

  Serial.print("Battery Voltage: ");
  Serial.print(voltage);
  Serial.println("V");
  Serial.print("Battery Percentage: ");
  Serial.print(bat_percentage);
  Serial.println("%");
  Blynk.virtualWrite(V3, voltage);
  Blynk.virtualWrite(V4, bat_percentage);

  if (bat_percentage <= 30) {
    Blynk.logEvent("battery_low", "Battery below 30% - Please charge");
    Serial.println("Warning: Battery low, Please Charge");
  }

  if (t > OVERHEAT_TEMP) {
    digitalWrite(CHARGE_EN_PIN, LOW);
    Blynk.logEvent("battery_overheat", String("Critical temperature: ") + t + "°C");
    Serial.println("EMERGENCY: Charging disabled due to overheating");
  } else {
    digitalWrite(CHARGE_EN_PIN, HIGH);
    Serial.println("Battery Temp Status: Normal");
  }

  if (voltage >= 4.18) {
    Blynk.virtualWrite(V5, "Fully Charged");
    Serial.println("Status: Battery fully charged, Please Disconnect Charger");
  }

  delay(3000);
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}