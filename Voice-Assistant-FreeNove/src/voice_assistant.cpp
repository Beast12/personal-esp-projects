#include "voice_assistant.h"
#include <WiFi.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include "ESP_I2S.h"
#include "es8311.h"
#include "Audio.h"
#include "config.h"
#include "ui.h"

// Hardware pin definitions for Freenove 3.5" ST77922 board
#define I2S_MCK 17
#define I2S_BCK 18
#define I2S_DINT 16
#define I2S_DOUT 15
#define I2S_WS 21
#define AP_ENABLE 1

#define I2C_SDA 38
#define I2C_SCL 39
#define I2C_SPEED 400000

// Size of PCM audio chunk to send to HA
#define AUDIO_CHUNK_SIZE 1024

// Instances and state variables
static WebSocketsClient webSocket;
static Audio audio_player;
static I2SClass mic_i2s;
static es8311_handle_t es_handle = NULL;

static bool ws_connected = false;
static bool ws_authenticated = false;
static bool is_recording = false;
static bool player_running = false;

static int msg_id = 1;
static int stt_handler_id = -1;
static String tts_url = "";

static int current_volume = -1;
static bool last_mute_state = false;

// Forward declarations
static void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
static void start_recording();
static void stop_recording();
static void send_pipeline_run();

// Weakly-linked audio callbacks from ESP32-audioI2S
void audio_eof_mp3(const char *info) {
    Serial.printf("Audio Callback: EOF MP3 - %s\n", info);
    VoiceAssistant::on_playback_finished();
}

void audio_eof_stream(const char *info) {
    Serial.printf("Audio Callback: EOF Stream - %s\n", info);
    VoiceAssistant::on_playback_finished();
}

void audio_eof_speech(const char *info) {
    Serial.printf("Audio Callback: EOF Speech - %s\n", info);
    VoiceAssistant::on_playback_finished();
}

void VoiceAssistant::init() {
    Serial.println("VoiceAssistant: Initializing audio subsystems...");

    // 1. Initialize Amplifier Power pin
    pinMode(AP_ENABLE, OUTPUT);
    digitalWrite(AP_ENABLE, LOW); // Mute initially to prevent speaker pops

    // 2. Initialize ES8311 Codec via I2C (shares the same bus with the touchpad)
    es_handle = es8311_create(I2C_NUM_0, ES8311_ADDRESS_0);
    if (es_handle) {
        const es8311_clock_config_t es_clk = {
            .mclk_inverted = false,
            .sclk_inverted = false,
            .mclk_from_mclk_pin = true,
            .mclk_frequency = EXAMPLE_MCLK_FREQ_HZ,
            .sample_frequency = EXAMPLE_SAMPLE_RATE
        };
        
        esp_err_t err = es8311_init(es_handle, &es_clk, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16);
        if (err == ESP_OK) {
            es8311_sample_frequency_config(es_handle, EXAMPLE_SAMPLE_RATE * EXAMPLE_MCLK_MULTIPLE, EXAMPLE_SAMPLE_RATE);
            
            // Set initial volume and microphone gain
            es8311_voice_volume_set(es_handle, 75, NULL);
            es8311_microphone_gain_set(es_handle, ES8311_MIC_GAIN_36DB);
            Serial.println("VoiceAssistant: ES8311 Codec successfully configured.");
        } else {
            Serial.println("VoiceAssistant: Failed to initialize ES8311 Codec!");
        }
    } else {
        Serial.println("VoiceAssistant: Failed to create ES8311 handle!");
    }

    // Enable amplifier power
    digitalWrite(AP_ENABLE, HIGH);

    // 3. Initialize Audio Player Pinout
    audio_player.setPinout(I2S_BCK, I2S_WS, I2S_DOUT, I2S_MCK);
    audio_player.setVolume(21); // Set software volume to maximum, we control via ES8311 hardware volume

    // 4. Initialize WebSocket Client
    Serial.printf("VoiceAssistant: Connecting to Home Assistant at ws://%s:%d/api/websocket\n", HA_HOST, HA_PORT);
    webSocket.begin(HA_HOST, HA_PORT, "/api/websocket");
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000);
}

void VoiceAssistant::loop() {
    // Poll WebSocket client
    webSocket.loop();

    // Poll Audio Player
    if (player_running) {
        audio_player.loop();
    }

    // 1. Process Hardware Volume Controls dynamically from the UI slider
    int ui_vol = ui_get_volume();
    if (ui_vol != current_volume) {
        current_volume = ui_vol;
        if (es_handle) {
            es8311_voice_volume_set(es_handle, current_volume, NULL);
            Serial.printf("VoiceAssistant: Volume updated to %d%%\n", current_volume);
        }
    }

    // 2. Process Mute state dynamically from the UI button
    bool ui_mute = ui_is_muted();
    if (ui_mute != last_mute_state) {
        last_mute_state = ui_mute;
        if (es_handle) {
            es8311_voice_mute(es_handle, last_mute_state);
            Serial.printf("VoiceAssistant: Mute state updated to %s\n", last_mute_state ? "MUTED" : "UNMUTED");
        }
    }

    // 3. Raw audio streaming logic when in LISTENING state
    if (is_recording && ws_connected && ws_authenticated && stt_handler_id >= 0) {
        // Construct binary frame: [1 byte stt_handler_id] + [PCM bytes]
        uint8_t buffer[AUDIO_CHUNK_SIZE + 1];
        buffer[0] = (uint8_t)stt_handler_id;

        // Read mono 16-bit 16kHz PCM data from microphone
        size_t read_bytes = mic_i2s.readBytes((char*)(buffer + 1), AUDIO_CHUNK_SIZE);
        if (read_bytes > 0) {
            webSocket.sendBIN(buffer, read_bytes + 1);
        }
    }
}

