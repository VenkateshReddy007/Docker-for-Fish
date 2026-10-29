#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
bool ledTestDone = false ; 

#define IS_BOX true

const char* ssid = "Yiascm Hackathon 1";
const char* password = "Hack@2026";

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
  int latestPos = res.indexOf("\"latest\"");

  if (latestPos >= 0) {
    int start = res.indexOf('{', latestPos);
    int end = res.indexOf('}', start);

    if (start >= 0 && end > start) {
      res = res.substring(start, end + 1);
    }
  }

  String color = "";
  int key = res.indexOf("\"ring_color\"");

  if (key >= 0) {
    int colon = res.indexOf(':', key);
    int firstQuote = res.indexOf('"', colon + 1);
    int secondQuote = res.indexOf('"', firstQuote + 1);

    if (firstQuote >= 0 && secondQuote > firstQuote) {
      color = res.substring(firstQuote + 1, secondQuote);
    }
  }

  int beeps = 0;
  int beepKey = res.indexOf("\"beeps\"");

  if (beepKey >= 0) {
    int colon = res.indexOf(':', beepKey);

    if (colon >= 0) {
      beeps = res.substring(colon + 1).toInt();
    }
  }

  Serial.printf("[LED] Color: %s | Beeps: %d\n",
                color.c_str(), beeps);

  digitalWrite(PIN_LED_GREEN, color == "GREEN" ? HIGH : LOW);
  digitalWrite(PIN_LED_YELLOW, color == "YELLOW" ? HIGH : LOW);
  digitalWrite(PIN_LED_RED, color == "RED" ? HIGH : LOW);

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

void loopLED(){
  digitalWrite(PIN_LED_GREEN ,HIGH);
  digitalWrite(PIN_LED_YELLOW , LOW);
  digitalWrite(PIN_LED_RED ,LOW );
  delay(2000);

  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_YELLOW, HIGH);
  digitalWrite(PIN_LED_RED , LOW);
  delay(2000);

  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_RED , HIGH);
  delay(2000);
}


void loop() {

  if(!ledTestDone) {
    ledTestDone = true;
    loopLED();
    return;
  }
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