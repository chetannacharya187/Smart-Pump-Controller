#ifndef CONFIG_H
#define CONFIG_H

// --- WI-FI CREDENTIALS ---
// Remember to change these before pushing to GitHub!
static const char* primarySSID = "YOUR_WIFI_NAME";
static const char* primaryPASS = "YOUR_WIFI_PASSWORD";
static const char* secondarySSID = "YOUR_BACKUP_WIFI_NAME";
static const char* secondaryPASS = "YOUR_BACKUP_PASSWORD";

// --- HARDWARE PINS ---
const int RELAY_PIN = 4;
const int SUMP_FLOAT_PIN = 19; 
const int TRIG_PIN = 5;        
const int ECHO_PIN = 18;       

// --- TANK CALIBRATION ---
const int TANK_EMPTY_CM = 108; 
const int TANK_FULL_CM = 25;   
const float LITERS_PER_CM = 19.2; 

// --- TIMERS (in milliseconds) ---
const unsigned long FLOW_CHECK_INTERVAL = 4UL * 60UL * 1000UL; 
const unsigned long SUMP_LOCKOUT_DURATION = 20UL * 60UL * 1000UL; 
const unsigned long TANK_LOCKOUT_DURATION = 50UL * 60UL * 1000UL; 
const unsigned long WEB_OVERRIDE_TIMEOUT = 15UL * 60UL * 1000UL; 
const unsigned long PING_INTERVAL = 2000; 

#endif