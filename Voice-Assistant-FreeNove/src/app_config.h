#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <Arduino.h>

struct AppConfig {
    char wifi_ssid[64];
    char wifi_password[64];
    char ha_host[64];
    int ha_port;
    char ha_token[256];
    char ha_pipeline_id[64];
    char wake_word[64];
    char theme_color[16]; // e.g. "#00d2ff"
};

// Initializes configuration (loads from Preferences or falls back to config.h defaults)
void app_config_init();

// Retrieves the active configuration
const AppConfig& app_config_get();

// Saves the new configuration to NVS
void app_config_save(const AppConfig& new_cfg);

// Resets config in NVS to defaults
void app_config_reset();

#endif // APP_CONFIG_H
