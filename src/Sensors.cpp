#include <Arduino.h>
#include "Sensors.h"
#include "Config.h"
#include "Globals.h"

int lastValidDistance = -1;
int lastTrackedDistance = -1;
int consecutiveErrors = 0;
unsigned long lastPingTime = 0;

const int MAX_JUMP_CM = 15; 
const int MAX_ERRORS = 5;

void setupSensors() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

int getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return 999; 
  return duration * 0.034 / 2;
}

void updateSensors(unsigned long currentMillis) {
  if (currentMillis - lastPingTime >= PING_INTERVAL) {
    lastPingTime = currentMillis;
    int rawDistance = getDistance();
    
    // Sanity Filter
    if (lastValidDistance == -1) {
      if (rawDistance != 999 && rawDistance > 0) {
        lastValidDistance = rawDistance;
        consecutiveErrors = 0;
      }
    } else {
      if (rawDistance != 999 && rawDistance > 0 && abs(rawDistance - lastValidDistance) <= MAX_JUMP_CM) {
        lastValidDistance = rawDistance;
        consecutiveErrors = 0;
      } else {
        consecutiveErrors++;
      }
    }

    // Error Handling
    if (consecutiveErrors >= MAX_ERRORS) {
      lastValidDistance = -1; 
      lastTrackedDistance = -1; 
      if (relayState == true && autoMode) { 
        relayState = false;
        isFilling = false; 
        digitalWrite(RELAY_PIN, LOW);
        tankLockedOut = true;
        tankLockoutStartTime = currentMillis;
        Serial.println("SAFETY SHUTOFF: Sensor lost track.");
      }
    }

    currentTankDistance = lastValidDistance;

    // Calculate Usage & Percent
    if (currentTankDistance != -1) {
      if (currentTankDistance >= TANK_EMPTY_CM) {
        currentTankPercent = 0;
      } else if (currentTankDistance <= TANK_FULL_CM) {
        currentTankPercent = 100;
      } else {
        currentTankPercent = map(currentTankDistance, TANK_EMPTY_CM, TANK_FULL_CM, 0, 100);
      }

      if (lastTrackedDistance == -1) {
        lastTrackedDistance = currentTankDistance;
      } else {
        if (!relayState) {
          int drop = currentTankDistance - lastTrackedDistance;
          if (drop >= 2) { 
            todayUsage += drop * LITERS_PER_CM;
            lastTrackedDistance = currentTankDistance;
          } else if (drop <= -2) {
            lastTrackedDistance = currentTankDistance;
          }
        } else {
          lastTrackedDistance = currentTankDistance;
        }
      }
    }
  }
}