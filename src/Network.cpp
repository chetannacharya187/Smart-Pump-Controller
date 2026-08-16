#include <WiFi.h>
#include <ArduinoOTA.h>
#include "Network.h"
#include "Config.h"
#include "Globals.h"
#include "WebDashboard.h"

unsigned long lastWifiCheck = 0;
bool usingSecondary = false;
bool wasDisconnected = false;

// --- SERVER API ENDPOINTS ---
void handleRoot() { server.send(200, "text/html", PAGE_HTML); }

void handleStatus() { 
  long sumpTimeLeft = 0;
  if (sumpLockedOut) { sumpTimeLeft = (SUMP_LOCKOUT_DURATION - (millis() - sumpLockoutStartTime)) / 1000; }

  long tankTimeLeft = 0;
  if (tankLockedOut) { tankTimeLeft = (TANK_LOCKOUT_DURATION - (millis() - tankLockoutStartTime)) / 1000; }

  long cooldownLeft = 0;
  if (millis() - lastToggleTime < 5000) {
    cooldownLeft = (5000 - (millis() - lastToggleTime)) / 1000;
    if (cooldownLeft == 0) cooldownLeft = 1;
  }

  long motorRunTime = 0;
  if (relayState) { motorRunTime = (millis() - motorStartTime) / 1000; }

  int litersLeft = 0;
  if (currentTankDistance <= TANK_EMPTY_CM && currentTankDistance > 0) {
    litersLeft = (TANK_EMPTY_CM - currentTankDistance) * LITERS_PER_CM;
  }

  String json = "{";
  json += "\"litersLeft\":" + String(litersLeft) + ",";
  json += "\"motorRunTime\":" + String(motorRunTime) + ",";
  json += "\"cooldown\":" + String(cooldownLeft) + ","; 
  json += "\"relay\":" + String(relayState) + ",";
  json += "\"auto\":" + String(autoMode) + ",";
  json += "\"tankPct\":" + String(currentTankPercent) + ",";
  json += "\"tankCm\":" + String(currentTankDistance) + ",";
  json += "\"tankLocked\":" + String(tankLockedOut) + ",";
  json += "\"tankTimeLeft\":" + String(tankTimeLeft) + ",";
  json += "\"sumpHasWater\":" + String(sumpHasWater) + ",";
  json += "\"sumpLocked\":" + String(sumpLockedOut) + ",";
  json += "\"sumpTimeLeft\":" + String(sumpTimeLeft) + ",";
  json += "\"flowLocked\":" + String(flowLockout) + ","; 
  json += "\"topUpActive\":" + String(topUpActive) + ",";
  json += "\"todayUsage\":" + String((int)todayUsage) + ","; 
  json += "\"yesterdayUsage\":" + String((int)yesterdayUsage) + ","; 
  json += "\"network\":\"" + (WiFi.status() == WL_CONNECTED ? WiFi.SSID() : String("Disconnected")) + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

void handleSetManual() {
  autoMode = false;
  if (relayState) { webOverrideStartTime = millis(); }
  server.send(200, "text/plain", "OK");
}

void handleToggleTopUp() {
  topUpActive = !topUpActive;
  server.send(200, "text/plain", "OK");
}

void handleToggle() {
  if (millis() - lastToggleTime < 5000) { server.send(429, "text/plain", "Too Fast"); return; }
  lastToggleTime = millis(); 

  relayState = !relayState;
  autoMode = false; 
  if (relayState) { webOverrideStartTime = millis(); }
  digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
  server.send(200, "text/plain", "OK");
}

void handleResetAuto() {
  if (millis() - lastToggleTime < 5000) { server.send(429, "text/plain", "Too Fast"); return; }
  if (relayState == true) { lastToggleTime = millis(); }
  
  autoMode = true;
  sumpLockedOut = false; 
  tankLockedOut = false;
  flowLockout = false; 
  isFilling = false; 
  digitalWrite(RELAY_PIN, LOW); 
  relayState = false;
  server.send(200, "text/plain", "OK");
}

// --- NETWORK INITIALIZATION ---
void setupNetwork() {
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(primarySSID, primaryPASS);

  lcd.setCursor(0,1);
  lcd.print("Connecting WiFi...");
  
  Serial.print("Connecting to Primary Wi-Fi");
  int bootTimeout = 0;
  while (WiFi.status() != WL_CONNECTED && bootTimeout < 30) {
    delay(1000);
    Serial.print(".");
    bootTimeout++;
  }
  Serial.println("");

  // Setup Time (IST)
  configTime(19800, 0, "pool.ntp.org", "time.nist.gov");
  delay(2000); 

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP()); 
  } 
  
  ArduinoOTA.begin();
  
  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/setManual", handleSetManual);
  server.on("/toggleTopUp", handleToggleTopUp);
  server.on("/toggle", handleToggle);
  server.on("/resetAuto", handleResetAuto);
  server.begin();

  lastWifiCheck = millis();
}

void handleWiFi() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastWifiCheck >= 10000) {
    lastWifiCheck = currentMillis;

    if (WiFi.status() != WL_CONNECTED) {
      wasDisconnected = true;
      Serial.println("Wi-Fi Lost. Attempting Background Reconnect...");
      WiFi.disconnect(true);
      delay(50);
      WiFi.mode(WIFI_STA);
      
      if (usingSecondary) {
         WiFi.begin(primarySSID, primaryPASS);
         usingSecondary = false;
      } else {
         WiFi.begin(secondarySSID, secondaryPASS);
         usingSecondary = true;
      }
    } else {
      if (wasDisconnected) {
        Serial.print("Network Hopped! IP: ");
        Serial.println(WiFi.localIP());
        wasDisconnected = false;
      }
      if (usingSecondary && (currentMillis % (5UL * 60UL * 1000UL) < 10000)) {
        Serial.println("Jumping back to Primary Wi-Fi...");
        WiFi.disconnect(true);
        delay(50);
        WiFi.mode(WIFI_STA);
        WiFi.begin(primarySSID, primaryPASS);
        usingSecondary = false;
      }
    }
  }
}

void handleClient() { server.handleClient(); }
void handleOTA() { ArduinoOTA.handle(); }