#include "web_server_portal.h"
#include "app_config.h"
#include <WebServer.h>
#include <WiFi.h>

static WebServer server(80);

// Embedded Single-Page Jarvis Configuration Portal
static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>J.A.R.V.I.S. - Core Configuration</title>
    <link href="https://fonts.googleapis.com/css2?family=Orbitron:wght@400;700;900&family=Share+Tech+Mono&family=Rajdhani:wght@500;700&display=swap" rel="stylesheet">
    <style>
        :root {
            --theme-color: #00d2ff;
            --theme-color-glow: rgba(0, 210, 255, 0.3);
            --bg-grad-start: #0a0f1d;
            --bg-grad-end: #03060c;
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }

        body {
            font-family: 'Rajdhani', sans-serif;
            background: radial-gradient(circle at center, var(--bg-grad-start), var(--bg-grad-end));
            background-color: var(--bg-grad-end);
            color: #d1f4ff;
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
            overflow-x: hidden;
            letter-spacing: 0.5px;
        }

        /* Ambient scanlines */
        body::before {
            content: " ";
            display: block;
            position: fixed;
            top: 0; left: 0; bottom: 0; right: 0;
            background: linear-gradient(rgba(18, 16, 16, 0) 50%, rgba(0, 0, 0, 0.25) 50%), linear-gradient(90deg, rgba(255, 0, 0, 0.06), rgba(0, 255, 0, 0.02), rgba(0, 0, 255, 0.06));
            z-index: 100;
            background-size: 100% 4px, 6px 100%;
            pointer-events: none;
        }

        .portal-container {
            width: 100%;
            max-width: 850px;
            background: rgba(10, 20, 40, 0.35);
            backdrop-filter: blur(15px);
            -webkit-backdrop-filter: blur(15px);
            border: 1px solid rgba(0, 210, 255, 0.15);
            box-shadow: 0 0 40px rgba(0, 0, 0, 0.8), 0 0 20px var(--theme-color-glow);
            border-radius: 12px;
            padding: 40px;
            position: relative;
            z-index: 10;
        }

        /* Futuristic target corners */
        .portal-container::before, .portal-container::after {
            content: '';
            position: absolute;
            width: 15px;
            height: 15px;
            border-color: var(--theme-color);
            border-style: solid;
            pointer-events: none;
        }
        .portal-container::before {
            top: -1px; left: -1px;
            border-width: 2px 0 0 2px;
        }
        .portal-container::after {
            bottom: -1px; right: -1px;
            border-width: 0 2px 2px 0;
        }

        header {
            text-align: center;
            margin-bottom: 35px;
            position: relative;
        }

        h1 {
            font-family: 'Orbitron', sans-serif;
            font-size: 2.2rem;
            font-weight: 900;
            color: #fff;
            text-transform: uppercase;
            letter-spacing: 4px;
            text-shadow: 0 0 10px var(--theme-color), 0 0 20px var(--theme-color);
            margin-bottom: 8px;
        }

        .system-status {
            font-family: 'Share Tech Mono', monospace;
            font-size: 0.95rem;
            color: var(--theme-color);
            text-transform: uppercase;
            animation: pulse 2s infinite alternate;
        }

        @keyframes pulse {
            0% { opacity: 0.6; }
            100% { opacity: 1; text-shadow: 0 0 8px var(--theme-color); }
        }

        .grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 25px;
        }

        @media (max-width: 768px) {
            .grid {
                grid-template-columns: 1fr;
            }
            .portal-container {
                padding: 25px;
            }
        }

        .section-card {
            background: rgba(2, 6, 12, 0.6);
            border: 1px solid rgba(0, 210, 255, 0.08);
            border-radius: 8px;
            padding: 25px;
            transition: all 0.3s ease;
        }

        .section-card:hover {
            border-color: rgba(0, 210, 255, 0.25);
            box-shadow: 0 0 15px rgba(0, 210, 255, 0.05);
        }

        .section-card h2 {
            font-family: 'Orbitron', sans-serif;
            font-size: 1.1rem;
            text-transform: uppercase;
            letter-spacing: 1.5px;
            color: #fff;
            margin-bottom: 20px;
            border-bottom: 1px solid rgba(0, 210, 255, 0.15);
            padding-bottom: 8px;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }

        .form-group {
            margin-bottom: 18px;
        }

        label {
            display: block;
            font-size: 0.85rem;
            text-transform: uppercase;
            color: var(--theme-color);
            margin-bottom: 6px;
            font-weight: 700;
            letter-spacing: 1px;
        }

        input[type="text"], input[type="password"], input[type="number"], select {
            width: 100%;
            background: rgba(0, 20, 40, 0.4);
            border: 1px solid rgba(0, 210, 255, 0.2);
            border-radius: 4px;
            padding: 10px 12px;
            color: #fff;
            font-family: 'Share Tech Mono', monospace;
            font-size: 0.95rem;
            transition: all 0.3s ease;
        }

        input:focus, select:focus {
            outline: none;
            border-color: var(--theme-color);
            box-shadow: 0 0 10px rgba(0, 210, 255, 0.25);
            background: rgba(0, 20, 40, 0.6);
        }

        .wifi-select-container {
            display: flex;
            gap: 8px;
        }

        .btn-scan {
            background: transparent;
            border: 1px solid var(--theme-color);
            color: var(--theme-color);
            padding: 0 15px;
            border-radius: 4px;
            cursor: pointer;
            font-family: 'Orbitron', sans-serif;
            font-size: 0.8rem;
            transition: all 0.3s ease;
            text-transform: uppercase;
        }

        .btn-scan:hover {
            background: var(--theme-color);
            color: #03060c;
            box-shadow: 0 0 10px var(--theme-color);
        }

        /* Color customizer styles */
        .color-selector {
            display: flex;
            flex-wrap: wrap;
            gap: 10px;
            margin-bottom: 15px;
        }

        .color-swatch {
            width: 32px;
            height: 32px;
            border-radius: 50%;
            cursor: pointer;
            border: 2px solid transparent;
            transition: transform 0.2s ease, border-color 0.2s ease;
        }

        .color-swatch:hover {
            transform: scale(1.15);
        }

        .color-swatch.active {
            border-color: #fff;
            box-shadow: 0 0 10px #fff;
        }

        .custom-color-picker {
            display: flex;
            align-items: center;
            gap: 10px;
            margin-top: 12px;
        }

        .custom-color-picker input[type="color"] {
            border: none;
            background: transparent;
            width: 36px;
            height: 36px;
            cursor: pointer;
        }

        /* Action buttons */
        .actions {
            margin-top: 30px;
            text-align: center;
            display: flex;
            justify-content: center;
            gap: 20px;
        }

        .btn-submit {
            background: transparent;
            border: 2px solid var(--theme-color);
            color: #fff;
            padding: 12px 35px;
            font-family: 'Orbitron', sans-serif;
            font-size: 1rem;
            font-weight: 700;
            text-transform: uppercase;
            letter-spacing: 2px;
            cursor: pointer;
            border-radius: 6px;
            transition: all 0.3s ease;
            box-shadow: 0 0 15px var(--theme-color-glow);
        }

        .btn-submit:hover {
            background: var(--theme-color);
            color: #000;
            box-shadow: 0 0 25px var(--theme-color), 0 0 10px var(--theme-color);
            transform: translateY(-2px);
        }

        .btn-reset {
            background: transparent;
            border: 1px solid rgba(255, 60, 60, 0.4);
            color: rgba(255, 120, 120, 0.8);
            padding: 12px 25px;
            font-family: 'Orbitron', sans-serif;
            font-size: 0.9rem;
            text-transform: uppercase;
            cursor: pointer;
            border-radius: 6px;
            transition: all 0.3s ease;
        }

        .btn-reset:hover {
            background: rgba(255, 60, 60, 0.15);
            border-color: rgb(255, 60, 60);
            color: #fff;
            box-shadow: 0 0 15px rgba(255, 60, 60, 0.3);
        }

        /* Toast overlay */
        .toast {
            position: fixed;
            bottom: 30px;
            left: 50%;
            transform: translateX(-50%) translateY(100px);
            background: rgba(3, 6, 12, 0.95);
            border: 1px solid var(--theme-color);
            box-shadow: 0 0 20px var(--theme-color-glow);
            padding: 15px 30px;
            border-radius: 6px;
            color: #fff;
            font-family: 'Share Tech Mono', monospace;
            z-index: 1000;
            transition: all 0.4s cubic-bezier(0.175, 0.885, 0.32, 1.275);
            opacity: 0;
            text-transform: uppercase;
            text-align: center;
        }

        .toast.show {
            transform: translateX(-50%) translateY(0);
            opacity: 1;
        }
    </style>
