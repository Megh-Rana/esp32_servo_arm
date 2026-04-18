# ESP32 Servo Arm

Web-controlled 4-servo robotic arm running on ESP32 WROOM-32.  
Two control modes × two servo libraries = 4 sketches.

---

## Wiring

| Servo | Function | ESP32 Pin |
|-------|----------|-----------|
| SG90 / MG90S | Left / Right | GPIO 13 |
| SG90 / MG90S | Up / Down | GPIO 12 |
| SG90 / MG90S | Forward / Back | GPIO 14 |
| SG90 / MG90S | Grip Open/Close | GPIO 27 |

**Power**
- Servo VCC → external 5V rail (not ESP32 3.3V — 4 servos will brown it out)
- Servo GND → external GND **and** ESP32 GND (common ground required)
- ESP32 powered separately via USB or LiPo + regulator

---

## Files

| File | Control | Library |
|------|---------|---------|
| `v1_buttons_ledc.ino` | 8-button web UI | Raw LEDC (no extra lib) |
| `v1_buttons_esp32servo.ino` | 8-button web UI | ESP32Servo |
| `v2_gyro_ledc.ino` | Phone gyroscope + 2 grip buttons | Raw LEDC (no extra lib) |
| `v2_gyro_esp32servo.ino` | Phone gyroscope + 2 grip buttons | ESP32Servo |

---

## Dependencies

- **LEDC versions** — no extra libraries, just the ESP32 Arduino core
- **ESP32Servo versions** — install [ESP32Servo by Kevin Harrington](https://github.com/madhephaestus/ESP32Servo) via Arduino Library Manager

---

## Usage

### 1. Flash
Open the desired `.ino` in Arduino IDE, select board **ESP32 Dev Module**, and upload.

### 2. Connect
On your phone or laptop, connect to WiFi:
- SSID: `RoboArm`
- Password: `12345678`

### 3. Open the UI
Navigate to `http://192.168.4.1` in a browser.

### Version 1 — Button Control
8 buttons control each servo in both directions. Each tap moves the servo by a fixed step.

### Version 2 — Gyroscope Control
- Tap **Enable Motion Sensor** (required on iOS for permission)
- Tilt your phone to move the arm:
  - Tilt left/right (gamma) → Left/Right servo
  - Tilt forward/back (beta) → Forward/Back servo
  - Rotate/compass (alpha) → Up/Down servo
- Hold **Open** or **Close** buttons for the grip

---

## Battery Tips
- All sketches detach servo PWM signal ~800ms after the last command — servos hold position mechanically and stop drawing idle current
- WiFi TX power is set to 8.5 dBm (reduced from default 20 dBm) — sufficient for close-range use
- Gyro version polls at 10 Hz, keeping CPU and network load low
