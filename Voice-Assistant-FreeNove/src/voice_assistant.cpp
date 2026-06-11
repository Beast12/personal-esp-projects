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
static Audio* audio_player = nullptr;
static bool should_delete_player = false;
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
static void send_pipeline_run(bool wake_word_mode);

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

    // 1. Initialize Amplifier Power pin (Active-Low)
    pinMode(AP_ENABLE, OUTPUT);
    digitalWrite(AP_ENABLE, HIGH); // Mute/disable initially to prevent speaker pops

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
            es8311_microphone_config(es_handle, false);
            
            // Set initial volume and microphone gain
            es8311_voice_volume_set(es_handle, 75, NULL);
            es8311_microphone_gain_set(es_handle, ES8311_MIC_GAIN_42DB);
            Serial.println("VoiceAssistant: ES8311 Codec successfully configured.");
        } else {
            Serial.println("VoiceAssistant: Failed to initialize ES8311 Codec!");
        }
    } else {
        Serial.println("VoiceAssistant: Failed to create ES8311 handle!");
    }

    // Keep amplifier disabled initially; we will enable it dynamically during playback

    // 3. Audio Player will be initialized dynamically during playback

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
    if (player_running && audio_player) {
        audio_player->loop();
    }

    // Safely delete audio player from the main loop if requested
    if (should_delete_player) {
        should_delete_player = false;
        if (audio_player) {
            delete audio_player;
            audio_player = nullptr;
            Serial.println("VoiceAssistant: Audio player deleted safely.");
        }
        // Disable amplifier power (active-low)
        digitalWrite(AP_ENABLE, HIGH);
        // Reset state back to Idle
        ui_set_state(STATE_IDLE);
        ui_set_status_text("SYSTEM ONLINE");
        
        // Auto-restart wake-word detection
        VoiceAssistant::start_listening(true);
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
            // Audio diagnostics
            int16_t* samples = (int16_t*)(buffer + 1);
            size_t num_samples = read_bytes / 2;
            int16_t min_val = 32767;
            int16_t max_val = -32768;
            int64_t sum = 0;
            for (size_t i = 0; i < num_samples; i++) {
                int16_t sample = samples[i];
                if (sample < min_val) min_val = sample;
                if (sample > max_val) max_val = sample;
                sum += abs(sample);
            }
            static uint32_t chunk_count = 0;
            if (chunk_count++ % 15 == 0) {
                Serial.printf("Audio stream diagnostics: read=%d, min=%d, max=%d, avg_abs=%d\n",
                              (int)read_bytes, min_val, max_val, (int)(sum / num_samples));
            }

            webSocket.sendBIN(buffer, read_bytes + 1);
        }
    }
}

void VoiceAssistant::start_listening(bool wake_word_mode) {
    stt_handler_id = -1;
    if (player_running) {
        if (audio_player) {
            audio_player->stopSong();
        }
        player_running = false;
        should_delete_player = true;
    }

    if (!ws_connected || !ws_authenticated) {
        Serial.println("VoiceAssistant: Cannot start pipeline. Not connected to HA.");
        ui_set_state(STATE_IDLE);
        ui_set_status_text("HA OFFLINE");
        ui_set_transcript_text("Error: WebSocket connection to Home Assistant is offline.");
        return;
    }

    Serial.printf("VoiceAssistant: Triggering Home Assistant Assist Pipeline (wake_word=%s)...\n", wake_word_mode ? "true" : "false");
    send_pipeline_run(wake_word_mode);
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
    Serial.println("VoiceAssistant: TTS Playback finished. Requesting player deletion.");
    player_running = false;
    should_delete_player = true;
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

    // Set read timeout to 100ms to allow DMA buffer to accumulate enough bytes
    mic_i2s.setTimeout(100);

    is_recording = true;
    Serial.println("VoiceAssistant: I2S Microphone streaming active.");
}

static void stop_recording() {
    if (!is_recording) return;
    
    mic_i2s.end();
    is_recording = false;
    Serial.println("VoiceAssistant: I2S Microphone streaming stopped.");
}

