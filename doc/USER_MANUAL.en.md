# Smoker Pro User Manual

This manual explains how to use the Smoker Pro system on the M5Stack Core2 to monitor temperature, control the heater, and review saved session history.

---

## 1. First-time setup

When the device starts in a new environment, it will enter setup mode automatically if the Wi-Fi configuration is not available yet.

1. Open the Wi-Fi settings on your smartphone.
2. Connect to the network named SMOKER-SETUP.
3. Open your browser and go to http://192.168.4.1 if the setup page does not appear automatically.
4. Enter your home Wi-Fi details and the Meross smart plug information, then press Save.
5. The device will restart and connect to your network.

---

## 2. Accessing the smoker

Once the device is connected to the network, you can open the built-in dashboard from a smartphone or browser.

### iPhone / Mac
Open the browser address bar and enter:
👉 http://smoker.local

### Android / Windows
If the .local address does not work, use your router settings or a network scanner to find the device IP address and enter it directly in the browser.

---

## 3. Add it to your home screen

To make the system easier to use, you can add the dashboard to your phone’s home screen as a web app.

1. Open the dashboard in your browser.
2. On iPhone, tap the Share button and choose Add to Home Screen.
3. On Android, use the browser menu and choose Add to Home Screen.
4. The app icon will appear on your home screen for quick access.

---

## 4. Basic operation

From the dashboard, you can:

- Start automatic temperature control
  - Enter the target temperature and press START AUTO CONTROL to keep the smoker stable.
- Use manual overrides
  - FORCE ON keeps the heater on continuously.
  - MONITOR records temperature without operating the heater.
  - STOP / IDLE turns the heater off.
- Adjust PID settings
  - Change the P, I, and D values and the PWM cycle, then save the configuration.

---

## 5. Managing history data and notes

The system saves recent temperature history and allows you to export or restore session data.

### Export data
After a smoke session, save the recorded data.
1. Enter the date, food, location, and notes in the Session Info & Notes section.
2. Tap DOWNLOAD CSV.
3. A CSV file containing the temperature trend and session information will be saved to your device.

### Import data
To review a previous session:
1. Tap IMPORT CSV.
2. Select a previously saved CSV file.
3. The chart and session information will be restored automatically.

### Reset logs
Before starting a new session, press CLEAR LOG to reset the stored data.
