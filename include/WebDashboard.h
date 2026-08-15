#ifndef WEBDASHBOARD_H
#define WEBDASHBOARD_H

#include <Arduino.h> // Needed for PROGMEM

const char PAGE_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Smart Pump Dashboard</title>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; margin-top: 30px; background-color: #121212; color: #ffffff; }
    .card { background-color: #1e1e1e; padding: 30px; border-radius: 12px; display: inline-block; box-shadow: 0 4px 8px rgba(0,0,0,0.5); min-width: 300px; }
    .button { padding: 15px 30px; font-size: 20px; cursor: pointer; border: none; border-radius: 8px; color: white; transition: 0.3s; width: 100%; font-weight: bold; margin-bottom: 15px; }
    .reset { background-color: #ff9800; display: none; }
    .reset:disabled { opacity: 0.5; cursor: not-allowed; }
    h2 { margin-top: 0; }
    .level-text { font-size: 22px; font-weight: bold; margin-bottom: 5px; }
    .sub-text { font-size: 14px; color: #aaaaaa; margin-bottom: 20px; }
    .status-good { color: #4CAF50; font-weight: bold; }
    .status-bad { color: #f44336; font-weight: bold; }
    .status-wait { color: #ff9800; font-weight: bold; }
    #wifiStatus { font-size: 12px; color: #888; margin-top: 20px; }
    
    .btn-override { background-color: #8b0000; color: white; width: 100%; padding: 15px; border-radius: 8px; font-weight: bold; font-size: 18px; border: none; cursor: pointer; margin-bottom: 15px;}
    .god-mode-panel { background: #2a0000; padding: 20px; border-radius: 8px; margin-bottom: 15px; display: none; border: 2px solid #f44336; }
    
    .switch-container { display: flex; align-items: center; justify-content: center; gap: 15px; margin: 15px 0; }
    .switch { position: relative; display: inline-block; width: 60px; height: 34px; }
    .switch input { opacity: 0; width: 0; height: 0; }
    .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #555; transition: .3s; border-radius: 34px; border: 2px solid #333;}
    .slider:before { position: absolute; content: ""; height: 26px; width: 26px; left: 2px; bottom: 2px; background-color: white; transition: .3s; border-radius: 50%; box-shadow: 0 2px 5px rgba(0,0,0,0.5);}
    input:checked + .slider { background-color: #f44336; border-color: #8b0000; }
    input:checked + .slider:before { transform: translateX(26px); }
    input:disabled + .slider { opacity: 0.3; cursor: not-allowed; }
    
    .usage-box { border-top: 1px solid #333; margin-top: 20px; padding-top: 20px; }
    .usage-val { font-size: 24px; font-weight: bold; color: #00bcd4; }
  </style>
</head>
<body>
  <div class="card">
    <h2>System Status</h2>
    
    <div class="level-text">Tank: <span id="tankPct">--%</span></div>
    <div class="sub-text" id="tankStatus">Distance: <span id="tankCm">--</span> cm</div>
    
    <div class="level-text" style="font-size: 18px;">Sump Status:</div>
    <div class="sub-text" id="sumpStatus" style="font-size: 16px; margin-bottom: 30px;">Checking...</div>
    <div id="pumpStateBadge" style="font-size: 20px; font-weight: bold; margin-bottom: 5px; color: #888; letter-spacing: 1px;">-- System Booting... --</div>
    <div id="stopwatchDisplay" style="font-size: 18px; color: #00bcd4; font-weight: bold; margin-bottom: 20px; display: none;">Runtime: 00m 00s</div>
    
    <button id="overrideBtn" class="btn-override" onclick="enterGodMode()">!! ENTER MANUAL OVERRIDE !!</button>
    
    <div id="godModePanel" class="god-mode-panel">
      <div style="color: #f44336; font-weight: bold; margin-bottom: 5px; font-size: 18px;">!! GOD MODE ACTIVE !!</div>
      <div style="font-size: 12px; color: #ffaaaa; margin-bottom: 15px;">Automated sensors & limits disabled.</div>
      
      <div class="switch-container">
        <span style="font-size: 16px; font-weight: bold; color: #888;" id="lblOff">OFF</span>
        <label class="switch">
          <input type="checkbox" id="pumpToggle" onclick="handleToggleClick(event)">
          <span class="slider"></span>
        </label>
        <span style="font-size: 16px; font-weight: bold; color: #888;" id="lblOn">ON</span>
      </div>
      <div id="cooldownText" style="color: #ff9800; font-weight: bold; font-size: 14px; height: 16px;"></div>
    </div>

    <button id="resetBtn" class="button reset" onclick="resetAuto()">Resume Auto Mode</button>
    
    <!-- NEW: Top-Up Mode Toggle -->
    <div style="border-top: 1px solid #333; margin-top: 20px; padding-top: 20px; display: flex; justify-content: space-between; align-items: center;">
      <div style="text-align: left;">
        <div style="font-weight: bold; font-size: 16px;">Top-Up Mode (90%)</div>
        <div style="font-size: 12px; color: #aaa;">Auto-Enables: 11PM to 7AM</div>
      </div>
      <label class="switch">
        <input type="checkbox" id="topUpToggle" onclick="handleTopUpClick(event)">
        <span class="slider"></span>
      </label>
    </div>

    <!-- NEW: Water Tracker for Dashboard -->
    <div class="usage-box" style="display: flex; justify-content: space-around; text-align: center;">
      <div>
        <div style="font-size: 14px; color: #aaa; margin-bottom: 5px;">Yesterday</div>
        <div class="usage-val" style="color: #888;"><span id="yesterdayVal">0</span> L</div>
      </div>
      <div>
        <div style="font-size: 14px; color: #aaa; margin-bottom: 5px;">Today</div>
        <div class="usage-val"><span id="usageVal">0</span> L</div>
      </div>
    </div>

    <div id="wifiStatus">Network: <span id="networkName">--</span></div>
  </div>

  <script>
    let frontendOverride = false;
    let isCooldownActive = false;

    function formatTime(seconds) {
      let m = Math.floor(seconds / 60);
      let s = seconds % 60;
      return m + "m " + s + "s";
    }

    function updateUI() {
      fetch('/status').then(res => res.json()).then(data => {
        
        frontendOverride = (data.auto === 0); 
        
        if (data.cooldown > 0 && !isCooldownActive) {
          if (frontendOverride) {
            document.getElementById('pumpToggle').checked = (data.relay === 1);
            document.getElementById('lblOn').style.color = data.relay === 1 ? "#f44336" : "#555";
            document.getElementById('lblOff').style.color = data.relay === 1 ? "#555" : "#ccc";
          }
          startCooldown(data.cooldown);
        }
        
        if (data.tankCm === -1) {
           document.getElementById('tankPct').innerText = "--%";
           document.getElementById('tankStatus').innerText = "Initializing Sensor...";
        } else {
           document.getElementById('tankPct').innerText = data.tankPct + "% (" + data.litersLeft + " L)";
           let tankDiv = document.getElementById('tankStatus');
           if (data.tankLocked === 1) {
             tankDiv.innerHTML = "<span class='status-wait'>FULL: Settling (" + formatTime(data.tankTimeLeft) + ")</span>";
           } else {
             tankDiv.innerText = "Distance: " + data.tankCm + " cm";
           }
        }

        let sumpDiv = document.getElementById('sumpStatus');
        let hasWater = (data.sumpHasWater === 1);

        if (data.flowLocked === 1) {
          sumpDiv.innerHTML = "<span class='status-bad'>!! ERROR: PUMP DRY RUN DETECTED !!</span>";
        } else if (data.sumpLocked === 1) {
          if (hasWater) {
            sumpDiv.innerHTML = "<span class='status-wait'>WATER AVAIL (RECOVERING: " + formatTime(data.sumpTimeLeft) + ")</span>";
          } else {
            sumpDiv.innerHTML = "<span class='status-bad'>EMPTY (RECOVERING: " + formatTime(data.sumpTimeLeft) + ")</span>";
          }
        } else {
          if (hasWater) {
            sumpDiv.innerHTML = "<span class='status-good'>WATER AVAILABLE</span>";
          } else {
            sumpDiv.innerHTML = "<span class='status-bad'>SUMP EMPTY</span>";
          }
        }

        let badge = document.getElementById('pumpStateBadge');
        let stopwatch = document.getElementById('stopwatchDisplay');
        if (data.relay === 1) {
          badge.innerText = ">> PUMP IS RUNNING <<";
          badge.style.color = "#4CAF50"; 
          badge.style.marginBottom = "5px";
          
          let m = Math.floor(data.motorRunTime / 60);
          let s = data.motorRunTime % 60;
          stopwatch.innerText = "Runtime: " + m + "m " + (s < 10 ? "0" : "") + s + "s";
          stopwatch.style.display = "block";
        } else {
          badge.innerText = "-- PUMP ON STANDBY --";
          badge.style.color = "#888"; 
          badge.style.marginBottom = "20px";
          stopwatch.style.display = "none";
        }
        
        let overrideBtn = document.getElementById('overrideBtn');
        let godModePanel = document.getElementById('godModePanel');
        let resetBtn = document.getElementById('resetBtn');
        let pumpToggle = document.getElementById('pumpToggle');

        if (frontendOverride) {
          overrideBtn.style.display = "none";
          godModePanel.style.display = "block";
          resetBtn.style.display = "block";
          
          if (!isCooldownActive) {
            pumpToggle.checked = (data.relay === 1);
            document.getElementById('lblOn').style.color = data.relay === 1 ? "#f44336" : "#555";
            document.getElementById('lblOff').style.color = data.relay === 1 ? "#555" : "#ccc";
          }
        } else {
          overrideBtn.style.display = "block";
          godModePanel.style.display = "none";
          resetBtn.style.display = "none";
        }

        document.getElementById('topUpToggle').checked = (data.topUpActive === 1);
        document.getElementById('usageVal').innerText = data.todayUsage;
        document.getElementById('yesterdayVal').innerText = data.yesterdayUsage;
        document.getElementById('networkName').innerText = data.network;
      });
    }

    function enterGodMode() { fetch('/setManual').then(() => updateUI()); }
    
    function handleTopUpClick(event) { fetch('/toggleTopUp').then(() => updateUI()); }

    function handleToggleClick(event) {
      if (isCooldownActive) { event.preventDefault(); return; }
      startCooldown(5);
      fetch('/toggle').then(() => updateUI());
    }

    function startCooldown(seconds) {
      isCooldownActive = true;
      let toggleInput = document.getElementById('pumpToggle');
      let resetBtn = document.getElementById('resetBtn');
      
      toggleInput.disabled = true; 
      resetBtn.disabled = true;    
      
      let secondsLeft = seconds;
      let cdText = document.getElementById('cooldownText');
      cdText.innerText = "HARDWARE SAFETY LOCK... " + secondsLeft + "s";
      
      let cdInterval = setInterval(() => {
        secondsLeft--;
        if (secondsLeft > 0) {
          cdText.innerText = "HARDWARE SAFETY LOCK... " + secondsLeft + "s";
        } else {
          clearInterval(cdInterval);
          isCooldownActive = false;
          toggleInput.disabled = false;
          resetBtn.disabled = false;
          cdText.innerText = "";
          updateUI(); 
        }
      }, 1000);
    }

    function resetAuto() { 
      if(isCooldownActive) return; 
      fetch('/resetAuto').then(() => updateUI()); 
    }

    setInterval(updateUI, 2500);
    updateUI();
  </script>
</body>
</html>
)=====";

#endif