# Smoker Pro User Manual

This manual explains how to access the smart smoker (Smoker Pro) from a smartphone, as well as how to manage temperature control and save history logs.

---

## 1. First-time setup (connecting to Wi-Fi)

When the device (ESP8266) starts in a new environment, or if the configured Wi-Fi network cannot be found, it will automatically enter setup mode.

1. Open the Wi-Fi settings on your smartphone.
2. Find the Wi-Fi network named SMOKER-SETUP and connect to it.
3. Once connected, your browser should open the initial setup screen automatically. If it does not, open your browser and go to http://192.168.4.1.
4. Follow the on-screen instructions to enter your home Wi-Fi SSID and password, along with the Meross smart plug information, then press Save.
5. The device will restart and connect to your home Wi-Fi network.

---

## 2. How to access the smoker

Use the following methods when the device is connected to your home Wi-Fi network.

### For iPhone (Safari) / Mac
Open the browser address bar and enter:
👉 http://smoker.local

### For Android / Windows
In some Android and Windows environments, .local addresses may not work. In that case, use your router’s settings page or a network scanning app such as Fing to find the ESP8266’s assigned IP address (for example, 192.168.x.x), then enter it directly in the browser.

---

## 3. Add it to your home screen (recommended)

To avoid typing the URL every time, it is recommended to add the dashboard to your smartphone home screen as a PWA.

1. Open the dashboard in your browser (for example, http://smoker.local).
2. For iPhone: tap the Share icon (a square with an arrow) and select Add to Home Screen.
3. For Android: open the browser menu and select Add to Home Screen.
4. A dedicated icon will be created on your home screen. You can then launch the smoker directly from there.

---

## 4. Basic operation

On the dashboard, you can perform the following actions.

- Temperature Control
  - Enter the target temperature and press START AUTO CONTROL to let the smart plug cycle ON/OFF automatically and keep the temperature stable.
- Manual Overrides
  - FORCE ON: keeps the heater ON continuously.
  - MONITOR: records temperature only and does not operate the heater.
  - STOP / IDLE: stops control and turns the heater OFF.
- PID Tuning (Advanced Settings)
  - Adjust the P, I, and D gains, as well as the PWM cycle, then press SAVE PID CONFIG.

---

## 5. Managing history data and notes (Data Logs)

The smoker automatically saves temperature history for about 30 hours. You can export this data to your phone or import previous logs for review.

### Export data (Download CSV)
After a smoke session, save the recorded data.
1. In the Session Info & Notes section, enter the date, food, location, and notes.
2. Tap DOWNLOAD CSV.
3. A CSV file containing the entered information and temperature data will be saved to your phone. Example file name: smoker_2024-04-26_Bacon.csv

### Import data (Upload CSV)
To review a previous smoke session:
1. Tap IMPORT CSV.
2. Select a previously saved CSV file from your phone.
3. The chart will be recreated and the session information and notes will be filled in automatically.

### Reset logs
Before starting a new smoke session, or if you want to start the graph over, press CLEAR LOG to reset the device’s stored data.
