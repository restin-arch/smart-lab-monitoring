# 🔬 IoT-Based Smart Laboratory Safety, Security & Environmental Monitoring System

Production-ready, standalone firmware and Tiny REST API system for **ESP8266 NodeMCU**, featuring multi-sensor environmental telemetry, gas/smoke detection, ultrasonic distance & perimeter intrusion guard, 6-axis tamper detection, and real-time Web Dashboard + MongoDB REST API backend integration.

---

## 📌 1. Hardware Pinout & Wiring Table

| Hardware Component | NodeMCU Silk | ESP8266 GPIO | Wire / Signal Type | Description |
| :--- | :--- | :--- | :--- | :--- |
| **MQ-2 Gas Sensor AO** | `A0` | ADC0 (GPIO17)| Analog Input | Reads LPG, Smoke, CO & Combustible Gas level (0–1023) |
| **MPU6050 SCL** | `D1` | GPIO5 | I2C Clock | Serial Clock line for 6-axis IMU |
| **MPU6050 SDA** | `D2` | GPIO4 | I2C Data | Serial Data line for 6-axis IMU |
| **DHT11 Data** | `D5` | GPIO14 | Digital / 1-Wire | Temperature & Humidity sensor signal |
| **PIR Motion Sensor**| `D6` | GPIO12 | Digital Input | Motion & Intruder detection signal (HIGH on motion) |
| **HC-SR04 Trig** | `D7` | GPIO13 | Digital Output | Ultrasonic pulse trigger output |
| **HC-SR04 Echo** | `D8` | GPIO15 | Digital Input | Ultrasonic pulse timing input |
| **VCC (3.3V / 5V)** | `3V3` & `VIN` | Power Rail | 3.3V & 5V | 3.3V for DHT11 & MPU6050; 5V (VIN/USB) for MQ-2, PIR & HC-SR04 |
| **GND** | `GND` | Ground | Ground | Common Ground connection across all modules |

---

## ⚡ 2. Circuit Diagram (ASCII Representation)

```
                       +-------------------------------+
                       |        ESP8266 NodeMCU        |
                       |                               |
                       | A0  3V3 GND D1 D2 D5 D6 D7 D8 |
                       +-+----+---+---+--+--+--+--+-+--+
                         |    |   |   |  |  |  |  | |
     +-------------------+    |   |   |  |  |  |  | +--------------------+
     |                        |   |   |  |  |  |  +----------------+     |
     |   +--------------------+   |   |  |  +------------+         |     |
     |   |                        |   |  |               |         |     |
+----+---+----+           +-------+---+--+--+---+       +----+-----+--+  |
|  MQ-2 GAS   |           |    DHT11 SENSOR     |       | PIR MOTION  |  |
|   SENSOR    |           |                     |       |   SENSOR    |  |
| VCC GND AO  |           |   VCC  GND   DATA   |       | VCC GND OUT |  |
+--+---+---+--+           +----+----+-----+-----+       +--+---+---+--+  |
   |   |   |                   |    |     |                |   |   |     |
  5V  GND A0                  3.3V GND   D5               5V  GND D6     |
                                                                         |
+---------------------+                                 +----------------+----+
| MPU6050 (6-DOF IMU) |                                 | HC-SR04 ULTRASONIC  |
| VCC  GND  SCL  SDA  |                                 | VCC GND TRIG ECHO   |
+--+----+----+----+---+                                 +--+---+---+---+------+
   |    |    |    |                                        |   |   |   |
  3.3V GND  D1   D2                                       5V  GND D7  D8
```

---

## 📚 3. Required Arduino IDE Libraries

Install these libraries via **Tools -> Manage Libraries...** in Arduino IDE:

1. **`ESP8266WiFi`** *(Built into ESP8266 core)*
2. **`ESP8266WebServer`** *(Built into ESP8266 core)*
3. **`ESP8266HTTPClient`** *(Built into ESP8266 core)*
4. **`ArduinoJson`** by *Benoît Blanchon* (v6.x or v7.x)
5. **`Wire`** *(Built into Arduino core)*
6. **`DHT sensor library`** by *Adafruit*
7. **`Adafruit MPU6050`** by *Adafruit*
8. **`Adafruit Unified Sensor`** by *Adafruit*

---

## 🌐 4. Features & Operating Logic

1. **HC-SR04 Ultrasonic Distance Guard**:
   - Continuous non-blocking pulse time measurement converting duration to centimeters ($cm$).
   - Triggers `PROXIMITY_INTRUSION` alert when an object/person moves closer than `30.0 cm`.

2. **Combustible Gas & Smoke Guard (MQ-2)**:
   - Analog sampling via pin `A0` reading LPG, smoke, CO, and methane concentration (0–1023 ADC raw value).
   - Triggers `GAS_LEAK_ALERT` when ADC value exceeds `350` (or > 34% gas concentration).

3. **Environmental Telemetry (DHT11)**:
   - Continuous sampling of ambient temperature and relative humidity via DHT11.
   - Triggers `HIGH_TEMP` alert when temperature exceeds `35.0 °C`.

4. **Intrusion Motion Detection (PIR)**:
   - Infrared PIR motion sensor monitors lab occupancy during restricted hours.

5. **Seismic, Vibration & Tamper Guard (MPU6050)**:
   - 6-DOF IMU measures acceleration & angular velocity across X, Y, Z.
   - Detects physical disturbances, equipment displacement, or seismic tremors.

6. **Web Dashboard**:
   - Built-in HTTP server running on Port 80.
   - Live AJAX dashboard displaying real-time sensor metrics, gas percentages, ultrasonic distance, vector grids, and active safety alerts.

---

## 🚀 5. Quick Start Guide

### Step 1: Upload Firmware to ESP8266
1. Open [`IoT_Smart_Lab.ino`](file:///C:/Users/ASUS/.gemini/antigravity-ide/scratch/esp8266-smart-lab/IoT_Smart_Lab.ino) in Arduino IDE.
2. Select Board: `NodeMCU 1.0 (ESP-12E Module)`.
3. Set your WiFi credentials in lines 46–47:
   ```cpp
   const char* WIFI_SSID     = "YOUR_WIFI_NAME";
   const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
   ```
4. Click **Upload**.

### Step 2: Open Web Dashboard
1. Open the Arduino Serial Monitor at `115200 baud`.
2. Find the assigned IP Address (e.g., `http://192.168.1.125`).
3. Open any web browser on your smartphone or PC connected to the same network and navigate to `http://192.168.1.125`.
