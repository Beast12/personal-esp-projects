#include "ui.h"
#include <Arduino.h>

// Global UI objects
static lv_obj_t * screen;
static lv_obj_t * status_label;
static lv_obj_t * wifi_label;
static lv_obj_t * transcript_label;
static lv_obj_t * reactor_arc;
static lv_obj_t * reactor_core;
static lv_obj_t * btn_mic;
static lv_obj_t * btn_mute;
static lv_obj_t * slider_vol;

// State management variables
static AssistantState current_state = STATE_IDLE;
static bool is_muted = false;

// Custom colors
#define COLOR_CYAN     lv_color_hex(0x00D2FF)
#define COLOR_DARK_CYAN lv_color_hex(0x005A70)
#define COLOR_ORANGE   lv_color_hex(0xFF5F00)
#define COLOR_AMBER    lv_color_hex(0xFFD200)
#define COLOR_BG       lv_color_hex(0x0A0D10)
#define COLOR_PANEL    lv_color_hex(0x11161D)
#define COLOR_RED      lv_color_hex(0xFF2E2E)
#define COLOR_DARK_RED lv_color_hex(0x8A1010)

// Animation Callbacks
static void rotation_anim_cb(void * var, int32_t val)
{
    lv_arc_set_rotation((lv_obj_t *)var, val);
}

static void pulse_anim_cb(void * var, int32_t val)
{
    lv_obj_set_style_shadow_width((lv_obj_t *)var, val, 0);
}

static void size_anim_cb(void * var, int32_t val)
{
    lv_obj_set_size((lv_obj_t *)var, val, val);
}

// Button Events
static void btn_mic_cb(lv_event_t * e)
{
    Serial.println("UI Event: Mic Triggered");
    if (current_state == STATE_IDLE) {
        ui_set_state(STATE_LISTENING);
        ui_set_status_text("LISTENING...");
        ui_set_transcript_text("Hearing wake trigger...");
    } else {
        ui_set_state(STATE_IDLE);
        ui_set_status_text("SYSTEM ONLINE");
    }
}

static void btn_mute_cb(lv_event_t * e)
{
    is_muted = !is_muted;
    Serial.printf("UI Event: Mute State = %s\n", is_muted ? "MUTED" : "UNMUTED");
    
    if (is_muted) {
        lv_obj_set_style_bg_color(btn_mute, COLOR_DARK_RED, 0);
        lv_obj_set_style_border_color(btn_mute, COLOR_RED, 0);
    } else {
        lv_obj_set_style_bg_color(btn_mute, COLOR_PANEL, 0);
        lv_obj_set_style_border_color(btn_mute, COLOR_DARK_CYAN, 0);
    }
}

static void slider_vol_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    int32_t value = lv_slider_get_value(slider);
    Serial.printf("UI Event: Volume Changed = %d%%\n", (int)value);
}

