#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <WebServer.h>
#include <LiquidCrystal_I2C.h>
#include <Preferences.h>

// --- SHARED OBJECTS ---
extern WebServer server;
extern LiquidCrystal_I2C lcd;
extern Preferences preferences;

// --- SHARED VARIABLES ---
extern bool relayState;
extern bool autoMode;
extern bool isFilling;
extern bool topUpActive;
extern bool flowLockout;
extern bool sumpLockedOut;
extern bool tankLockedOut;
extern bool sumpHasWater;
extern int currentHour;

extern int currentTankPercent;
extern int currentTankDistance;
extern float todayUsage;
extern float yesterdayUsage;

extern unsigned long motorStartTime;
extern unsigned long sumpLockoutStartTime;
extern unsigned long tankLockoutStartTime;
extern unsigned long lastToggleTime;
extern unsigned long webOverrideStartTime;

#endif