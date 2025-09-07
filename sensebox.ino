#include <time.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1331.h>
#include "Adafruit_SHT4x.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "NodeData.h"
#include "JsonHelpers.h"
#include <vector>

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define cs  7
#define rst 3
#define dc  2
Adafruit_SSD1331 display = Adafruit_SSD1331(&SPI, cs, dc, rst);

Adafruit_SHT4x sht4 = Adafruit_SHT4x();

const char* ssid     = "SKYWXRFD";
const char* password = "Knh9wHi6FQQS";
const char* serverUrl = "http://192.168.0.67:3000/api/data";

Sensor temperatureSensor = { "Temperature Sensor", "TEMPERATURE", "SHT-41D" };
Sensor humiditySensor    = { "Humidity Sensor",    "HUMIDITY",    "SHT-41D" };
Node node = {
  "Node 1",
  "Garage",
  { &temperatureSensor, &humiditySensor }
};

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  WiFi.begin(ssid, password);

  display.begin();
  display.fillScreen(BLACK);
  displayText(0, 0, 1, "Garage", YELLOW);
  display.drawLine(0, 10, 96, 10, YELLOW);

  if (!sht4.begin()) {
    Serial.println("Couldn't find SHT4x");
    while (1) delay(1);
  }

  sht4.setPrecision(SHT4X_HIGH_PRECISION);
  sht4.setHeater(SHT4X_NO_HEATER);

  // Set a fake time: 6 Sept 2025, 14:00:00
  struct tm tm;
  tm.tm_year = 2025 - 1900;  // years since 1900
  tm.tm_mon  = 9 - 1;        // months since January
  tm.tm_mday = 7;
  tm.tm_hour = 18;
  tm.tm_min  = 24;
  tm.tm_sec  = 0;
  time_t t = mktime(&tm);
  struct timeval now = { .tv_sec = t };
  settimeofday(&now, NULL);

  Serial.print("Connecting to wifi");
}

void loop() {

  static bool readThisSecond = false;
  static bool storedThisInterval = false;
  static bool sentThisInterval = false;
  static int lastSec = -1;

  time_t now = getCurrentTime();
  struct tm t;
  gmtime_r(&now, &t);  // broken-down UTC time

  // --- Reset flags at new second ---
  if (t.tm_sec != lastSec) {
    readThisSecond = false;
    storedThisInterval = false;
    sentThisInterval = false;
    lastSec = t.tm_sec;
  }

  // --- Display every second ---
  if (!readThisSecond) {
    sensors_event_t humidity, temp;
    if (!sht4.getEvent(&humidity, &temp)) {
      Serial.println("Failed to read sensor");
        delay(500);
        return;
    }
    readThisSecond = true;

    // --- Store every 15 seconds ---
    if ((t.tm_sec % 15 == 0) && !storedThisInterval) {
      addReading(temperatureSensor, now, temp.temperature, "CELSIUS");
      addReading(humiditySensor, now, humidity.relative_humidity, "PERCENT");
      storedThisInterval = true;
    }

    // Send every 2 minutes ---
    if ((t.tm_min % 2 == 0) && (t.tm_sec == 0) && !sentThisInterval) {
      // sendData();
      sentThisInterval = true;
    }

    updateDisplay(now, temp.temperature, humidity.relative_humidity);
  }
  
  delay(50);
}

void updateDisplay(time_t now, float temperature, float humidity) {
  String dateStr = formatDate(now);
  String timeStr = formatTime(now);

  String tempStr = String(temperature);
  tempStr.concat(char(247));
  tempStr.concat("C");

  String humidStr = String(humidity);
  humidStr.concat("%RH");

  displayText(0, 14, 1, dateStr, CYAN);
  displayText(0, 24, 1, timeStr, CYAN);
  displayText(0, 34, 1, tempStr, CYAN);
  displayText(0, 44, 1, humidStr, CYAN);

  drawWiFiIcon(86, 0, WiFi.status() == WL_CONNECTED);

  String bufferStr = String(getBufferPercentFull(temperatureSensor));
  bufferStr.concat("% Full");
  displayText(0, 54, 1, bufferStr, YELLOW);
}

void displayText(int x, int y, int size, String text, int color) {
  display.setCursor(x, y);
  display.setTextColor(color, BLACK);
  display.setTextSize(size);
  display.print(text);
}

void drawWiFiIcon(int x, int y, bool connected) {
  uint16_t color = connected ? GREEN : RED;

  // Circle
  display.fillCircle(x + 4, y + 4, 3, color);
}

void sendData(float temperature, float humidity) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");

  String json = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + "}";
  int code = http.POST(json);
  Serial.print("HTTP Response code: ");
  Serial.println(code);

  http.end();
}

time_t getCurrentTime() {
    return time(nullptr);  // seconds since Jan 1, 1970 UTC
}

String formatDate(time_t t) {
    struct tm timeinfo;
    gmtime_r(&t, &timeinfo);

    char buf[16];
    strftime(buf, sizeof(buf), "%d/%m/%Y", &timeinfo);
    return String(buf);
}

String formatTime(time_t t) {
    struct tm timeinfo;
    gmtime_r(&t, &timeinfo);

    char buf[16];
    strftime(buf, sizeof(buf), "%H:%M:%S", &timeinfo);
    return String(buf);
}