void VoiceAssistant::start_listening() {
    if (player_running) {
        audio_player.stopSong();
        player_running = false;
    }

    if (!ws_connected || !ws_authenticated) {
        Serial.println("VoiceAssistant: Cannot start pipeline. Not connected to HA.");
        ui_set_state(STATE_IDLE);
        ui_set_status_text("HA OFFLINE");
        ui_set_transcript_text("Error: WebSocket connection to Home Assistant is offline.");
        return;
    }

    Serial.println("VoiceAssistant: Triggering Home Assistant Assist Pipeline...");
    send_pipeline_run();
}

void VoiceAssistant::stop_listening() {
    if (is_recording) {
        stop_recording();

        // Send a single binary byte containing only the handler ID to signal EOF to HA
        if (ws_connected && stt_handler_id >= 0) {
            uint8_t eof_packet[1];
            eof_packet[0] = (uint8_t)stt_handler_id;
            webSocket.sendBIN(eof_packet, 1);
            Serial.println("VoiceAssistant: Sent EOF signal to Home Assistant.");
        }
    }
}

bool VoiceAssistant::is_active() {
    return is_recording || player_running;
}

// Global hook called when TTS playback ends
void VoiceAssistant::on_playback_finished() {
    Serial.println("VoiceAssistant: TTS Playback finished.");
    player_running = false;
    
    // Reset state back to Idle
    ui_set_state(STATE_IDLE);
    ui_set_status_text("SYSTEM ONLINE");
}

static void start_recording() {
    if (is_recording) return;

    // Set pins and initialize I2S RX channel
    mic_i2s.setPins(I2S_BCK, I2S_WS, I2S_DOUT, I2S_DINT, I2S_MCK);
    // 16kHz, 16-bit, Mono config
    if (!mic_i2s.begin(I2S_MODE_STD, 16000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO, I2S_STD_SLOT_LEFT)) {
        Serial.println("VoiceAssistant: Failed to start I2S RX channel!");
        return;
    }

    // Set read timeout to 0 so reading is completely non-blocking
    mic_i2s.setTimeout(0);

    is_recording = true;
    Serial.println("VoiceAssistant: I2S Microphone streaming active.");
}

static void stop_recording() {
    if (!is_recording) return;
    
    mic_i2s.end();
    is_recording = false;
    Serial.println("VoiceAssistant: I2S Microphone streaming stopped.");
}

static void send_pipeline_run() {
    JsonDocument doc;
    doc["id"] = msg_id++;
    doc["type"] = "assist_pipeline/run";
    doc["start_stage"] = "stt";
    doc["end_stage"] = "tts";

    JsonObject input = doc.createNestedObject("input");
    input["sample_rate"] = 16000;

    if (strlen(HA_PIPELINE_ID) > 0) {
        doc["pipeline"] = HA_PIPELINE_ID;
    }

    String jsonStr;
    serializeJson(doc, jsonStr);
    webSocket.sendTXT(jsonStr);
}

