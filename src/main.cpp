#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>


#define IS_BOX true

const char* ssid = "";
const char* password = "";

String baseUrl = "https://8000-kode-ws-5549e3aa7.hebbale.academy";

#define PIN_DHT 25
#define DHT_TYPE DHT11
DHT dht(PIN_DHT , DHT_TYPE);
#define PIN_MQ135 34
#define PIN_BTN 19
#define PIN_BUZZER 15
#define PIN_LED_GREEN 26
#define PIN_LED_YELLOW 33
#define PIN_LED_RED 27

float lastTemp = 4.0, lastHum = 78.0;

void readDHT11() {
  float humidity = dht.readHumidity();
  float temperature =
  dht.readTemperature();
  if(isnan(humidity) || isnan(temperature)) {
    Serial.println("[DHT11] Failed to read sensor!");
    return;
  }
  lastTemp = temperature;
  lastHum = humidity;

  Serial.printf(
    "[DHT11] Temperature: %.1f C | Humidity : %.1f %%\n",
    lastTemp,lastHum
  );
}

void setLEDsAndBuzzer(String res) {
  JsonDocument doc;

  DeserializationError error = deserializeJson(doc , res);

  if(error){
    Serial.print("[JSON] Parse failed: ");
    Serial.println(error.c_str());
    return;
  }

  JsonObject data = doc["latest"].is<JsonObject>() ? doc["latest"].as<JsonObject>() : doc.as<JsonObject>();
  const char* color = data["ring_color"] | "UNKNOWN" ;
  int beeps = data["beeps"] | 0;

  Serial.print("[LED] Ring color: %s | Beeps: %d\n", color ,beeps);
  digitalWrite(PIN_LED_GREEN,
               strcmp(color , "GREEN") == 0 ? HIGH : LOW );

  digitalWrite(PIN_LED_YELLOW,
               strcmp(color , "YELLOW") == 0 ? HIGH : LOW );

  digitalWrite(PIN_LED_RED,
               strcmp(color , "RED") == 0 ? HIGH : LOW );

  for (int i = 0; i < beeps; i++) {
    digitalWrite(PIN_BUZZER, HIGH);
    delay(200);
    digitalWrite(PIN_BUZZER, LOW);
    delay(200);
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  digitalWrite(PIN_LED_GREEN , HIGH);
  digitalWrite(PIN_LED_YELLOW , HIGH);
  digitalWrite(PIN_LED_RED , HIGH);
  Serial.println(IS_BOX ? "BOX ESP32 STARTED" : "RING ESP32 STARTED");

  WiFi.begin(ssid, password);
}


void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    if (IS_BOX) {
      readDHT11();

      

      int rawGas = analogRead(PIN_MQ135);
      float gasPpm = map(constrain(rawGas, 0, 4095), 0, 4095, 20, 280);
      bool btnPressed = (digitalRead(PIN_BTN) == LOW);
      
      Serial.printf("Raw MQ135 ADC : %d | Gas: %.0f PPM\n" , rawGas , gasPpm);

      Serial.printf(
          "[BOX] Temp:%.1fC Hum:%.0f%% Gas:%.0fPPM Btn:%d\n",
          lastTemp, lastHum, gasPpm, btnPressed
      );

      http.begin(baseUrl + "/telemetry");
      http.addHeader("Content-Type", "application/json");

      String body = "{\"crate_temp\":" + String(lastTemp) +
                    ",\"humidity\":" + String(lastHum) +
                    ",\"gas_ppm\":" + String(gasPpm) +
                    ",\"switch_user\":" + (btnPressed ? "true" : "false") + "}";

      if (http.POST(body) == 200) {
        setLEDsAndBuzzer(http.getString());
      }
    } else {
      http.begin(baseUrl + "/state");

      if (http.GET() == 200) {
        String res = http.getString();
        Serial.println("[RING] Synced with Box!");
        setLEDsAndBuzzer(res);
      }
    }

    http.end();
  }

  delay(2000);
}