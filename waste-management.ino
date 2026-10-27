#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <LittleFS.h>
#include "HX711.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Hotspot Credentials
const char* ssid = "food-waste";
const char* password = "12345678";

WebServer server(80);

// Hardware Pins
const int LOADCELL_DOUT_PIN = 19;
const int LOADCELL_SCK_PIN = 18;

const int LED_GREEN  = 25;
const int LED_YELLOW = 26;
const int LED_RED    = 27;

const int BTN_SAVE_DAY = 32;
const int BTN_NEXT_DAY = 33;

HX711 scale;
float calibration_factor = 370.0;
const float ZERO_THRESHOLD = 5.0;
const float DETECTION_TRIGGER = 10.0;

// Application Variables
float totalFoodPrepared = 1000.0;
float dailyWaste[7] = {0, 0, 0, 0, 0, 0, 0};
int currentDay = 0;

enum SystemState { WAITING_FOR_WASTE, COUNTDOWN, SHOW_FINAL_WEIGHT };
SystemState currentState = WAITING_FOR_WASTE;
unsigned long countdownStartTime = 0;
float finalWeight = 0.0;

void updateLEDs(float percentage) {
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  if (percentage <= 30.0) {
    digitalWrite(LED_GREEN, HIGH);
  } else if (percentage > 40.0 && percentage <= 70.0) {
    digitalWrite(LED_YELLOW, HIGH);
  } else if (percentage > 70.0) {
    digitalWrite(LED_RED, HIGH);
  } else {
    digitalWrite(LED_GREEN, HIGH);
  }
}

void saveCurrentWeight() {
  dailyWaste[currentDay] += finalWeight;
  finalWeight = 0.0;
  currentState = WAITING_FOR_WASTE;
  scale.tare(10);
}

void moveToNextDay() {
  currentDay = (currentDay + 1) % 7;
  finalWeight = 0.0;
  currentState = WAITING_FOR_WASTE;
  scale.tare(10);
}

