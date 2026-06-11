#include "app_config.h"
#include "config.h"
#include <Preferences.h>

static AppConfig active_config;
static Preferences prefs;

void app_config_init() {
    prefs.begin("app-config", false);

    // Read wifi_ssid; if not present, assume fresh boot and load defaults from config.h
    String ssid = prefs.getString("wifi_ssid", "");
    if (ssid.length() == 0) {
        Serial.println("AppConfig: No existing config found. Loading config.h defaults...");
        
        strncpy(active_config.wifi_ssid, WIFI_SSID, sizeof(active_config.wifi_ssid) - 1);
        strncpy(active_config.wifi_password, WIFI_PASSWORD, sizeof(active_config.wifi_password) - 1);
        strncpy(active_config.ha_host, HA_HOST, sizeof(active_config.ha_host) - 1);
        active_config.ha_port = HA_PORT;
        strncpy(active_config.ha_token, HA_TOKEN, sizeof(active_config.ha_token) - 1);
        strncpy(active_config.ha_pipeline_id, HA_PIPELINE_ID, sizeof(active_config.ha_pipeline_id) - 1);
        strncpy(active_config.wake_word, "Hey Jarvis", sizeof(active_config.wake_word) - 1);
        strncpy(active_config.theme_color, "#00d2ff", sizeof(active_config.theme_color) - 1);
        
        // Save these defaults to NVS immediately so they are persistent
        app_config_save(active_config);
    } else {
        Serial.println("AppConfig: Loading configuration from NVS...");
        
        strncpy(active_config.wifi_ssid, ssid.c_str(), sizeof(active_config.wifi_ssid) - 1);
        
        String pass = prefs.getString("wifi_pass", "");
        strncpy(active_config.wifi_password, pass.c_str(), sizeof(active_config.wifi_password) - 1);
        
        String host = prefs.getString("ha_host", "");
        strncpy(active_config.ha_host, host.c_str(), sizeof(active_config.ha_host) - 1);
        
        active_config.ha_port = prefs.getInt("ha_port", 8123);
        
        String token = prefs.getString("ha_token", "");
        strncpy(active_config.ha_token, token.c_str(), sizeof(active_config.ha_token) - 1);
        
        String pipeline = prefs.getString("ha_pipe", "");
        strncpy(active_config.ha_pipeline_id, pipeline.c_str(), sizeof(active_config.ha_pipeline_id) - 1);
        
        String wake = prefs.getString("wake_word", "Hey Jarvis");
        strncpy(active_config.wake_word, wake.c_str(), sizeof(active_config.wake_word) - 1);
        
        String theme = prefs.getString("theme_color", "#00d2ff");
        strncpy(active_config.theme_color, theme.c_str(), sizeof(active_config.theme_color) - 1);
    }

    prefs.end();

    Serial.println("------ Current System Configuration ------");
    Serial.printf("WiFi SSID: %s\n", active_config.wifi_ssid);
    Serial.printf("HA Host: %s:%d\n", active_config.ha_host, active_config.ha_port);
    Serial.printf("HA Pipeline: %s\n", active_config.ha_pipeline_id);
    Serial.printf("Wake Word: %s\n", active_config.wake_word);
    Serial.printf("Theme Color: %s\n", active_config.theme_color);
    Serial.println("------------------------------------------");
}

const AppConfig& app_config_get() {
    return active_config;
}

void app_config_save(const AppConfig& new_cfg) {
    prefs.begin("app-config", false);
    
    prefs.putString("wifi_ssid", new_cfg.wifi_ssid);
    prefs.putString("wifi_pass", new_cfg.wifi_password);
    prefs.putString("ha_host", new_cfg.ha_host);
    prefs.putInt("ha_port", new_cfg.ha_port);
    prefs.putString("ha_token", new_cfg.ha_token);
    prefs.putString("ha_pipe", new_cfg.ha_pipeline_id);
    prefs.putString("wake_word", new_cfg.wake_word);
    prefs.putString("theme_color", new_cfg.theme_color);
    
    prefs.end();
    
    // Update active cache
    active_config = new_cfg;
    Serial.println("AppConfig: Configuration updated and saved to NVS.");
}

void app_config_reset() {
    prefs.begin("app-config", false);
    prefs.clear();
    prefs.end();
    Serial.println("AppConfig: NVS cleared. Restoring defaults...");
    app_config_init();
}
