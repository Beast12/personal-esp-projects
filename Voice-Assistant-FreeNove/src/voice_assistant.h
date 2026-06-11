#ifndef VOICE_ASSISTANT_H
#define VOICE_ASSISTANT_H

#include <Arduino.h>

class VoiceAssistant {
public:
    static void init();
    static void loop();
    static void start_listening(bool wake_word_mode = false);
    static void stop_listening();
    static bool is_active();
    static void on_playback_finished();
};

#endif // VOICE_ASSISTANT_H
