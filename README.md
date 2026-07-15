# -IoT-Based-Robotic-Arm-using-ESP8266
A real-time embedded robotic arm controlled through a browser-based interface using ESP8266, WebSockets, I2C, and the PCA9685 PWM Servo Driver.

## 📖 Overview

This project implements a "6-Degree-of-Freedom (6-DOF) robotic arm" that can be controlled wirelessly through any web browser. The ESP8266 microcontroller creates its own Wi-Fi Access Point, hosts a web interface, and communicates with a PCA9685 PWM driver over the I2C protocol to control six servo motors.

The system also supports *motion recording and playback*, allowing users to record a sequence of arm movements and replay them automatically.

---

## ✨ Features

- 🌐 Wi-Fi based robotic arm control
- 📱 Browser-based user interface
- 🎛️ Real-time servo control using sliders
- 🔄 Motion recording and playback
- 🔁 Loop playback mode
- ⚡ Low-latency communication using WebSockets
- 🎯 Precise servo control using PCA9685

---

## 🛠️ Hardware Used

- ESP8266 NodeMCU
- PCA9685 16-Channel PWM Servo Driver
- MG995 Servo Motors (High Torque)
- SG90 Servo Motors (Micro Servo)
- External 5V Power Supply
- Jumper Wires
- 6-DOF Robotic Arm

---

## 💻 Software & Technologies

- Arduino IDE
- Embedded C++
- HTML
- CSS
- JavaScript
- WebSockets
- I2C Communication Protocol

---

## 📂 Project Architecture


User
   │
   ▼
Web Browser
   │
   ▼
HTML + CSS + JavaScript
   │
   ▼
WebSocket Communication
   │
   ▼
ESP8266 Web Server
   │
   ▼
I2C Communication
   │
   ▼
PCA9685 PWM Driver
   │
   ▼
PWM Signals
   │
   ▼
Servo Motors
   │
   ▼
Robotic Arm Movement


---

## ⚙️ Working Principle

1. ESP8266 creates a Wi-Fi Access Point.
2. User connects to the Wi-Fi network using a mobile phone or laptop.
3. The browser loads the control webpage hosted by the ESP8266.
4. Slider movements are sent to the ESP8266 using WebSockets.
5. ESP8266 processes the received commands.
6. Servo angle values are transmitted to the PCA9685 over I2C.
7. PCA9685 generates precise PWM signals.
8. Servo motors rotate to the required angles, moving the robotic arm.

---

## 📡 Communication Flow
Browser
      │
      ▼
WebSocket
      │
      ▼
ESP8266
      │
      ▼
I2C
      │
      ▼
PCA9685
      │
      ▼
PWM
      │
      ▼
Servo Motors

