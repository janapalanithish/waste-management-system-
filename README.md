# Smart Waste Management System

An **ESP32-based Smart Food Waste Management System** designed to measure, record, and monitor food waste using a load cell and HX711 amplifier. The measured waste weight is displayed on an OLED display, and the user can save the measurement using a push button. The system also provides waste-level indication through LEDs and a Wi-Fi-based web dashboard for daily and weekly monitoring.

## 📌 Project Overview

Food waste is difficult to monitor accurately when it is estimated manually. This project provides a simple embedded system that automatically measures the weight of food waste and records the measurements for monitoring.

The **ESP32** acts as the main controller and integrates the load cell, HX711 amplifier, OLED display, push buttons, indicator LEDs, and Wi-Fi web interface.

The system can:

- Measure the weight of food waste.
- Display the measured weight on an OLED display.
- Calculate the percentage of food waste.
- Save the measured waste for the current day.
- Track waste measurements for up to seven days.
- Indicate the waste level using LEDs.
- Provide a Wi-Fi-based web dashboard for monitoring.
- Continue weighing even when Wi-Fi is unavailable.

---

## ⚙️ Main Features

- **Real-time weight measurement**
- **Load cell + HX711-based weighing**
- **OLED display**
- **Daily waste recording**
- **Seven-day waste tracking**
- **Waste percentage calculation**
- **Green, Yellow and Red waste-status indication**
- **Push-button control**
- **ESP32 Wi-Fi connectivity**
- **Web-based monitoring dashboard**
- **Offline weighing support**

---

## 🧩 Hardware Components

| Component | Purpose |
|---|---|
| **ESP32** | Main microcontroller and Wi-Fi communication |
| **Load Cell** | Measures the weight of food waste |
| **HX711** | Amplifies and converts the load-cell signal |
| **OLED Display** | Displays weight, status and waste information |
| **Push Buttons** | Save the measurement and move to the next day |
| **Green LED** | Indicates safe/low waste condition |
| **Yellow LED** | Indicates moderate waste condition |
| **Red LED** | Indicates high waste condition |
| **Breadboard / Connecting Wires** | Circuit connections |
| **Waste Container / Platform** | Holds the waste during measurement |

---

## 💻 Software & Technologies

- **ESP32**
- **Arduino IDE**
- **C/C++**
- **ESP32 WebServer**
- **Wi-Fi**
- **I2C**
- **HX711 Library**
- **Adafruit GFX Library**
- **Adafruit SSD1306 Library**
- **HTML/CSS Web Interface**

---

## 🔄 Working Principle

The system operates in the following sequence:

1. The load cell detects the weight placed on the weighing platform.
2. The **HX711** amplifies and converts the load-cell signal.
3. The **ESP32** reads and processes the weight data.
4. The measured weight is displayed on the OLED.
5. When waste is detected, the system performs a short settling/countdown period to obtain a stable measurement.
6. The final waste weight is calculated and displayed.
7. The waste percentage is calculated using the amount of food prepared.
8. The corresponding LED indicates the waste condition.
9. The user can press the **Save Weight** button to record the measurement.
10. The **Next Day** button can be used to move to the next day's record.
11. The ESP32 also provides the recorded information through its Wi-Fi web dashboard.

---

## 📊 Waste Percentage

The system calculates the percentage of waste using:

```text
Waste Percentage = (Daily Waste / Total Food Prepared) × 100
```

The system maintains daily measurements and can calculate the total waste and average waste percentage over a seven-day monitoring period.

---

## 🚦 Waste-Level Indication

The system uses three LEDs to provide a simple visual indication of the waste condition.

| LED | Condition |
|---|---|
| 🟢 **Green** | Safe / Low Waste |
| 🟡 **Yellow** | Moderate Waste |
| 🔴 **Red** | High Waste |

The OLED also displays the corresponding system status.

---

## 🌐 Web Dashboard

The ESP32 hosts a Wi-Fi-based web interface that allows the user to monitor the system.

The dashboard provides:

- Current day
- Total food prepared
- Live waste weight
- Saved waste weight
- Waste percentage
- Current waste status
- Seven-day waste records
- Total weekly waste
- Weekly average waste percentage
- Save Weight operation
- Next Day operation

The dashboard automatically updates the displayed information while the system is operating.

---

## 🏗️ System Architecture

```text
                FOOD WASTE
                     │
                     ▼
                LOAD CELL
                     │
                     ▼
              HX711 AMPLIFIER
                     │
                     ▼
              ESP32 CONTROLLER
             ┌───────┼────────┐
             │       │        │
             ▼       ▼        ▼
          OLED     LEDs    PUSH BUTTONS
          DISPLAY  STATUS     CONTROL
             │
             ▼
          WI-FI
             │
             ▼
       WEB DASHBOARD
             │
             ▼
     DAILY / WEEKLY DATA
```

---

## 🔌 Prototype

The project is implemented as a working hardware prototype integrating:

- ESP32 microcontroller
- Load cell
- HX711 module
- OLED display
- Indicator LEDs
- Push buttons
- Waste measurement platform

The prototype demonstrates the complete process from **waste measurement to recording and wireless monitoring**.

---
## 🎥 Project Demonstration Video

[▶ Watch the Prototype Demonstration Video](https://drive.google.com/file/d/183LwbrBgz0TicCVMK1CNNcnJVxgE4GBo/view?usp=share_link)

---

## 📁 Project Structure

```text
waste-management-system/
│
├── README.md
├── src/
│   └── ESP32 project source code
│
├── images/
│   └── Project prototype images
│
├── circuit/
│   └── Circuit / wiring diagrams
│
└── documentation/
    └── Project report and related documentation
```

> The folder structure can be adjusted according to the files added to the repository.

---

## 🎯 Applications

This system can be used for monitoring food waste in environments such as:

- College canteens
- Restaurants
- Hostels
- Food service areas
- Institutional kitchens
- Other locations where food waste needs to be measured and monitored

---

## 🚀 Future Improvements

Possible future improvements include:

- Cloud-based long-term data storage
- Historical graphs and analytics
- Mobile-friendly monitoring
- Multiple-bin monitoring
- Automated notifications
- Improved mechanical enclosure
- More advanced waste analysis

---



## 📜 Project Status

**Status:** Working Prototype

The current prototype demonstrates food-waste weight measurement, OLED display, waste-status indication, daily recording, seven-day tracking, and Wi-Fi-based monitoring using an ESP32.
