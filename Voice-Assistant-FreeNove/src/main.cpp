#include <Arduino.h>
#include <WiFi.h>
#include "display.h"
#include "ui.h"
#include "app_config.h"
#include "web_server_portal.h"
#include "voice_assistant.h"

Display screen;

// Connection tracking
bool wifi_connected = false;
bool ap_mode = false;
bool web_server_started = false;
unsigned long wifi_check_timer = 0;
unsigned long wifi_start_time = 0;

// Interaction state tracking
AssistantState last_checked_state = STATE_IDLE;

extern "C" void ui_update_wifi_status(bool connected, const char * ip_or_ssid);

void setup()
{
    // Initialize USB CDC serial
    Serial.begin(115200);
    delay(2000);
    Serial.println("JARVIS OS Core booting...");

    // Initialize dynamic configuration from NVS/Preferences
    app_config_init();

    // Initialize display and touch controllers
    screen.init();
    Serial.println("Display hardware online.");

    // Initialize UI Layout and styles
    ui_init();
    ui_set_status_text("SYSTEM OFFLINE");
    ui_set_transcript_text("Welcome back, sir. Booting local network adapters...");
    Serial.println("UI layer online.");

    // Connect to Wi-Fi asynchronously (non-blocking)
    Serial.printf("WiFi: Connecting to SSID: %s\n", app_config_get().wifi_ssid);
    WiFi.begin(app_config_get().wifi_ssid, app_config_get().wifi_password);
    wifi_start_time = millis();
    ui_set_status_text("WIFI SEARCHING...");

    // Initialize voice assistant and audio codec
    VoiceAssistant::init();
}

void loop()
{
    // Give time to LVGL to perform rendering and process touch inputs
    screen.routine();

    unsigned long current_time = millis();

    // 1. Asynchronous Wi-Fi Connection Manager
    if (current_time - wifi_check_timer > 1000) {
        wifi_check_timer = current_time;
        
        if (WiFi.status() == WL_CONNECTED) {
            if (!wifi_connected) {
                wifi_connected = true;
                ap_mode = false;
                IPAddress ip = WiFi.localIP();
                char ip_str[24];
                snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
                
                // Update Wi-Fi UI elements
                ui_update_wifi_status(true, ip_str);
                ui_set_status_text("SYSTEM ONLINE");
                ui_set_transcript_text("J.A.R.V.I.S. is ready.\nWi-Fi connection established.");
                
                Serial.printf("WiFi: Connected. IP: %s\n", ip_str);
                
                if (!web_server_started) {
                    web_server_init();
                    web_server_started = true;
                }
            }
        } else if (!ap_mode) {
            // Fall back to AP mode if we fail to connect within 15 seconds
            if (current_time - wifi_start_time > 15000) {
                Serial.println("WiFi: Connection timeout. Starting fallback Access Point...");
                WiFi.disconnect();
                WiFi.softAP("Jarvis-Satellite-AP");
                ap_mode = true;
                wifi_connected = false;
                
                ui_update_wifi_status(false, "192.168.4.1");
                ui_set_status_text("PORTAL ACTIVE");
                ui_set_transcript_text("SSID: Jarvis-Satellite-AP\nConnect and go to:\nhttp://192.168.4.1");
                
                if (!web_server_started) {
                    web_server_init();
                    web_server_started = true;
                }
            } else {
                if (wifi_connected || current_time < 5000) {
                    wifi_connected = false;
                    ui_update_wifi_status(false, "");
                    ui_set_status_text("WIFI DISCONNECTED");
                    Serial.println("WiFi: Disconnected / unavailable.");
                }
            }
        }
    }

    // 2. Interaction State Machine Hooked to VoiceAssistant
    if (wifi_connected && !ap_mode) {
        AssistantState active_state = ui_get_state();
        
        // Check if state changed (e.g. from user pressing the TALK button on screen)
        if (active_state != last_checked_state) {
            AssistantState old_state = last_checked_state;
            last_checked_state = active_state;
            Serial.printf("State Transition Detected: %d -> %d\n", (int)old_state, (int)active_state);
            
            if (active_state == STATE_LISTENING) {
                VoiceAssistant::start_listening(false);
            } else if (active_state == STATE_IDLE && old_state == STATE_LISTENING) {
                VoiceAssistant::stop_listening();
            }
        }

        // Poll Voice Assistant events and I2S audio
        VoiceAssistant::loop();
    }

    // 3. Poll web server clients
    if (web_server_started) {
        web_server_handle();
    }

    delay(5);
}
