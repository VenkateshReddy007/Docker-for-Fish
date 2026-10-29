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
#define PIN_LED_GREEN 35
#define PIN_LED_YELLOW 33
#define PIN_LED_RED 27

float lastTemp = 4.0, lastHum = 78.0;

void readDHT11() {
  uint8_t data[5] = {0, 0, 0, 0, 0};

  pinMode(PIN_DHT, OUTPUT);
  digitalWrite(PIN_DHT, LOW);
  delay(18);
  digitalWrite(PIN_DHT, HIGH);
  delayMicroseconds(40);
  pinMode(PIN_DHT, INPUT_PULLUP);

  if (pulseIn(PIN_DHT, HIGH, 1000) == 0) return;
  {
    Serial.println("[DHT11] No Response on GPI0 13");
    return;
  }

  for (int i = 0; i < 40; i++) {
    while (digitalRead(PIN_DHT) == LOW);

    unsigned long t = micros();

    while (digitalRead(PIN_DHT) == HIGH) {
      if (micros() - t > 100) break;
    }

    if (micros() - t > 40) {
      data[i / 8] |= (1 << (7 - (i % 8)));
    }
  }

  if (data[0] > 0 &&
      ((data[0] + data[1] + data[2] + data[3]) & 0xFF) == data[4]) {
    lastHum = data[0];
    lastTemp = data[2];
  }
}

void setLEDsAndBuzzer(String res) {
  digitalWrite(PIN_LED_GREEN,
               res.indexOf("\"ring_color\":\"GREEN\"") > 0 ? HIGH : LOW);

  digitalWrite(PIN_LED_YELLOW,
               res.indexOf("\"ring_color\":\"YELLOW\"") > 0 ? HIGH : LOW);

  digitalWrite(PIN_LED_RED,
               res.indexOf("\"ring_color\":\"RED\"") > 0 ? HIGH : LOW);

  int beeps = (res.indexOf("\"beeps\":2") > 0)
                  ? 2
                  : ((res.indexOf("\"beeps\":1") > 0) ? 1 : 0);

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

      Serial.printf("[DHT11] Temperature: %.1f C | Humidity: %.1f %%\n" , lastTemp , lastHum);

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