#include <Arduino.h>
#include <Wire.h>
#include "Display.h"
#include "Config.h"
#include "Globals.h"

unsigned long lastLcdUpdate = 0;
unsigned long lastRow3Cycle = 0;
bool showYesterday = false;

String padLine(String txt) {
  while(txt.length() < 20) { txt += " "; }
  return txt;
}

void setupDisplay() {
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("System Booting...");
}

void updateDisplay(unsigned long currentMillis) {
  if (currentMillis - lastLcdUpdate >= 1000) {
    lastLcdUpdate = currentMillis;
    
    // Row 0: Volume & %
    String r0 = "TANK:";
    if (currentTankDistance == -1) {
      r0 += " --% (ERROR)";
    } else {
      int curVol = (TANK_EMPTY_CM - currentTankDistance) * LITERS_PER_CM;
      if (curVol < 0) curVol = 0;
      r0 += String(currentTankPercent) + "% (" + String(curVol) + "L)";
    }

    // Row 1: Sump Safety
    String r1 = "SUMP: ";
    if (flowLockout) {
      r1 += "DRY RUN ERROR!";
    } else if (!sumpHasWater) {
      if (sumpLockedOut) r1 += "EMPTY (TIMER)";
      else r1 += "EMPTY";
    } else { 
      if (sumpLockedOut) r1 += "RECOVERING...";
      else r1 += "WATER AVAIL";
    }

    // Row 2: Today
    String r2 = "TODAY USED: " + String((int)todayUsage) + " L";

    // Row 3: Cycling Gamification
    if (currentMillis - lastRow3Cycle >= 10000) {
      lastRow3Cycle = currentMillis;
      showYesterday = !showYesterday;
    }
    
    String r3 = "";
    if (showYesterday) {
      r3 = "YEST: " + String((int)yesterdayUsage) + " L";
    } else {
      if (currentHour == -1) {
        r3 = "WAITING FOR TIME...";
      } else if (currentHour < 12) { 
        // --- MORNING LOGIC (Midnight to 11:59 AM) ---
        if (todayUsage < 200) r3 = ">> GOOD MORNING! <<";
        else r3 = ">> HIGH MORNING USE!";
      } else { 
        // --- AFTERNOON/EVENING LOGIC (12:00 PM to 11:59 PM) ---
        if (yesterdayUsage > 50) { // If we have valid yesterday data to compare to
          if (todayUsage < (yesterdayUsage * 0.8)) {
            r3 = ">> SAVING WATER! <<";
          } else if (todayUsage <= yesterdayUsage) {
            r3 = ">> ON TRACK! <<";
          } else {
            r3 = ">> OVER YESTERDAY! <<";
          }
        } else { // Fallback if yesterday was 0 (e.g., first day running)
          if (todayUsage <= 400) r3 = ">> GREAT JOB! <<";
          else if (todayUsage <= 800) r3 = ">> NORMAL USAGE <<";
          else r3 = ">> HIGH USAGE! <<";
        }
      }
    }

    lcd.setCursor(0, 0); lcd.print(padLine(r0));
    lcd.setCursor(0, 1); lcd.print(padLine(r1));
    lcd.setCursor(0, 2); lcd.print(padLine(r2));
    lcd.setCursor(0, 3); lcd.print(padLine(r3));
  }
}