#ifndef UI_H
#define UI_H

#include <lvgl.h>

enum AssistantState {
    STATE_IDLE,
    STATE_LISTENING,
    STATE_THINKING,
    STATE_SPEAKING
};

void ui_init(void);
void ui_set_state(AssistantState state);
AssistantState ui_get_state(void);
void ui_set_status_text(const char *text);
void ui_set_transcript_text(const char *text);
bool ui_is_muted(void);
int ui_get_volume(void);

#endif // UI_H
