#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WebServer.h>
#include <Preferences.h>
#include <time.h>

#include "Config.h"
#include "Globals.h"
#include "Network.h"
#include "Sensors.h"
#include "Display.h"

// --- INSTANTIATE SHARED OBJECTS ---
WebServer server(80);
LiquidCrystal_I2C lcd(0x27, 20, 4);
Preferences preferences;

// --- INSTANTIATE SHARED VARIABLES ---
bool relayState = false;
bool autoMode = true; 
bool isFilling = false;
bool topUpActive = false;
bool flowLockout = false;
bool sumpLockedOut = false;
bool tankLockedOut = false;
bool sumpHasWater = false;

int currentTankPercent = 0;
int currentTankDistance = -1; 
float todayUsage = 0.0;
float yesterdayUsage = 0.0;

unsigned long motorStartTime = 0;
unsigned long sumpLockoutStartTime = 0;
unsigned long tankLockoutStartTime = 0;
unsigned long lastToggleTime = 0;
unsigned long webOverrideStartTime = 0;

// --- LOCAL VARIABLES ---
float lastSavedUsage = 0.0;
int currentDay = -1;
int lastCheckedHour = -1;
bool lastRawSump = false;
unsigned long lastSumpDebounceTime = 0;
unsigned long flowCheckStartTime = 0;
int lastFlowCheckDistance = -1;
const int MIN_FLOW_CM = 2;

void setup() {
  Serial.begin(115200);
  
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  pinMode(SUMP_FLOAT_PIN, INPUT_PULLUP); 

  delay(50); 
  sumpHasWater = (digitalRead(SUMP_FLOAT_PIN) == LOW);
  lastRawSump = sumpHasWater;

  // Initialize Modules
  setupDisplay();
  setupSensors();
  
  // Load Memory
  preferences.begin("waterData", false);
  todayUsage = preferences.getFloat("today", 0.0);
  yesterdayUsage = preferences.getFloat("yesterday", 0.0);
  lastSavedUsage = todayUsage;

  // Connect to Internet
  setupNetwork();
}

void loop() {
  handleOTA();
  handleClient();
  handleWiFi();

  unsigned long currentMillis = millis();

  // Edge detection
  static bool lastRelayState = false;
  if (relayState && !lastRelayState) {
    motorStartTime = currentMillis;
    lastRelayState = true;
  } else if (!relayState && lastRelayState) {
    lastRelayState = false;
  }

  // 0. MASTER SUMP DEBOUNCE ENGINE
  bool rawSump = (digitalRead(SUMP_FLOAT_PIN) == LOW);
  if (rawSump != lastRawSump) {
    lastSumpDebounceTime = currentMillis; 
  }
  if ((currentMillis - lastSumpDebounceTime) >= 3000) {
    sumpHasWater = rawSump; 
  }
  lastRawSump = rawSump;

  // 1. UPDATE SENSORS
  updateSensors(currentMillis);

  // 2. MEMORY WEAR-LEVELING
  if (todayUsage - lastSavedUsage >= 250.0) {
    preferences.putFloat("today", todayUsage);
    lastSavedUsage = todayUsage;
    Serial.println("FLASH SAVED: 250L Milestone Reached.");
  }

  // 3. NTP MIDNIGHT ROLLOVER & TOP-UP SCHEDULE
  static unsigned long lastTimeCheck = 0;
  if (currentMillis - lastTimeCheck >= 60000) {
    lastTimeCheck = currentMillis;
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 0)) { 
      
      // Midnight Reset
      if (currentDay == -1) {
        currentDay = timeinfo.tm_mday;
      } else if (currentDay != timeinfo.tm_mday) {
        yesterdayUsage = todayUsage;
        todayUsage = 0.0;
        lastSavedUsage = 0.0;
        preferences.putFloat("yesterday", yesterdayUsage);
        preferences.putFloat("today", 0.0);
        currentDay = timeinfo.tm_mday;
      }

      // Automatic Top-Up Logic
      if (lastCheckedHour == -1) {
        lastCheckedHour = timeinfo.tm_hour;
        if (timeinfo.tm_hour >= 23 || timeinfo.tm_hour < 7) topUpActive = true;
        else topUpActive = false;
      } else if (lastCheckedHour != timeinfo.tm_hour) {
        if (timeinfo.tm_hour == 23) topUpActive = true; 
        else if (timeinfo.tm_hour == 7) topUpActive = false; 
        lastCheckedHour = timeinfo.tm_hour;
      }
    }
  }

  // 4. UPDATE LCD DISPLAY
  updateDisplay(currentMillis);

  // 5. CHECK TIMERS
  if (sumpLockedOut && (currentMillis - sumpLockoutStartTime >= SUMP_LOCKOUT_DURATION)) { sumpLockedOut = false; }
  if (tankLockedOut && (currentMillis - tankLockoutStartTime >= TANK_LOCKOUT_DURATION)) { tankLockedOut = false; }

  // 6. GOD MODE SAFETY TIMEOUT
  if (!autoMode && relayState) {
    if (currentMillis - webOverrideStartTime >= WEB_OVERRIDE_TIMEOUT) {
      relayState = false;
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("GOD MODE MAX TIME REACHED (15 MIN). Pump Stopped.");
    }
  }

  // 7. FULLY AUTOMATIC LOGIC
  if (autoMode && currentTankDistance != -1) {
    
    // Sump Dry Run Protection
    if (!sumpHasWater && !sumpLockedOut) {
      sumpLockedOut = true;
      sumpLockoutStartTime = currentMillis;
      relayState = false;
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("AUTO: SUMP EMPTY! Pump Stopped. Lockout Started.");
    }

    // Virtual Flow Switch Watchdog
    if (relayState && !flowLockout) {
      if (flowCheckStartTime == 0) {
        flowCheckStartTime = currentMillis;
        lastFlowCheckDistance = currentTankDistance;
      } else if (currentMillis - flowCheckStartTime >= FLOW_CHECK_INTERVAL) {
        int waterRise = lastFlowCheckDistance - currentTankDistance; 
        if (waterRise < MIN_FLOW_CM) {
          flowLockout = true;
          relayState = false;
          digitalWrite(RELAY_PIN, LOW);
          lastToggleTime = currentMillis; 
          Serial.println("SAFETY SHUTOFF: Virtual Flow Switch detected Dry Run!");
        } else {
          flowCheckStartTime = currentMillis;
          lastFlowCheckDistance = currentTankDistance;
        }
      }
    } else if (!relayState) {
      flowCheckStartTime = 0; 
    }
    
    // Memory Flag Logic
    int triggerLimit = topUpActive ? 90 : 60;
    if (currentTankPercent <= triggerLimit && !tankLockedOut && !flowLockout) {
      if (!isFilling) { isFilling = true; }
    } else if (currentTankPercent >= 100) {
      if (isFilling) {
        isFilling = false;
        relayState = false;
        digitalWrite(RELAY_PIN, LOW);
        tankLockedOut = true;
        tankLockoutStartTime = currentMillis;
      }
    }

    // Execute Pump
    if (isFilling && sumpHasWater && !sumpLockedOut && !tankLockedOut && !flowLockout) {
      if (!relayState) {
        relayState = true;
        digitalWrite(RELAY_PIN, HIGH);
        lastToggleTime = currentMillis; 
      }
    } else {
      if (relayState) {
        relayState = false;
        digitalWrite(RELAY_PIN, LOW);
        lastToggleTime = currentMillis; 
      }
    }
  }
}