static void send_pipeline_run(bool wake_word_mode) {
    JsonDocument doc;
    doc["id"] = msg_id++;
    doc["type"] = "assist_pipeline/run";
    doc["start_stage"] = wake_word_mode ? "wake_word" : "stt";
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
                // Automatically enter wake-word detection mode on boot
                VoiceAssistant::start_listening(true);
            } 
            else if (strcmp(type, "auth_invalid") == 0) {
                ws_authenticated = false;
                Serial.println("VoiceAssistant: Authentication failed! Please verify HA_TOKEN.");
                ui_set_status_text("AUTH ERROR");
                ui_set_transcript_text("Authentication failed. Please verify your HA_TOKEN.");
            } 
            else if (strcmp(type, "result") == 0) {
                bool success = doc["success"];
                if (!success) {
                    const char* err_code = doc["error"]["code"];
                    const char* err_msg = doc["error"]["message"];
                    Serial.printf("VoiceAssistant: Command failed! Code: %s, Message: %s\n", 
                                  err_code ? err_code : "", err_msg ? err_msg : "");
                } else {
                    Serial.println("VoiceAssistant: Command succeeded (result).");
                }
            }
            else if (strcmp(type, "event") == 0) {
                const char* event_type = doc["event"]["type"];
                if (!event_type) return;
                
                Serial.printf("VoiceAssistant: Received event type: %s\n", event_type);

                if (strcmp(event_type, "run-start") == 0) {
                    stt_handler_id = doc["event"]["data"]["runner_data"]["stt_binary_handler_id"];
                    Serial.printf("VoiceAssistant: HA Pipeline run started. Handler ID: %d\n", stt_handler_id);
                } 
                else if (strcmp(event_type, "wake_word-start") == 0) {
                    Serial.println("VoiceAssistant: Wake-word detection active. Listening on server...");
                    ui_set_state(STATE_IDLE);
                    ui_set_status_text("READY (SAY JARVIS)");
                    ui_set_transcript_text("Listening for \"Hey Jarvis\"...");
                    start_recording();
                }
                else if (strcmp(event_type, "wake_word-end") == 0) {
                    Serial.println("VoiceAssistant: Wake-word detected!");
                }
                else if (strcmp(event_type, "stt-start") == 0) {
                    Serial.println("VoiceAssistant: STT stage started.");
                    
                    ui_set_state(STATE_LISTENING);
                    ui_set_status_text("LISTENING...");
                    ui_set_transcript_text("Listening...");
                    
                    if (!is_recording) {
                        start_recording();
                    }
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
                        
                        // Create the audio player dynamically
                        if (audio_player) {
                            delete audio_player;
                        }
                        audio_player = new Audio();
                        
                        // Set pinout and initial volume
                        audio_player->setPinout(I2S_BCK, I2S_WS, I2S_DOUT, I2S_MCK);
                        audio_player->setVolume(21);
                        
                        // Ensure ES8311 is unmuted and volume is applied before starting stream
                        if (es_handle) {
                            es8311_voice_volume_set(es_handle, current_volume >= 0 ? current_volume : 70, NULL);
                            es8311_voice_mute(es_handle, last_mute_state);
                        }
                        
                        // Enable amplifier power (active-low)
                        digitalWrite(AP_ENABLE, LOW);
                        Serial.println("VoiceAssistant: Audio amplifier enabled.");

                        // Start playing the TTS audio
                        player_running = audio_player->connecttohost(tts_url.c_str(), "", "", HA_TOKEN);
                        if (!player_running) {
                            Serial.println("VoiceAssistant: Failed to start TTS audio playback!");
                            delete audio_player;
                            audio_player = nullptr;
                            // Disable amplifier power (active-low)
                            digitalWrite(AP_ENABLE, HIGH);
                            // Return to Idle
                            ui_set_state(STATE_IDLE);
                            ui_set_status_text("SYSTEM ONLINE");
                        }
                    }
                } 
                else if (strcmp(event_type, "run-end") == 0) {
                    Serial.println("VoiceAssistant: Pipeline run completed.");
                    // If we did not receive any TTS url to play, restart wake-word pipeline automatically
                    if (!player_running) {
                        VoiceAssistant::start_listening(true);
                    }
                } 
                else if (strcmp(event_type, "error") == 0) {
                    const char* code = doc["event"]["data"]["code"];
                    const char* message = doc["event"]["data"]["message"];
                    Serial.printf("VoiceAssistant: Pipeline error: %s - %s\n", code ? code : "", message ? message : "");
                    
                    stop_recording();
                    // Ensure amplifier is disabled on error
                    digitalWrite(AP_ENABLE, HIGH);
                    
                    if (code && strcmp(code, "wake-word-timeout") != 0) {
                        ui_set_state(STATE_IDLE);
                        ui_set_status_text("PIPELINE ERROR");
                        if (message) {
                            String errMsg = "Error: ";
                            errMsg += message;
                            ui_set_transcript_text(errMsg.c_str());
                        }
                    }
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

// Audio player callbacks
void audio_info(const char *info) {
    Serial.printf("AudioPlayer info: %s\n", info);
    
    // Check for sample rate updates from the decoder
    if (strstr(info, "SampleRate:") != NULL) {
        int rate = atoi(info + 11);
        if (rate > 0 && es_handle) {
            Serial.printf("VoiceAssistant: Reconfiguring ES8311 sample rate to %d Hz...\n", rate);
            int multipliers[] = {256, 384, 512, 128};
            bool success = false;
            for (int m : multipliers) {
                esp_err_t err = es8311_sample_frequency_config(es_handle, rate * m, rate);
                if (err == ESP_OK) {
                    Serial.printf("VoiceAssistant: Configured ES8311 sample rate with MCLK multiplier %d\n", m);
                    success = true;
                    break;
                }
            }
            if (!success) {
                Serial.printf("VoiceAssistant: Failed to configure ES8311 sample rate for %d Hz!\n", rate);
            }
        }
    }
    
    // If stream is lost, don't try to reconnect to an ephemeral TTS URL
    if (strstr(info, "Stream lost") != NULL) {
        Serial.println("VoiceAssistant: Stream lost detected. Stopping playback.");
        VoiceAssistant::on_playback_finished();
    }
}
