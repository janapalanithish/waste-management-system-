First divide the code into two different parts.
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "HX711.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Hotspot Credentials (Mee hotspot name & password ivvandi)
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
float calibration_factor = 370.0; // Mee calibrated factor
const float ZERO_THRESHOLD = 5.0;
const float DETECTION_TRIGGER = 10.0;

// Application Variables
float totalFoodPrepared = 1000.0; // Default 1000g
float dailyWaste[7] = {0, 0, 0, 0, 0, 0, 0};
int currentDay = 0; // 0 = Day 1, 6 = Day 7

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

// Webpage UI
String getHTML() {
  float totalWeekWaste = 0.0;
  for (int i = 0; i < 7; i++) {
    totalWeekWaste += dailyWaste[i];
  }
  float currentPercent = (totalFoodPrepared > 0) ? ((dailyWaste[currentDay] + finalWeight) / totalFoodPrepared) * 100.0 : 0.0;
  float weekPercent = (totalFoodPrepared > 0) ? (totalWeekWaste / (totalFoodPrepared * 7.0)) * 100.0 : 0.0;

  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>Food Waste Tracker</title>";
  html += "<style>";
  html += "body{font-family:Segoe UI,sans-serif;background:#0f172a;color:#f8fafc;padding:15px;text-align:center;margin:0;}";
  html += ".card{background:#1e293b;border-radius:12px;padding:16px;margin:12px auto;max-width:480px;}";
  html += "input[type=number]{width:80%;padding:10px;margin:8px 0;border-radius:6px;border:none;font-size:16px;}";
  html += "button{background:#38bdf8;color:#0f172a;font-weight:bold;padding:10px 18px;border:none;border-radius:6px;cursor:pointer;margin:5px;font-size:15px;}";
  html += ".btn-save{background:#22c55e;color:#000;}.btn-next{background:#f59e0b;color:#000;}";
  html += "table{width:100%;border-collapse:collapse;margin-top:12px;}th,td{border:1px solid #334155;padding:8px;}th{background:#334155;}";
  html += ".badge{padding:8px 14px;border-radius:6px;font-weight:bold;display:inline-block;margin-top:8px;}";
  html += ".green{background:#22c55e;color:#000;}.yellow{background:#eab308;color:#000;}.red{background:#ef4444;color:#fff;}";
  html += "</style></head><body>";

  html += "<h2>Smart Food Waste Tracker</h2>";

  html += "<div class='card'>";
  html += "<h3>Day Operations (Day " + String(currentDay + 1) + ")</h3>";
  html += "<a href='/webSave'><button class='btn-save'>Save Weight</button></a>";
  html += "<a href='/webNext'><button class='btn-next'>Next Day</button></a>";
  html += "</div>";

  html += "<div class='card'>";
  html += "<h3>Set Total Food Prepared</h3>";
  html += "<form action='/setFood' method='POST'>";
  html += "<input type='number' step='0.1' name='totalFood' placeholder='Total Food in Grams' value='" + String(totalFoodPrepared) + "' required><br>";
  html += "<button type='submit'>Update Total Food</button>";
  html += "</form>";
  html += "</div>";

  html += "<div class='card'>";
  html += "<h3>Day " + String(currentDay + 1) + " Live Status</h3>";
  html += "<p>Live Weight: <b>" + String(finalWeight, 1) + " g</b></p>";
  html += "<p>Saved Waste Today: <b>" + String(dailyWaste[currentDay], 1) + " g</b></p>";
  html += "<p>Waste Percentage: <b>" + String(currentPercent, 1) + "%</b></p>";

  if (currentPercent <= 30.0) {
    html += "<span class='badge green'>STATUS: SAFE (&le; 30%)</span>";
  } else if (currentPercent > 40.0 && currentPercent <= 70.0) {
    html += "<span class='badge yellow'>WARNING: MODERATE (> 40%)</span>";
  } else if (currentPercent > 70.0) {
    html += "<span class='badge red'>ALERT: HIGH FOOD WASTE (> 70%)</span>";
  }
  html += "</div>";

  html += "<div class='card'>";
  html += "<h3>Weekly Overview</h3>";
  html += "<table><tr><th>Day</th><th>Total Waste</th></tr>";
  for (int i = 0; i < 7; i++) {
    html += "<tr><td>Day " + String(i + 1) + (i == currentDay ? " (Active)" : "") + "</td>";
    html += "<td>" + String(dailyWaste[i], 1) + " g</td></tr>";
  }
  html += "</table>";
  html += "<hr style='border:0;border-top:1px solid #334155;margin:15px 0;'>";
  html += "<p>Total 7 Days Waste: <b>" + String(totalWeekWaste, 1) + " g</b></p>";
  html += "<p>Weekly Average Waste: <b>" + String(weekPercent, 1) + "%</b></p>";
  html += "</div>";

  html += "<script>setTimeout(()=>{location.reload();}, 4000);</script>";
  html += "</body></html>";
  return html;
}

void handleRoot() { server.send(200, "text/html", getHTML()); }

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

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED Failed");
  }

  // 1. Initial Load Cell Tare First (Guarantees zero start)
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor);
  delay(500);
  scale.tare(15);

  // 2. Wi-Fi Connection Attempt (Non-blocking: Max 8 seconds)
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

  // Fast check so weight is never blocked
  if (!scale.is_ready()) return;

  float currentWeight = scale.get_units(2);
  if (abs(currentWeight) < ZERO_THRESHOLD) {
    currentWeight = 0.0;
  }

  // Hardware Switch 1: Save Weight
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

  // Hardware Switch 2: Next Day
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

  // Scale State Machine
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

      // Trigger Countdown when weight > 10g
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
        // Fast 5 samples to lock reading without freezing loop
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

      // Plate theeyagane auto zero
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