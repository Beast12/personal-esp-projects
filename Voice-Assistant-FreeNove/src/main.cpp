#include <Arduino.h>
#include <WiFi.h>
#include "display.h"
#include "ui.h"

Display screen;

// Wi-Fi settings
const char* ssid = "Skynet";
const char* password = "TheLoveHouse";

// Connection tracking
bool wifi_connected = false;
unsigned long wifi_check_timer = 0;

// Simulation tracking
AssistantState last_checked_state = STATE_IDLE;
unsigned long state_transition_timer = 0;

extern "C" void ui_update_wifi_status(bool connected, const char * ip_or_ssid);

void setup()
{
    // Initialize USB CDC serial
    Serial.begin(115200);
    delay(2000);
    Serial.println("JARVIS OS Core booting...");

    // Initialize display and touch controllers
    screen.init();
    Serial.println("Display hardware online.");

    // Initialize UI Layout and styles
    ui_init();
    ui_set_status_text("SYSTEM OFFLINE");
    ui_set_transcript_text("Welcome back, sir. Booting local network adapters...");
    Serial.println("UI layer online.");

    // Connect to Wi-Fi asynchronously (non-blocking)
    Serial.printf("Connecting to Wi-Fi SSID: %s\n", ssid);
    WiFi.begin(ssid, password);
    ui_set_status_text("WIFI SEARCHING...");
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
                IPAddress ip = WiFi.localIP();
                char ip_str[24];
                snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
                
                // Update Wi-Fi UI elements
                ui_update_wifi_status(true, ip_str);
                ui_set_status_text("SYSTEM ONLINE");
                ui_set_transcript_text("J.A.R.V.I.S. is ready.\nWi-Fi connection established.");
                
                Serial.printf("Network connected. IP: %s\n", ip_str);
            }
        } else {
            if (wifi_connected || current_time < 5000) {
                wifi_connected = false;
                ui_update_wifi_status(false, "");
                ui_set_status_text("WIFI DISCONNECTED");
                Serial.println("Network disconnected / unavailable.");
            }
        }
    }

    // 2. Animated Interaction State Machine
    AssistantState active_state = ui_get_state();
    
    // Check if state changed (e.g. from user pressing the TALK button on screen)
    if (active_state != last_checked_state) {
        last_checked_state = active_state;
        state_transition_timer = current_time;
        Serial.printf("State Transition Detected: %d\n", (int)active_state);
    }

    // Run simulated response workflow
    if (active_state == STATE_LISTENING) {
        // Wait 4 seconds in Listening state, then simulate voice input termination
        if (current_time - state_transition_timer > 4000) {
            Serial.println("Simulating: Voice input received. Processing...");
            ui_set_state(STATE_THINKING);
            ui_set_status_text("PROCESSING...");
            ui_set_transcript_text("User: \"Are all systems functioning, Jarvis?\"");
        }
    } 
    else if (active_state == STATE_THINKING) {
        // Wait 3 seconds in Thinking state, then simulate generating response
        if (current_time - state_transition_timer > 3000) {
            Serial.println("Simulating: Response generated. Speaking...");
            ui_set_state(STATE_SPEAKING);
            ui_set_status_text("SPEAKING...");
            ui_set_transcript_text("J.A.R.V.I.S.: \"Indeed they are, sir. I have verified all local network parameters and dashboard drivers. All systems are green.\"");
        }
    } 
    else if (active_state == STATE_SPEAKING) {
        // Wait 6 seconds in Speaking state, then return to Idle
        if (current_time - state_transition_timer > 6000) {
            Serial.println("Simulating: Speaking completed. Returning to Idle...");
            ui_set_state(STATE_IDLE);
            ui_set_status_text("SYSTEM ONLINE");
            ui_set_transcript_text("Ready. Standing by for next command, sir.");
        }
    }

    delay(5);
}