void ui_init(void)
{
    // Configure default screen style (Dark Background)
    screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, COLOR_BG, 0);

    // 1. Header Area
    lv_obj_t * header = lv_obj_create(screen);
    lv_obj_set_size(header, 300, 40);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_bg_color(header, COLOR_PANEL, 0);
    lv_obj_set_style_border_color(header, COLOR_DARK_CYAN, 0);
    lv_obj_set_style_border_width(header, 1, 0);
    lv_obj_set_style_radius(header, 4, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * title = lv_label_create(header);
    lv_label_set_text(title, "J.A.R.V.I.S.");
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_set_style_text_color(title, COLOR_CYAN, 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);

    wifi_label = lv_label_create(header);
    lv_label_set_text(wifi_label, "WI-FI DISCONNECTED");
    lv_obj_align(wifi_label, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_set_style_text_color(wifi_label, COLOR_RED, 0);
    lv_obj_set_style_text_font(wifi_label, &lv_font_montserrat_14, 0);

    // 2. Middle Arc Reactor Area
    lv_obj_t * reactor_container = lv_obj_create(screen);
    lv_obj_set_size(reactor_container, 300, 210);
    lv_obj_align(reactor_container, LV_ALIGN_TOP_MID, 0, 60);
    lv_obj_set_style_bg_color(reactor_container, COLOR_PANEL, 0);
    lv_obj_set_style_border_color(reactor_container, COLOR_DARK_CYAN, 0);
    lv_obj_set_style_border_width(reactor_container, 1, 0);
    lv_obj_set_style_radius(reactor_container, 8, 0);
    lv_obj_clear_flag(reactor_container, LV_OBJ_FLAG_SCROLLABLE);

    // Outer Arc
    reactor_arc = lv_arc_create(reactor_container);
    lv_obj_set_size(reactor_arc, 130, 130);
    lv_obj_align(reactor_arc, LV_ALIGN_CENTER, 0, -20);
    lv_arc_set_bg_angles(reactor_arc, 0, 360);
    lv_arc_set_angles(reactor_arc, 0, 90); // 90 degree active segment
    lv_obj_remove_style(reactor_arc, NULL, LV_PART_KNOB);
    lv_obj_set_style_arc_color(reactor_arc, COLOR_DARK_CYAN, LV_PART_MAIN);
    lv_obj_set_style_arc_width(reactor_arc, 4, LV_PART_MAIN);
    lv_obj_set_style_arc_color(reactor_arc, COLOR_CYAN, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(reactor_arc, 8, LV_PART_INDICATOR);

    // Glowing core
    reactor_core = lv_obj_create(reactor_container);
    lv_obj_set_size(reactor_core, 60, 60);
    lv_obj_align_to(reactor_core, reactor_arc, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_radius(reactor_core, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(reactor_core, COLOR_CYAN, 0);
    lv_obj_set_style_border_width(reactor_core, 0, 0);
    lv_obj_set_style_shadow_color(reactor_core, COLOR_CYAN, 0);
    lv_obj_set_style_shadow_width(reactor_core, 15, 0);
    lv_obj_set_style_shadow_spread(reactor_core, 4, 0);

    // Status Label below the reactor
    status_label = lv_label_create(reactor_container);
    lv_label_set_text(status_label, "SYSTEM OFFLINE");
    lv_obj_align(status_label, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_text_color(status_label, COLOR_CYAN, 0);
    lv_obj_set_style_text_font(status_label, &lv_font_montserrat_14, 0);

    // 3. Lower Transcript / Output Text Panel
    lv_obj_t * text_panel = lv_obj_create(screen);
    lv_obj_set_size(text_panel, 300, 110);
    lv_obj_align(text_panel, LV_ALIGN_TOP_MID, 0, 280);
    lv_obj_set_style_bg_color(text_panel, COLOR_PANEL, 0);
    lv_obj_set_style_border_color(text_panel, COLOR_DARK_CYAN, 0);
    lv_obj_set_style_border_width(text_panel, 1, 0);
    lv_obj_set_style_radius(text_panel, 8, 0);
    lv_obj_set_style_pad_all(text_panel, 10, 0);
    lv_obj_clear_flag(text_panel, LV_OBJ_FLAG_SCROLLABLE);

    transcript_label = lv_label_create(text_panel);
    lv_label_set_text(transcript_label, "System initialization pending...\nTouch microphone or connect Wi-Fi to start.");
    lv_obj_set_width(transcript_label, 280);
    lv_obj_align(transcript_label, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_text_color(transcript_label, COLOR_CYAN, 0);
    lv_obj_set_style_text_font(transcript_label, &lv_font_montserrat_14, 0);
    lv_label_set_long_mode(transcript_label, LV_LABEL_LONG_WRAP);

    // 4. Bottom Controls Row
    lv_obj_t * controls_row = lv_obj_create(screen);
    lv_obj_set_size(controls_row, 300, 70);
    lv_obj_align(controls_row, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(controls_row, COLOR_BG, 0);
    lv_obj_set_style_border_width(controls_row, 0, 0);
    lv_obj_set_style_pad_all(controls_row, 0, 0);
    lv_obj_clear_flag(controls_row, LV_OBJ_FLAG_SCROLLABLE);

    // Mic Trigger Button
    btn_mic = lv_btn_create(controls_row);
    lv_obj_set_size(btn_mic, 65, 50);
    lv_obj_align(btn_mic, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_bg_color(btn_mic, COLOR_PANEL, 0);
    lv_obj_set_style_border_color(btn_mic, COLOR_DARK_CYAN, 0);
    lv_obj_set_style_border_width(btn_mic, 1, 0);
    lv_obj_set_style_radius(btn_mic, 10, 0);
    lv_obj_t * mic_label = lv_label_create(btn_mic);
    lv_label_set_text(mic_label, "TALK");
    lv_obj_align(mic_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(mic_label, COLOR_CYAN, 0);
    lv_obj_set_style_text_font(mic_label, &lv_font_montserrat_14, 0);
    lv_obj_add_event_cb(btn_mic, btn_mic_cb, LV_EVENT_CLICKED, NULL);

    // Mute Button
    btn_mute = lv_btn_create(controls_row);
    lv_obj_set_size(btn_mute, 65, 50);
    lv_obj_align(btn_mute, LV_ALIGN_LEFT_MID, 75, 0);
    lv_obj_set_style_bg_color(btn_mute, COLOR_PANEL, 0);
    lv_obj_set_style_border_color(btn_mute, COLOR_DARK_CYAN, 0);
    lv_obj_set_style_border_width(btn_mute, 1, 0);
    lv_obj_set_style_radius(btn_mute, 10, 0);
    lv_obj_t * mute_label = lv_label_create(btn_mute);
    lv_label_set_text(mute_label, "MUTE");
    lv_obj_align(mute_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(mute_label, COLOR_CYAN, 0);
    lv_obj_set_style_text_font(mute_label, &lv_font_montserrat_14, 0);
    lv_obj_add_event_cb(btn_mute, btn_mute_cb, LV_EVENT_CLICKED, NULL);

    // Volume Slider
    slider_vol = lv_slider_create(controls_row);
    lv_obj_set_size(slider_vol, 135, 12);
    lv_obj_align(slider_vol, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_slider_set_range(slider_vol, 0, 100);
    lv_slider_set_value(slider_vol, 70, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider_vol, COLOR_PANEL, LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider_vol, COLOR_DARK_CYAN, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider_vol, COLOR_CYAN, LV_PART_KNOB);
    lv_obj_set_style_pad_all(slider_vol, 2, LV_PART_KNOB);
    lv_obj_add_event_cb(slider_vol, slider_vol_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Set Default Idle State animations
    ui_set_state(STATE_IDLE);
}

void ui_set_state(AssistantState state)
{
    current_state = state;

    // Delete existing animations on reactor components
    lv_anim_del(reactor_arc, rotation_anim_cb);
    lv_anim_del(reactor_core, pulse_anim_cb);
    lv_anim_del(reactor_core, size_anim_cb);

    // Reset Core default sizes
    lv_obj_set_size(reactor_core, 60, 60);

    lv_color_t theme_color;
    int rot_duration = 0;
    int pulse_min = 10, pulse_max = 20, pulse_speed = 1000;

    switch (state)
    {
        case STATE_IDLE:
            theme_color = COLOR_CYAN;
            rot_duration = 4000;       // Slow rotation
            pulse_min = 12;
            pulse_max = 24;
            pulse_speed = 1500;       // Slow breathing pulse
            lv_arc_set_angles(reactor_arc, 0, 90); // Simple single segment
            break;

        case STATE_LISTENING:
            theme_color = COLOR_ORANGE;
            rot_duration = 1000;       // Fast rotation
            pulse_min = 18;
            pulse_max = 35;
            pulse_speed = 400;        // Fast energetic pulse
            lv_arc_set_angles(reactor_arc, 0, 180); // Double size segment
            break;

        case STATE_THINKING:
            theme_color = COLOR_AMBER;
            rot_duration = 750;        // Extremely fast rotation
            pulse_min = 8;
            pulse_max = 18;
            pulse_speed = 200;        // Rapid vibrations
            lv_arc_set_angles(reactor_arc, 0, 275); // Almost full ring
            break;

        case STATE_SPEAKING:
            theme_color = COLOR_CYAN;
            rot_duration = 2000;       // Medium rotation
            pulse_min = 15;
            pulse_max = 40;
            pulse_speed = 600;        // Fluid voice-like pulse
            lv_arc_set_angles(reactor_arc, 0, 120);
            
            // Also animate size for voice simulation!
            lv_anim_t as;
            lv_anim_init(&as);
            lv_anim_set_var(&as, reactor_core);
            lv_anim_set_exec_cb(&as, size_anim_cb);
            lv_anim_set_values(&as, 50, 70); // Core size changes
            lv_anim_set_time(&as, 400);
            lv_anim_set_playback_time(&as, 400);
            lv_anim_set_repeat_count(&as, LV_ANIM_REPEAT_INFINITE);
            lv_anim_start(&as);
            break;
    }

    // Apply color changes
    lv_obj_set_style_arc_color(reactor_arc, theme_color, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(reactor_core, theme_color, 0);
    lv_obj_set_style_shadow_color(reactor_core, theme_color, 0);

    // Apply rotation animation if enabled
    if (rot_duration > 0) {
        lv_anim_t ar;
        lv_anim_init(&ar);
        lv_anim_set_var(&ar, reactor_arc);
        lv_anim_set_exec_cb(&ar, rotation_anim_cb);
        lv_anim_set_values(&ar, 0, 360);
        lv_anim_set_time(&ar, rot_duration);
        lv_anim_set_repeat_count(&ar, LV_ANIM_REPEAT_INFINITE);
        lv_anim_start(&ar);
    }

    // Apply breathing pulse animation
    lv_anim_t ap;
    lv_anim_init(&ap);
    lv_anim_set_var(&ap, reactor_core);
    lv_anim_set_exec_cb(&ap, pulse_anim_cb);
    lv_anim_set_values(&ap, pulse_min, pulse_max);
    lv_anim_set_time(&ap, pulse_speed);
    lv_anim_set_playback_time(&ap, pulse_speed);
    lv_anim_set_repeat_count(&ap, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&ap);
}

AssistantState ui_get_state(void)
{
    return current_state;
}

void ui_set_status_text(const char *text)
{
    if (status_label) {
        lv_label_set_text(status_label, text);
    }
}

void ui_set_transcript_text(const char *text)
{
    if (transcript_label) {
        lv_label_set_text(transcript_label, text);
    }
}

// Hook to update Wi-Fi status indicators
extern "C" void ui_update_wifi_status(bool connected, const char * ip_or_ssid)
{
    if (!wifi_label) return;
    
    if (connected) {
        lv_obj_set_style_text_color(wifi_label, COLOR_CYAN, 0);
        lv_label_set_text_fmt(wifi_label, "WIFI: %s", ip_or_ssid);
    } else {
        lv_obj_set_style_text_color(wifi_label, COLOR_RED, 0);
        lv_label_set_text(wifi_label, "WIFI DISCONNECTED");
    }
}

bool ui_is_muted(void)
{
    return is_muted;
}

int ui_get_volume(void)
{
    if (slider_vol) {
        return lv_slider_get_value(slider_vol);
    }
    return 70;
}
