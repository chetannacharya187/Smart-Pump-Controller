# Smart-Pump-IoT-Controller 💧

An industrial-grade, ESP32-based fully automated water pump controller built with a modular C++ architecture in PlatformIO. 

This system manages a 1HP induction motor to transfer water from an underground sump to a 1480L rooftop tank. It replaces unreliable mechanical float switches with an ultrasonic sensor, dynamic state-machine logic, and a responsive local web dashboard.

## 🚀 Key Engineering Features

* **Master Sump Debounce Engine:** A custom software debounce state-machine filters out water ripples and dynamic drawdown, preventing motor dry-fires and false lockouts.
* **Virtual Water Meter:** Tracks exact water usage locally by calculating ultrasonic deltas (liters per cm) when the motor is off.
* **Flash Wear-Leveling:** Non-volatile memory (`Preferences.h`) saves water usage data only at 250L intervals to prevent burning out the ESP32's flash memory.
* **Time-Aware Resource Capture:** Uses NTP (Network Time Protocol) to automatically shift the refill trigger from 60% to 90% between 11 PM and 7 AM to maximize municipal water storage.
* **Background Wi-Fi Roaming:** Non-blocking connection logic seamlessly hops between primary and secondary Wi-Fi networks if the router drops, without interrupting motor safety checks.
* **OTA Updates (mDNS):** Fully supports Over-The-Air firmware flashing via `pump-ota.local`.

## 🛠️ Hardware Stack
* **Microcontroller:** ESP32 DOIT DevKit V1
* **Distance Sensor:** JSN-SR04T (Waterproof Ultrasonic)
* **Float Sensor:** FS37A (Sump Dry-Run Protection)
* **Relay:** 40A Solid State / Electromechanical Contactor (for 1HP Motor)
* **Display:** 20x4 I2C LCD

## 💻 Software Architecture
Built using **PlatformIO**, the codebase is separated into modular components:
* `Network.cpp`: Handles Wi-Fi roaming, WebServer API endpoints, and OTA.
* `Sensors.cpp`: Manages ultrasonic math, sanity filters (ignoring >15cm noise jumps), and the master float state machine.
* `Display.cpp`: Manages flicker-free string padding and UI rendering.
* `main.cpp`: The master orchestrator that bridges components via `Globals.h`.

## 📱 Local Web Dashboard
The ESP32 hosts a responsive, dark-mode web dashboard featuring:
* Real-time tank percentage, volume (L), and motor runtime stopwatches.
* Sump recovery countdowns and diagnostic warnings.
* Hardware-locked "God Mode" (Manual Override) with a strict 15-minute safety timeout.
* Daily and Yesterday water usage tracking.
