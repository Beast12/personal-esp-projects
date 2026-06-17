# Live Stocks Viewer (ESPHome + LVGL)

A live stock market dashboard on the ESP32-S3 touchscreen panel (`JC3248W535C_I`) utilizing ESPHome, LVGL, and free public Yahoo Finance data.

## Features

- **4-Stock Grid Dashboard:** Beautiful 2x2 layout displaying stock symbol, current price, change amount, and percentage change.
- **Web-Based Symbol Configuration:** Exposes a web dashboard on your local network (HTTP portal) where you can type in stock symbols (e.g. `AAPL`, `TSLA`, `MSFT`, `NVDA`) and submit them.
- **Real-Time Responsiveness:** Changing symbols on the web portal triggers an immediate API request to update the screen.
- **Persistent Symbols:** Symbol settings are persisted in ESP32 Flash memory and survive reboots.
- **Color-Coded Status and Borders:** Cards dynamically change their border and text color (green for gains, red for losses) based on price movements.
- **Local Time Display:** Display last updated time in the header using local SNTP time.

## Hardware Configuration (JC3248W535C_I)

- **SoC:** ESP32-S3 (with Octal PSRAM)
- **Framework:** `esp-idf`
- **Display Platform:** `qspi_dbi` (CUSTOM model, CS on GPIO45, Data on GPIO21/48/40/39, CLK on GPIO47)
- **Touch Platform:** `axs15231` (I2C on GPIO4/8)
- **Backlight Platform:** `ledc` PWM on GPIO1

## Setup and Quick Start

1. **Verify Secrets:**
   Ensure `secrets.yaml` is pointing to your repository `secrets.yml` (a symlink is already pre-configured). It should contain your `wifi_ssid`, `wifi_password`, and `api_encryption_key`.

2. **Validate Syntax:**
   Verify the configuration compiles and has valid syntax:
   ```bash
   esphome config stocks.yml
   ```

3. **Compile and Upload:**
   Connect the ESP32-S3 panel to your computer and run:
   ```bash
   esphome run stocks.yml
   ```

4. **Access Web Configuration:**
   Once flashed, the device will output its IP address to the logger logs (or check your router client list). Open your browser and navigate to:
   ```
   http://<device_ip>
   ```
   Under the **Text** section, enter the tickers you want to monitor (e.g. `TSLA`, `AMD`, `GOOG`, `BTC-USD`). The screen will automatically refresh with the new market data.