</head>
<body>

    <div class="portal-container">
        <header>
            <h1>J.A.R.V.I.S. CONFIG</h1>
            <div class="system-status">System Core Initialization :: Active</div>
        </header>

        <form id="configForm">
            <div class="grid">
                
                <!-- Wi-Fi Settings -->
                <div class="section-card">
                    <h2>Wireless Network</h2>
                    <div class="form-group">
                        <label for="wifi_ssid">Access Point SSID</label>
                        <div class="wifi-select-container">
                            <select id="wifi_select" style="display:none;"></select>
                            <input type="text" id="wifi_ssid" name="wifi_ssid" required placeholder="Enter SSID manually or scan">
                            <button type="button" class="btn-scan" id="btnScan">Scan</button>
                        </div>
                    </div>
                    <div class="form-group">
                        <label for="wifi_pass">Network Key (Password)</label>
                        <input type="password" id="wifi_pass" name="wifi_pass" placeholder="••••••••••••">
                    </div>
                </div>

                <!-- Home Assistant -->
                <div class="section-card">
                    <h2>Home Assistant</h2>
                    <div class="form-group">
                        <label for="ha_host">Server Host IP / Address</label>
                        <input type="text" id="ha_host" name="ha_host" required placeholder="192.168.1.100">
                    </div>
                    <div class="form-group">
                        <label for="ha_port">Port</label>
                        <input type="number" id="ha_port" name="ha_port" required placeholder="8123">
                    </div>
                </div>

                <!-- Token & Pipeline -->
                <div class="section-card" style="grid-column: span 1;">
                    <h2>Authentication & Pipeline</h2>
                    <div class="form-group">
                        <label for="ha_token">Long-Lived Access Token</label>
                        <input type="password" id="ha_token" name="ha_token" placeholder="Bearer Token from HA Profile">
                    </div>
                    <div class="form-group">
                        <label for="ha_pipe">Assist Pipeline ID (Optional)</label>
                        <input type="text" id="ha_pipe" name="ha_pipe" placeholder="Leave blank for Default Pipeline">
                    </div>
                </div>

                <!-- Personalization (Wake Word & Color) -->
                <div class="section-card" style="grid-column: span 1;">
                    <h2>System Personalization</h2>
                    <div class="form-group">
                        <label for="wake_word">Wake Word</label>
                        <select id="wake_word" name="wake_word">
                            <option value="Hey Jarvis">Hey Jarvis</option>
                            <option value="Okay Nabu">Okay Nabu</option>
                            <option value="Hey Mycroft">Hey Mycroft</option>
                        </select>
                    </div>
                    <div class="form-group">
                        <label>Interface Theme Color</label>
                        <div class="color-selector" id="swatchList">
                            <!-- Swatches rendered by JS -->
                        </div>
                        <div class="custom-color-picker">
                            <input type="color" id="customColor">
                            <span style="font-size:0.85rem; text-transform:uppercase;">Custom HEX Code</span>
                        </div>
                    </div>
                </div>

            </div>

            <!-- Theme Color Hex hidden input to send to backend -->
            <input type="hidden" id="theme_color" name="theme_color" value="#00d2ff">

            <div class="actions">
                <button type="button" class="btn-reset" id="btnReset">Factory Reset</button>
                <button type="submit" class="btn-submit">Initialize Core</button>
            </div>
        </form>
    </div>

    <div class="toast" id="toast">Settings Saved</div>

    <script>
        const DEFAULT_CONFIG = {
            wifi_ssid: "",
            ha_host: "",
            ha_port: 8123,
            ha_pipe: "",
            wake_word: "Hey Jarvis",
            theme_color: "#00d2ff"
        };

        const THEME_PRESETS = [
            { name: "Cyan", hex: "#00d2ff" },
            { name: "Iron Man", hex: "#ff3e3e" },
            { name: "Matrix", hex: "#3eff3e" },
            { name: "Royal Purple", hex: "#d83eff" },
            { name: "Gold", hex: "#ffbf00" },
            { name: "Hot Orange", hex: "#ff6c00" }
        ];

        // Apply theme color dynamically to document element
        function applyThemeColor(hex) {
            document.documentElement.style.setProperty('--theme-color', hex);
            // Calculate low transparency glow
            const r = parseInt(hex.slice(1, 3), 16);
            const g = parseInt(hex.slice(3, 5), 16);
            const b = parseInt(hex.slice(5, 7), 16);
            document.documentElement.style.setProperty('--theme-color-glow', `rgba(${r}, ${g}, ${b}, 0.35)`);
            document.getElementById('theme_color').value = hex;
        }

        // Render color swatches
        const swatchList = document.getElementById('swatchList');
        THEME_PRESETS.forEach(theme => {
            const swatch = document.createElement('div');
            swatch.className = 'color-swatch';
            swatch.style.backgroundColor = theme.hex;
            swatch.title = theme.name;
            swatch.addEventListener('click', () => {
                document.querySelectorAll('.color-swatch').forEach(s => s.classList.remove('active'));
                swatch.classList.add('active');
                applyThemeColor(theme.hex);
                document.getElementById('customColor').value = theme.hex;
            });
            swatchList.appendChild(swatch);
        });

        // Listen for custom color picker
        document.getElementById('customColor').addEventListener('input', (e) => {
            document.querySelectorAll('.color-swatch').forEach(s => s.classList.remove('active'));
            applyThemeColor(e.target.value);
        });

        // Show toaster notification
        function showToast(message, isError = false) {
            const toast = document.getElementById('toast');
            toast.textContent = message;
            toast.style.borderColor = isError ? '#ff3e3e' : 'var(--theme-color)';
            toast.style.boxShadow = isError ? '0 0 20px rgba(255, 62, 62, 0.4)' : '0 0 20px var(--theme-color-glow)';
            toast.classList.add('show');
            setTimeout(() => {
                toast.classList.remove('show');
            }, 5000);
        }

        // Load configuration from server
        async function fetchConfig() {
            try {
                const response = await fetch('/api/config');
                if (response.ok) {
                    const data = await response.json();
                    document.getElementById('wifi_ssid').value = data.wifi_ssid || "";
                    document.getElementById('ha_host').value = data.ha_host || "";
                    document.getElementById('ha_port').value = data.ha_port || 8123;
                    document.getElementById('ha_pipe').value = data.ha_pipeline_id || "";
                    document.getElementById('wake_word').value = data.wake_word || "Hey Jarvis";
                    
                    const savedColor = data.theme_color || "#00d2ff";
                    applyThemeColor(savedColor);
                    document.getElementById('customColor').value = savedColor;
                    
                    // Mark swatch active if it matches
                    const matchingSwatch = Array.from(document.querySelectorAll('.color-swatch'))
                        .find(s => s.style.backgroundColor === savedColor || s.title.toLowerCase() === savedColor.toLowerCase());
                    if (matchingSwatch) matchingSwatch.classList.add('active');
                }
            } catch (err) {
                console.error("Failed to fetch config", err);
            }
        }

        // Wi-Fi Scanning
        document.getElementById('btnScan').addEventListener('click', async () => {
            const scanBtn = document.getElementById('btnScan');
            const select = document.getElementById('wifi_select');
            const ssidInput = document.getElementById('wifi_ssid');
            
            scanBtn.disabled = true;
            scanBtn.textContent = "Scanning...";
            
            try {
                const res = await fetch('/api/wifi_scan');
                if (res.ok) {
                    const networks = await res.json();
                    select.innerHTML = '<option value="">-- Select Network --</option>';
                    networks.forEach(net => {
                        const opt = document.createElement('option');
                        opt.value = net.ssid;
                        opt.textContent = `${net.ssid} (${net.rssi} dBm)`;
                        select.appendChild(opt);
                    });
                    
                    // Show select dropdown
                    select.style.display = 'block';
                    ssidInput.style.display = 'none';
                    ssidInput.removeAttribute('required');
                    
                    select.onchange = (e) => {
                        if (e.target.value) {
                            ssidInput.value = e.target.value;
                            select.style.display = 'none';
                            ssidInput.style.display = 'block';
                            ssidInput.setAttribute('required', 'true');
                        }
                    };
                }
            } catch (err) {
                showToast("Wi-Fi Scan Failed", true);
            } finally {
                scanBtn.disabled = false;
                scanBtn.textContent = "Scan";
            }
        });

        // Submit form
        document.getElementById('configForm').addEventListener('submit', async (e) => {
            e.preventDefault();
            const formData = new FormData(e.target);
            const params = new URLSearchParams(formData);

            try {
                const res = await fetch('/api/save', {
                    method: 'POST',
                    body: params
                });

                if (res.ok) {
                    const result = await res.json();
                    showToast(result.message || "Settings Saved. Rebooting...");
                } else {
                    showToast("Failed to save configuration", true);
                }
            } catch (err) {
                showToast("Error sending configuration", true);
            }
        });

        // Factory reset
        document.getElementById('btnReset').addEventListener('click', async () => {
            if (confirm("Restore factory defaults? This clears Wi-Fi credentials.")) {
                try {
                    const res = await fetch('/api/reset', { method: 'POST' });
                    if (res.ok) {
                        showToast("System Reset! Rebooting...");
                        setTimeout(() => window.location.reload(), 2000);
                    }
                } catch (err) {
                    showToast("Reset request failed", true);
                }
            }
        });

        // Initial fetch
        fetchConfig();
    </script>
