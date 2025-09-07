#include <time.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1331.h>
#include "Adafruit_SHT4x.h"
#include <WiFi.h>

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// 14x14 Wi-Fi icon (1 = pixel on, 0 = off)
const uint8_t wifiIcon[] = {
  0b00000000, 0b00000000, // row 0
  0b00000111, 0b00000000, // row 1 (largest arc)
  0b00001000, 0b10000000, // row 2
  0b00010000, 0b01000000, // row 3
  0b00100000, 0b00100000, // row 4 (medium arc)
  0b01000000, 0b00010000, // row 5
  0b00000000, 0b00000000, // row 6 (gap)
  0b00000100, 0b01000000, // row 7 (smallest arc)
  0b00000011, 0b10000000, // row 8
  0b00000000, 0b00000000, // row 9
  0b00000000, 0b00000000, // row 10
  0b00000000, 0b00000000, // row 11
  0b00000001, 0b00000000, // row 12 (bottom dot)
  0b00000000, 0b00000000  // row 13
};

#define cs  7
#define rst 3
#define dc  2
Adafruit_SSD1331 display = Adafruit_SSD1331(&SPI, cs, dc, rst);

Adafruit_SHT4x sht4 = Adafruit_SHT4x();

const char* ssid     = "SKYWXRFD";
const char* password = "Knh9wHi6FQQS";

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  WiFi.begin(ssid, password);

  display.begin();
  display.fillScreen(BLACK);
  displayText(0, 0, 1, "Garage", YELLOW);
  display.drawLine(0, 10, 96, 10, YELLOW);
  display.drawLine(0, 55, 96, 55, YELLOW);

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
  tm.tm_mday = 6;
  tm.tm_hour = 14;
  tm.tm_min  = 0;
  tm.tm_sec  = 0;
  time_t t = mktime(&tm);
  struct timeval now = { .tv_sec = t };
  settimeofday(&now, NULL);

  Serial.print("Connecting to wifi");
}

void loop() {
  sensors_event_t humidity, temp;
  if (!sht4.getEvent(&humidity, &temp)) {
    Serial.println("Failed to read sensor");
    delay(1000);
    return;
  }

  Serial.print("Temp: ");
  Serial.print(temp.temperature);
  Serial.print(" °C  ");
  Serial.print("Humidity: ");
  Serial.print(humidity.relative_humidity);
  Serial.println(" %");

  updateDisplay(temp.temperature, humidity.relative_humidity);
  delay(2000);
}

void updateDisplay(float temperature, float humidity) {
  String date, time;
  dateTimeString(date, time);

  String tempText = String(temperature);
  tempText.concat(char(247));
  tempText.concat("C");

  String humidText = String(humidity);
  humidText.concat("%RH");

  display.fillRect(0, 14, 200, 40, BLACK);
  displayText(0, 14, 1, date, CYAN);
  displayText(0, 24, 1, time, CYAN);
  displayText(0, 34, 1, tempText, CYAN);
  displayText(0, 44, 1, humidText, CYAN);

  display.fillRect(80, 0, 8, 8, BLACK);
  drawWiFiIcon(86, 0, WiFi.status() == WL_CONNECTED);
}

void displayText(int x, int y, int size, String text, int color) {
  display.setCursor(x, y);
  display.setTextColor(color);
  display.setTextSize(size);
  display.print(text);
}

void dateTimeString(String &dateStr, String &timeStr) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    dateStr = "00/00/0000";
    timeStr = "00:00:00";
    return;
  }

  char buf[16];

  // Date string: dd/mm/YYYY
  strftime(buf, sizeof(buf), "%d/%m/%Y", &timeinfo);
  dateStr = String(buf);

  // Time string: hh:mm:ss
  strftime(buf, sizeof(buf), "%H:%M:%S", &timeinfo);
  timeStr = String(buf);
}

void drawWiFiIcon(int x, int y, bool connected) {
  uint16_t color = connected ? GREEN : RED;

  // Circle
  display.fillCircle(x + 4, y + 4, 3, color);
}