// WebSocket client event callback
static void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            ws_connected = false;
            ws_authenticated = false;
            stop_recording();
            Serial.println("VoiceAssistant: WebSocket disconnected!");
            ui_set_status_text("HA DISCONNECTED");
            break;
            
        case WStype_CONNECTED:
            ws_connected = true;
            Serial.printf("VoiceAssistant: WebSocket connected to HA. Authenticating...\n");
            ui_set_status_text("HA CONNECTING...");
            break;
            
        case WStype_TEXT: {
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, payload, length);
            if (error) {
                Serial.printf("VoiceAssistant: JSON deserialization failed: %s\n", error.c_str());
                return;
            }

            const char* type = doc["type"];
            if (!type) return;

            if (strcmp(type, "auth_required") == 0) {
                // Send Access Token
                JsonDocument authDoc;
                authDoc["type"] = "auth";
                authDoc["access_token"] = HA_TOKEN;
                
                String authStr;
                serializeJson(authDoc, authStr);
                webSocket.sendTXT(authStr);
                Serial.println("VoiceAssistant: Sent auth credentials.");
            } 
            else if (strcmp(type, "auth_ok") == 0) {
                ws_authenticated = true;
                Serial.println("VoiceAssistant: Authentication successful!");
                ui_set_status_text("SYSTEM ONLINE");
                ui_set_transcript_text("Ready. WebSocket uplink secure.");
            } 
            else if (strcmp(type, "auth_invalid") == 0) {
                ws_authenticated = false;
                Serial.println("VoiceAssistant: Authentication failed! Please verify HA_TOKEN.");
                ui_set_status_text("AUTH ERROR");
                ui_set_transcript_text("Authentication failed. Please verify your HA_TOKEN.");
            } 
            else if (strcmp(type, "event") == 0) {
                const char* event_type = doc["event"]["type"];
                if (!event_type) return;

                if (strcmp(event_type, "run-start") == 0) {
                    Serial.println("VoiceAssistant: HA Pipeline run started.");
                } 
                else if (strcmp(event_type, "stt-start") == 0) {
                    stt_handler_id = doc["event"]["data"]["stt_binary_handler_id"];
                    Serial.printf("VoiceAssistant: STT stage started. Handler ID: %d\n", stt_handler_id);
                    
                    ui_set_state(STATE_LISTENING);
                    ui_set_status_text("LISTENING...");
                    ui_set_transcript_text("Listening...");
                    
                    start_recording();
                } 
                else if (strcmp(event_type, "stt-vad-start") == 0) {
                    Serial.println("VoiceAssistant: Voice activity detected.");
                } 
                else if (strcmp(event_type, "stt-vad-end") == 0) {
                    Serial.println("VoiceAssistant: Silence detected. Processing speech...");
                    ui_set_state(STATE_THINKING);
                    ui_set_status_text("PROCESSING...");
                    stop_recording();

                    // Send EOF
                    if (stt_handler_id >= 0) {
                        uint8_t eof_packet[1];
                        eof_packet[0] = (uint8_t)stt_handler_id;
                        webSocket.sendBIN(eof_packet, 1);
                    }
                } 
                else if (strcmp(event_type, "stt-end") == 0) {
                    const char* text = doc["event"]["data"]["stt_output"]["text"];
                    if (text) {
                        Serial.printf("VoiceAssistant: Recognized Speech: \"%s\"\n", text);
                        String t = "You: \"";
                        t += text;
                        t += "\"";
                        ui_set_transcript_text(t.c_str());
                    }
                } 
                else if (strcmp(event_type, "intent-end") == 0) {
                    const char* resp = doc["event"]["data"]["intent_output"]["response"]["speech"]["plain"]["speech"];
                    if (resp) {
                        Serial.printf("VoiceAssistant: Intent response: \"%s\"\n", resp);
                        String r = "Jarvis: \"";
                        r += resp;
                        r += "\"";
                        ui_set_transcript_text(r.c_str());
                    }
                } 
                else if (strcmp(event_type, "tts-end") == 0) {
                    const char* url = doc["event"]["data"]["tts_output"]["url"];
                    if (url) {
                        // Build full absolute URL
                        String abs_url = "http://";
                        abs_url += HA_HOST;
                        abs_url += ":";
                        abs_url += String(HA_PORT);
                        abs_url += url;
                        tts_url = abs_url;
                        
                        Serial.printf("VoiceAssistant: Playing TTS from URL: %s\n", tts_url.c_str());
                        
                        // Transition UI to SPEAKING
                        ui_set_state(STATE_SPEAKING);
                        ui_set_status_text("SPEAKING...");
                        
                        // Make sure recording is ended
                        stop_recording();
                        
                        // Start playing the TTS audio
                        player_running = audio_player.connecttohost(tts_url.c_str(), "", "", HA_TOKEN);
                        if (!player_running) {
                            Serial.println("VoiceAssistant: Failed to start TTS audio playback!");
                            // Return to Idle
                            ui_set_state(STATE_IDLE);
                            ui_set_status_text("SYSTEM ONLINE");
                        }
                    }
                } 
                else if (strcmp(event_type, "run-end") == 0) {
                    Serial.println("VoiceAssistant: Pipeline run completed.");
                    // If we did not receive any TTS url to play, reset to Idle
                    if (!player_running) {
                        ui_set_state(STATE_IDLE);
                        ui_set_status_text("SYSTEM ONLINE");
                    }
                } 
                else if (strcmp(event_type, "error") == 0) {
                    const char* code = doc["event"]["data"]["code"];
                    const char* message = doc["event"]["data"]["message"];
                    Serial.printf("VoiceAssistant: Pipeline error: %s - %s\n", code ? code : "", message ? message : "");
                    
                    ui_set_state(STATE_IDLE);
                    ui_set_status_text("PIPELINE ERROR");
                    if (message) {
                        String errMsg = "Error: ";
                        errMsg += message;
                        ui_set_transcript_text(errMsg.c_str());
                    }
                    stop_recording();
                }
            }
            break;
        }
        
        case WStype_BIN:
            // HA WebSocket API does not send binary frames to the client in standard configuration
            break;
            
        case WStype_ERROR:
            Serial.println("VoiceAssistant: WebSocket error event!");
            break;
    }
}
