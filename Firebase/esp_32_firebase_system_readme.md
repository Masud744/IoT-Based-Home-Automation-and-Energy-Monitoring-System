# System Setup README (ESP1 & ESP2)

This document fully explains **ESP1** and **ESP2** setup, hardware requirements, pin connections, Firebase configuration, data flow, and library choices. Anyone reading this file should be able to reproduce the full system from scratch without additional guidance.

---

## 1. Overall System Overview

The system consists of **two ESP-based nodes**:

- **ESP1**: Sensor & Data Publisher Node
- **ESP2**: Control / Monitoring / Secondary Logic Node

Both ESPs communicate **indirectly via Firebase Realtime Database**.

ESP1 pushes real-time sensor and status data to Firebase. ESP2 reads those values and performs decision-making, visualization, or control actions.

---

## 2. ESP1 – Sensor & Data Publisher Node

### 2.1 Purpose of ESP1

ESP1 is responsible for:
- Reading physical sensor data
- Processing raw values (basic filtering / mapping)
- Uploading structured data to Firebase in real time

ESP1 does **NOT** directly communicate with ESP2 using serial, I2C, or WiFi peer-to-peer. Firebase acts as the shared cloud layer.

---

### 2.2 Hardware Required (ESP1)

| Component | Required | Notes |
|---------|---------|------|
| ESP32 / ESP8266 | Yes | Main controller |
| Sensor Module(s) | Yes | Example: ultrasonic, temp, gas, etc |
| Breadboard | Optional | For prototyping |
| Jumper Wires | Yes | Male–Female / Male–Male |
| Power Source | Yes | USB or regulated supply |

---

### 2.3 ESP1 Pin Connections

> Adjust GPIO numbers if your actual hardware differs

| Sensor | Sensor Pin | ESP Pin | Description |
|------|-----------|--------|-------------|
| Ultrasonic | TRIG | GPIO 5 | Trigger output |
| Ultrasonic | ECHO | GPIO 18 | Echo input |
| Temperature | DATA | GPIO 4 | Digital/Analog input |
| GND | GND | GND | Common ground |
| VCC | VCC | 3.3V / 5V | As per sensor |

---

### 2.4 Firebase Role of ESP1

ESP1 **only writes data** to Firebase.

It does not:
- Read commands
- Modify configuration flags

This keeps ESP1 simple and stable.

---

### 2.5 Firebase Data Structure (ESP1)

ESP1 updates the following paths:

```
/esp1/
   sensor1
   sensor2
   status
   timestamp
```

Example payload:

```
/esp1/sensor1 = 42
/esp1/sensor2 = 27.5
/esp1/status  = "ACTIVE"
/esp1/timestamp = 1700000000
```

---

### 2.6 How ESP1 Sends Data to Firebase

Flow:
1. ESP connects to WiFi
2. Firebase authentication initialized
3. Sensor values read
4. Values converted to primitive types (int / float / string)
5. Data pushed using `Firebase.setXXX()` methods

---

### 2.7 Libraries Used in ESP1 (and Why)

| Library | Used | Reason |
|-------|------|-------|
| WiFi.h / ESP8266WiFi.h | Yes | Network connection |
| Firebase_ESP_Client | Yes | Official modern Firebase library |
| ArduinoJson | Yes | JSON handling |

#### Why Other Libraries Are Avoided
- **Old FirebaseArduino**: deprecated, unstable
- **HTTPClient manual REST**: more code, less safety

---

## 3. ESP2 – Control & Monitoring Node

### 3.1 Purpose of ESP2

ESP2 is responsible for:
- Reading data from Firebase
- Applying logic / thresholds
- Displaying data or controlling actuators

ESP2 behaves as the **decision layer**.

---

### 3.2 Hardware Required (ESP2)

| Component | Required | Notes |
|---------|---------|------|
| ESP32 / ESP8266 | Yes | Main controller |
| Display / Actuator | Optional | OLED, relay, motor, etc |
| Jumper Wires | Yes | Connections |
| Power Source | Yes | Stable power |

---

### 3.3 ESP2 Pin Connections

| Device | Device Pin | ESP Pin | Description |
|------|-----------|--------|-------------|
| OLED | SDA | GPIO 21 | I2C Data |
| OLED | SCL | GPIO 22 | I2C Clock |
| Relay | IN | GPIO 26 | Control signal |
| GND | GND | GND | Common ground |
| VCC | VCC | 3.3V | Power |

---

### 3.4 Firebase Role of ESP2

ESP2:
- Reads ESP1 sensor values
- Optionally writes control commands

ESP2 may update:

```
/esp2/command
/esp2/mode
```

---

### 3.5 Firebase Read Flow (ESP2)

1. Connect to WiFi
2. Initialize Firebase
3. Read `/esp1/*` values
4. Apply conditions
5. Trigger outputs

Example condition:

```
If sensor1 > threshold → turn relay ON
```

---

### 3.6 Firebase Data Used by ESP2

| Path | Type | Purpose |
|----|-----|--------|
| /esp1/sensor1 | Integer | Decision logic |
| /esp1/sensor2 | Float | Monitoring |
| /esp1/status | String | System state |

---

### 3.7 Libraries Used in ESP2

Same core libraries as ESP1 for compatibility:

| Library | Reason |
|------|-------|
| WiFi.h | Network |
| Firebase_ESP_Client | Firebase sync |
| ArduinoJson | Parsing |

Using the same library set avoids:
- Version conflicts
- Inconsistent data types

---

## 4. Firebase Setup (One-Time)

### 4.1 What You MUST Create in Firebase

1. Firebase Project
2. Realtime Database
3. Database Rules
4. Web API Key

---

### 4.2 Realtime Database Rules (Testing Mode)

```
{
  "rules": {
    ".read": true,
    ".write": true
  }
}
```

Production systems should **not** use open rules.

---

### 4.3 Firebase Credentials Used in Code

Both ESPs require:

- API Key
- Database URL
- User Email (or anonymous auth)
- Password

These are defined once and reused.

---

## 5. Why Firebase is Used Instead of Direct ESP Communication

| Option | Reason Rejected |
|-----|----------------|
| ESP-NOW | Limited range |
| MQTT Broker | Extra server setup |
| Direct REST | Hard to scale |
| Firebase | Cloud sync, history, dashboard-ready |

Firebase allows:
- Logging
- Visualization
- Multi-device access

---

## 6. Data Flow Summary

```
[Sensors] → ESP1 → Firebase → ESP2 → Output
```

No direct ESP-to-ESP dependency exists.

---

## 7. Final Notes

- ESP1 should be powered and stable first
- ESP2 depends on ESP1 data
- Firebase structure must remain consistent
- Any path change must be updated in both codes

This README is intentionally detailed so future team members or evaluators can understand the full system without external explanation.