</body>
</html>
)rawliteral";

// Handler for GET /
static void handle_root() {
    server.send(200, "text/html", INDEX_HTML);
}

// Handler for GET /api/config
static void handle_get_config() {
    const AppConfig& cfg = app_config_get();
    
    // Construct JSON manually to avoid heap fragmentation issues
    String json;
    json.reserve(512);
    json += "{";
    json += "\"wifi_ssid\":\"" + String(cfg.wifi_ssid) + "\",";
    json += "\"ha_host\":\"" + String(cfg.ha_host) + "\",";
    json += "\"ha_port\":" + String(cfg.ha_port) + ",";
    json += "\"ha_pipeline_id\":\"" + String(cfg.ha_pipeline_id) + "\",";
    json += "\"wake_word\":\"" + String(cfg.wake_word) + "\",";
    json += "\"theme_color\":\"" + String(cfg.theme_color) + "\"";
    json += "}";
    
    server.send(200, "application/json", json);
}

// Handler for POST /api/save
static void handle_post_save() {
    if (!server.hasArg("wifi_ssid")) {
        server.send(400, "application/json", "{\"error\":\"Missing fields\"}");
        return;
    }

    AppConfig cfg;
    memset(&cfg, 0, sizeof(AppConfig));

    strncpy(cfg.wifi_ssid, server.arg("wifi_ssid").c_str(), sizeof(cfg.wifi_ssid) - 1);
    
    // Only update password if user entered a new one
    if (server.hasArg("wifi_pass") && server.arg("wifi_pass").length() > 0) {
        strncpy(cfg.wifi_password, server.arg("wifi_pass").c_str(), sizeof(cfg.wifi_password) - 1);
    } else {
        // Retain existing password
        strncpy(cfg.wifi_password, app_config_get().wifi_password, sizeof(cfg.wifi_password) - 1);
    }

    strncpy(cfg.ha_host, server.arg("ha_host").c_str(), sizeof(cfg.ha_host) - 1);
    cfg.ha_port = server.arg("ha_port").toInt();

    // Only update HA token if user entered a new one
    if (server.hasArg("ha_token") && server.arg("ha_token").length() > 0) {
        strncpy(cfg.ha_token, server.arg("ha_token").c_str(), sizeof(cfg.ha_token) - 1);
    } else {
        // Retain existing token
        strncpy(cfg.ha_token, app_config_get().ha_token, sizeof(cfg.ha_token) - 1);
    }

    strncpy(cfg.ha_pipeline_id, server.arg("ha_pipe").c_str(), sizeof(cfg.ha_pipeline_id) - 1);
    strncpy(cfg.wake_word, server.arg("wake_word").c_str(), sizeof(cfg.wake_word) - 1);
    strncpy(cfg.theme_color, server.arg("theme_color").c_str(), sizeof(cfg.theme_color) - 1);

    app_config_save(cfg);

    server.send(200, "application/json", "{\"status\":\"saved\",\"message\":\"Configuration Saved. Rebooting Jarvis OS...\"}");
    delay(1000);
    ESP.restart();
}

// Handler for POST /api/reset
static void handle_post_reset() {
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Performing factory reset...\"}");
    delay(1000);
    app_config_reset();
    ESP.restart();
}

// Handler for GET /api/wifi_scan
static void handle_get_wifi_scan() {
    Serial.println("WebPortal: Starting Wi-Fi network scan...");
    int n = WiFi.scanNetworks();
    
    String json = "[";
    for (int i = 0; i < n; ++i) {
        if (i > 0) json += ",";
        json += "{";
        json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
        json += "\"rssi\":" + String(WiFi.RSSI(i));
        json += "}";
    }
    json += "]";
    
    server.send(200, "application/json", json);
}

void web_server_init() {
    server.on("/", HTTP_GET, handle_root);
    server.on("/api/config", HTTP_GET, handle_get_config);
    server.on("/api/save", HTTP_POST, handle_post_save);
    server.on("/api/reset", HTTP_POST, handle_post_reset);
    server.on("/api/wifi_scan", HTTP_GET, handle_get_wifi_scan);
    
    server.begin();
    Serial.println("WebPortal: Configuration HTTP Server online on port 80.");
}

void web_server_handle() {
    server.handleClient();
}