// Serve HTML File from LittleFS
void handleRoot() {
  File file = LittleFS.open("/index.html", "r");
  if (!file) {
    server.send(404, "text/plain", "index.html not found. Upload LittleFS data!");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}

// JSON Data Endpoint for Frontend AJAX Calls
void handleData() {
  float totalWeekWaste = 0.0;
  for (int i = 0; i < 7; i++) {
    totalWeekWaste += dailyWaste[i];
  }
  float currentPercent = (totalFoodPrepared > 0) ? ((dailyWaste[currentDay] + finalWeight) / totalFoodPrepared) * 100.0 : 0.0;
  float weekPercent = (totalFoodPrepared > 0) ? (totalWeekWaste / (totalFoodPrepared * 7.0)) * 100.0 : 0.0;

  String statusText = "";
  String statusClass = "";
  if (currentPercent <= 30.0) {
    statusText = "STATUS: SAFE (<= 30%)";
    statusClass = "green";
  } else if (currentPercent > 40.0 && currentPercent <= 70.0) {
    statusText = "WARNING: MODERATE (> 40%)";
    statusClass = "yellow";
  } else {
    statusText = "ALERT: HIGH FOOD WASTE (> 70%)";
    statusClass = "red";
  }

  String json = "{";
  json += "\"currentDay\":" + String(currentDay + 1) + ",";
  json += "\"totalFoodPrepared\":" + String(totalFoodPrepared, 1) + ",";
  json += "\"finalWeight\":" + String(finalWeight, 1) + ",";
  json += "\"todayWaste\":" + String(dailyWaste[currentDay], 1) + ",";
  json += "\"currentPercent\":" + String(currentPercent, 1) + ",";
  json += "\"statusText\":\"" + statusText + "\",";
  json += "\"statusClass\":\"" + statusClass + "\",";
  json += "\"totalWeekWaste\":" + String(totalWeekWaste, 1) + ",";
  json += "\"weekPercent\":" + String(weekPercent, 1) + ",";
  json += "\"dailyWaste\":[";
  for (int i = 0; i < 7; i++) {
    json += String(dailyWaste[i], 1);
    if (i < 6) json += ",";
  }
  json += "]";
  json += "}";

  server.send(200, "application/json", json);
}

void handleSetFood() {
  if (server.hasArg("totalFood")) {
    totalFoodPrepared = server.arg("totalFood").toFloat();
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleWebSave() {
  saveCurrentWeight();
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleWebNext() {
  moveToNextDay();
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  pinMode(BTN_SAVE_DAY, INPUT_PULLUP);
  pinMode(BTN_NEXT_DAY, INPUT_PULLUP);

  if (!LittleFS.begin(true)) {
    Serial.println("An Error has occurred while mounting LittleFS");
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED Failed");
  }

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor);
  delay(500);
  scale.tare(15);

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(5, 15);
  display.println("Connecting Hotspot...");
  display.display();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 16) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("\nConnected! IP: ");
    Serial.println(WiFi.localIP());
    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.on("/setFood", HTTP_POST, handleSetFood);
    server.on("/webSave", handleWebSave);
    server.on("/webNext", handleWebNext);
    server.begin();
  } else {
    Serial.println("\nHotspot Not Connected (Offline Scale Active)");
  }

  updateLEDs(0.0);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    server.handleClient();
  }

  if (!scale.is_ready()) return;

  float currentWeight = scale.get_units(2);
  if (abs(currentWeight) < ZERO_THRESHOLD) {
    currentWeight = 0.0;
  }

  if (digitalRead(BTN_SAVE_DAY) == LOW) {
    delay(200);
    saveCurrentWeight();
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(24, 28);
    display.println("WEIGHT SAVED!");
    display.display();
    delay(1000);
  }

  if (digitalRead(BTN_NEXT_DAY) == LOW) {
    delay(200);
    moveToNextDay();
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(20, 28);
    display.println("SHIFTED TO:");
    display.setTextSize(2);
    display.setCursor(35, 42);
    display.print("DAY ");
    display.print(currentDay + 1);
    display.display();
    delay(1000);
  }

  float activePercentage = (totalFoodPrepared > 0) ? ((dailyWaste[currentDay] + finalWeight) / totalFoodPrepared) * 100.0 : 0.0;
  updateLEDs(activePercentage);

  switch (currentState) {
    case WAITING_FOR_WASTE: {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("DAY "); display.print(currentDay + 1);
      display.setCursor(64, 0);
      display.print("Tot:"); display.print((int)totalFoodPrepared);

      display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
      display.setCursor(0, 16);
      display.println("Place Waste Plate");

      display.setTextSize(2);
      display.setCursor(16, 32);
      display.print("0.0 g");

      display.setTextSize(1);
      display.setCursor(0, 54);
      if (WiFi.status() == WL_CONNECTED) {
        display.print("IP: "); display.print(WiFi.localIP());
      } else {
        display.print("Ready (Offline Mode)");
      }
      display.display();

      if (currentWeight >= DETECTION_TRIGGER) {
        countdownStartTime = millis();
        currentState = COUNTDOWN;
      }
      break;
    }

    case COUNTDOWN: {
      unsigned long elapsedSec = (millis() - countdownStartTime) / 1000;
      int remainingSec = 10 - elapsedSec;

      if (remainingSec > 0) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(10, 2);
        display.println("MEASURING WASTE...");
        display.drawLine(0, 12, 128, 12, SSD1306_WHITE);

        display.setCursor(20, 20);
        display.println("Settling in:");

        display.setTextSize(3);
        display.setCursor(54, 34);
        display.print(remainingSec);
        display.display();
      } else {
        finalWeight = scale.get_units(5);
        if (finalWeight < ZERO_THRESHOLD) finalWeight = 0.0;
        currentState = SHOW_FINAL_WEIGHT;
      }
      break;
    }

    case SHOW_FINAL_WEIGHT: {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("DAY "); display.print(currentDay + 1);
      display.setCursor(64, 0);
      display.print("W: "); display.print(activePercentage, 0); display.print("%");
      display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

      display.setTextSize(2);
      display.setCursor(10, 18);
      display.print(finalWeight, 1);
      display.setTextSize(1);
      display.print(" g");

      display.setCursor(0, 40);
      if (activePercentage <= 30.0) {
        display.print("Status: SAFE");
      } else if (activePercentage > 40.0 && activePercentage <= 70.0) {
        display.print("WARN: MODERATE");
      } else if (activePercentage > 70.0) {
        display.print("ALERT: HIGH WASTE");
      }

      display.setCursor(0, 54);
      display.print("Btn1:Save | Remove:0");
      display.display();

      if (currentWeight < ZERO_THRESHOLD) {
        delay(400);
        scale.tare(5);
        currentState = WAITING_FOR_WASTE;
      }
      break;
    }
  }

  delay(20);
}