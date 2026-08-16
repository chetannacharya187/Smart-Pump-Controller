# Smart-Pump-IoT-Controller 💧
<img width="170" alt="Auto Mode" src="https://github.com/user-attachments/assets/c6a49657-5d62-4b3d-985b-2e52333a2d2f" />
<img width="170" alt="Auto Mode" src="https://github.com/user-attachments/assets/14cd7ef7-1d52-4224-8327-817ea34ef94a" />
<img width="170" alt="Auto Mode" src="https://github.com/user-attachments/assets/2cc1fb51-d679-45ee-af7a-7563283ee925" />
<img width="170" alt="Auto Mode" src="https://github.com/user-attachments/assets/57479552-3bad-4287-a84d-953178ea3eb0" />
<img width="170" alt="Auto Mode" src="https://github.com/user-attachments/assets/a2adc0b2-0b2e-4cf9-a26f-06b967cebc7c" />
<img width="170" alt="Auto Mode" src="https://github.com/user-attachments/assets/05a89169-224c-4f76-9980-ab3c90188244" />
<img width="170" alt="Auto Mode" src="https://github.com/user-attachments/assets/8d69b3b2-ffe4-4df8-b08d-ecde2e5fc2a1" />
<img width="450" alt="Auto Mode" src="https://github.com/user-attachments/assets/5ee5cb83-40bb-4f16-b284-13fac14b87ad" />


An industrial-grade, ESP32-based fully automated water pump controller built with a modular C++ architecture in PlatformIO. 

This system manages a 1HP induction motor to transfer water from an underground sump to a 1480L rooftop tank. It replaces unreliable mechanical float switches with an ultrasonic sensor, dynamic state-machine logic, and a responsive local web dashboard.

## 🚀 Key Engineering Features

* **Master Sump Debounce Engine:** A custom software debounce state machine filters out water ripples and dynamic drawdown, preventing motor dry-fires and false lockouts.
* **Virtual Water Meter:** Tracks exact water usage locally by calculating ultrasonic deltas (litres per cm) when the motor is off.
* **Flash Wear-Leveling:** Non-volatile memory (`Preferences.h`) saves water usage data only at 250L intervals to prevent burning out the ESP32's flash memory.
* **Time-Aware Resource Capture:** Uses NTP (Network Time Protocol) to automatically shift the refill trigger from 60% to 90% between 11 PM and 7 AM to maximise municipal water storage.
* **Background Wi-Fi Roaming:** Non-blocking connection logic seamlessly hops between primary and secondary Wi-Fi networks if the router drops, without interrupting motor safety checks.

## 🛠️ Hardware Stack
* **Microcontroller:** ESP32 DOIT DevKit V1
* **Distance Sensor:** JSN-SR04T (Waterproof Ultrasonic)
* **Float Sensor:** FS37A (Sump Dry-Run Protection)
* **Relay:** 40A Solid State / Electromechanical Contactor (for 1HP Motor)
* **Display:** 20x4 I2C LCD

## 💻 Software Architecture
Built using **PlatformIO**, the codebase is separated into modular components:
* `Network.cpp`: Handles Wi-Fi roaming, WebServer API endpoints, and OTA.
* `Sensors.cpp`: Manages ultrasonic math, sanity filters (ignoring> 15 cm noise jumps), and the master float state machine.
* `Display.cpp`: Manages flicker-free string padding and UI rendering.
* `main.cpp`: The master orchestrator that bridges components via `Globals.h`.

## 📱 Local Web Dashboard
The ESP32 hosts a responsive, dark-mode web dashboard featuring:
* Real-time tank percentage, volume (L), and motor runtime stopwatches.
* Sump recovery countdowns and diagnostic warnings.
* Hardware-locked "God Mode" (Manual Override) with a strict 15-minute safety timeout.
* Daily and Yesterday water usage tracking.
