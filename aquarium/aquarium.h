#pragma once

#include "esphome.h"
#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>

// Define structures for our aquarium actors
struct FishState {
  lv_obj_t* body_obj;
  lv_obj_t* tail_obj;
  lv_obj_t* eye_obj;
  float x;
  float y;
  float vx;
  float vy;
  int width;
  int height;
  bool facing_right;
  int state;      // 0 = normal, 1 = panic, 2 = chasing food
  int type;       // 0 = goldfish, 1 = neon tetra, 2 = blowfish, 3 = seahorse
  int puff_timer; // blowfish specific puff frame count
  int depth;      // 0 = front, 1 = middle, 2 = back
  lv_color_t base_color;
};

struct BubbleState {
  lv_obj_t* obj;
  float x;
  float y;
  float speed;
  bool active;
};

struct JellyfishState {
  lv_obj_t* bell_obj;
  lv_obj_t* tentacle_l;
  lv_obj_t* tentacle_c;
  lv_obj_t* tentacle_r;
  float x;
  float y;
  float base_y;
  float speed_x;
  float speed_y;
  float phase;
};

struct FoodState {
  lv_obj_t* obj;
  float x;
  float y;
  float speed;
  bool active;
};

struct SeaweedState {
  lv_obj_t* base_obj;
  lv_obj_t* mid_obj;
  lv_obj_t* top_obj;
  float base_x;
  float base_width;
  float mid_width;
  float top_width;
  float speed;
  float phase;
};

class Aquarium {
 public:
  std::vector<FishState> fish;
  std::vector<BubbleState> bubbles;
  std::vector<FoodState> food;
  std::vector<SeaweedState> seaweed;
  
  lv_obj_t* bg_obj = nullptr;
  
  // Crab parts
  lv_obj_t* crab_body_obj = nullptr;
  lv_obj_t* crab_claw_l_obj = nullptr;
  lv_obj_t* crab_claw_r_obj = nullptr;
  lv_obj_t* crab_eye_l_obj = nullptr;
  lv_obj_t* crab_eye_r_obj = nullptr;
  
  // Jellyfish state
  JellyfishState jelly;
  
  // Shark parts
  lv_obj_t* shark_body_obj = nullptr;
  lv_obj_t* shark_tail_obj = nullptr;
  lv_obj_t* shark_fin_obj = nullptr;
  lv_obj_t* shark_eye_obj = nullptr;
  
  lv_obj_t* tap_obj = nullptr;
  
  // Castle parts
  lv_obj_t* castle_keep_obj = nullptr;
  lv_obj_t* castle_tower_l_obj = nullptr;
  lv_obj_t* castle_tower_r_obj = nullptr;
  lv_obj_t* castle_roof_l_obj = nullptr;
  lv_obj_t* castle_roof_r_obj = nullptr;
  lv_obj_t* castle_gate_obj = nullptr;
  
  // Chest parts
  lv_obj_t* chest_body_obj = nullptr;
  lv_obj_t* chest_lid_obj = nullptr;
  lv_obj_t* chest_lock_obj = nullptr;

  // Alert parts
  lv_obj_t* alert_banner_obj = nullptr;
  lv_obj_t* alert_label_obj = nullptr;
  lv_obj_t* alert_light_l_obj = nullptr;
  lv_obj_t* alert_light_r_obj = nullptr;
  
  // Alert state
  bool alert_active = false;
  std::string alert_msg = "";
  uint32_t alert_start_time = 0;
  uint32_t alert_duration_ms = 0;
  
  float crab_x = 220.0f;
  float crab_y = 295.0f;
  float crab_vx = 0.6f;
  
  float shark_x = -150.0f;
  float shark_y = 120.0f;
  float shark_vx = 0.0f;
  bool shark_active = false;
  
  int tap_timer = 0;
  int panic_timer = 0;
  int fed_count = 0;
  
  bool bubbler_on = true;
  bool disco_mode = false;
  int disco_hue = 0;
  
  void setup(
      lv_obj_t* bg,
      std::vector<lv_obj_t*> fish_objs,
      std::vector<lv_obj_t*> bubble_objs,
      std::vector<lv_obj_t*> food_objs,
      std::vector<lv_obj_t*> seaweed_objs_base,
      std::vector<lv_obj_t*> seaweed_objs_mid,
      std::vector<lv_obj_t*> seaweed_objs_top,
      std::vector<lv_obj_t*> crab_parts,
      std::vector<lv_obj_t*> jelly_parts,
      std::vector<lv_obj_t*> shark_parts,
      lv_obj_t* tap,
      std::vector<lv_obj_t*> castle_parts,
      std::vector<lv_obj_t*> chest_parts,
      std::vector<lv_obj_t*> alert_parts,
      esphome::font::Font* font_large,
      esphome::font::Font* font_medium,
      esphome::font::Font* font_small) {
    
    bg_obj = bg;
    
    crab_body_obj = crab_parts[0];
    crab_claw_l_obj = crab_parts[1];
    crab_claw_r_obj = crab_parts[2];
    crab_eye_l_obj = crab_parts[3];
    crab_eye_r_obj = crab_parts[4];
    
    // Jellyfish initialization
    jelly.bell_obj = jelly_parts[0];
    jelly.tentacle_l = jelly_parts[1];
    jelly.tentacle_c = jelly_parts[2];
    jelly.tentacle_r = jelly_parts[3];
    jelly.x = 240.0f;
    jelly.y = 100.0f;
    jelly.base_y = 100.0f;
    jelly.speed_x = 0.4f;
    jelly.speed_y = 0.05f;
    jelly.phase = 0.0f;
    
    shark_body_obj = shark_parts[0];
    shark_tail_obj = shark_parts[1];
    shark_fin_obj = shark_parts[2];
    shark_eye_obj = shark_parts[3];
    
    tap_obj = tap;
    
    // Castle
    castle_keep_obj = castle_parts[0];
    castle_tower_l_obj = castle_parts[1];
    castle_tower_r_obj = castle_parts[2];
    castle_roof_l_obj = castle_parts[3];
    castle_roof_r_obj = castle_parts[4];
    castle_gate_obj = castle_parts[5];
    
    // Chest
    chest_body_obj = chest_parts[0];
    chest_lid_obj = chest_parts[1];
    chest_lock_obj = chest_parts[2];
    
    // Alert
    alert_banner_obj = alert_parts[0];
    alert_label_obj = alert_parts[1];
    alert_light_l_obj = alert_parts[2];
    alert_light_r_obj = alert_parts[3];
    
    lv_obj_add_flag(alert_banner_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(alert_label_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(alert_light_l_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(alert_light_r_obj, LV_OBJ_FLAG_HIDDEN);
    
    // Initialize 12 fish with varying species, depth layers, and starting velocities
    // Species types: 0 = Goldfish, 1 = Neon Tetra, 2 = Blowfish, 3 = Seahorse
    // Depth layers: 0 = Foreground, 1 = Midground, 2 = Background
    struct FishInit {
      int type;
      int depth;
      lv_color_t color;
      int width;
      int height;
      float vx;
      float vy;
    };
    
    std::vector<FishInit> init_data = {
      // Goldfish (type 0)
      {0, 0, lv_color_hex(0xFF8F1C), 64, 20, 1.0f, 0.3f},   // FG
      {0, 1, lv_color_hex(0xC3670B), 48, 16, -0.8f, -0.2f}, // MG
      {0, 2, lv_color_hex(0x6E3A06), 32, 12, 0.6f, 0.1f},   // BG
      
      // Neon Tetras (type 1)
      {1, 0, lv_color_hex(0x00F5FF), 48, 16, -1.6f, -0.4f}, // FG
      {1, 1, lv_color_hex(0x1F80AF), 36, 12, 1.3f, 0.3f},   // MG
      {1, 1, lv_color_hex(0x1F80AF), 36, 12, -1.2f, -0.3f}, // MG
      {1, 2, lv_color_hex(0x0E3A50), 24, 10, 0.9f, 0.2f},   // BG
      {1, 2, lv_color_hex(0x0E3A50), 24, 10, -0.8f, -0.2f}, // BG
      {1, 2, lv_color_hex(0x0E3A50), 24, 10, 0.7f, 0.1f},   // BG
      
      // Blowfish (type 2)
      {2, 0, lv_color_hex(0xF1C40F), 48, 24, 0.7f, 0.1f},   // FG
      {2, 1, lv_color_hex(0xBCA00F), 36, 18, -0.5f, -0.1f}, // MG
      
      // Seahorse (type 3)
      {3, 1, lv_color_hex(0xFF69B4), 24, 24, 0.2f, 0.6f}    // MG
    };
    
    for (size_t i = 0; i < (fish_objs.size() / 3) && i < init_data.size(); ++i) {
      auto& data = init_data[i];
      float start_x = 30.0f + (rand() % 360);
      float start_y = 40.0f + (rand() % 180);
      
      lv_obj_t* body = fish_objs[i * 3];
      lv_obj_t* tail = fish_objs[i * 3 + 1];
      lv_obj_t* eye = fish_objs[i * 3 + 2];
      
      fish.push_back({
        body,
        tail,
        eye,
        start_x,
        start_y,
        data.vx,
        data.vy,
        data.width,
        data.height,
        (data.vx > 0),
        0,
        data.type,
        0,
        data.depth,
        data.color
      });
      
      // Make sure all parts are visible and clear borders
      lv_obj_clear_flag(body, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(tail, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(eye, LV_OBJ_FLAG_HIDDEN);
      
      lv_obj_set_style_border_width(body, 0, LV_PART_MAIN);
      lv_obj_set_style_border_width(tail, 0, LV_PART_MAIN);
      lv_obj_set_style_border_width(eye, 0, LV_PART_MAIN);
      
      // Apply initial styling colors
      lv_obj_set_style_bg_color(body, data.color, LV_PART_MAIN);
      if (data.type == 1) { // Neon Tetra red tail
        lv_color_t red_tail = lv_color_hex(0xFF1744);
        if (data.depth == 1) red_tail = lv_color_hex(0xB0122E);
        else if (data.depth == 2) red_tail = lv_color_hex(0x6E0A1D);
        lv_obj_set_style_bg_color(tail, red_tail, LV_PART_MAIN);
      } else {
        lv_obj_set_style_bg_color(tail, data.color, LV_PART_MAIN);
      }
      lv_obj_set_style_bg_color(eye, lv_color_hex(0x111111), LV_PART_MAIN); // black eye pupil
    }
    
    // Initialize bubbles
    for (auto* b : bubble_objs) {
      bubbles.push_back({b, 0.0f, 0.0f, 0.0f, false});
      lv_obj_add_flag(b, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_style_text_color(b, lv_color_hex(0xAED6F1), LV_PART_MAIN); // Light blue
    }
    
    // Initialize food
    for (auto* f : food_objs) {
      food.push_back({f, 0.0f, 0.0f, 0.0f, false});
      lv_obj_add_flag(f, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_style_text_color(f, lv_color_hex(0xD35400), LV_PART_MAIN); // Orange-Brown flakes
    }
    
    // Initialize seaweed states (5 stalks) with beautiful multi-level vertical green gradients
    float base_xs[5] = {35.0f, 85.0f, 190.0f, 290.0f, 435.0f};
    float base_ws[5] = {14.0f, 18.0f, 12.0f, 16.0f, 14.0f};
    float mid_ws[5] = {11.0f, 14.0f, 9.0f, 12.0f, 11.0f};
    float top_ws[5] = {8.0f, 10.0f, 6.0f, 8.0f, 8.0f};
    
    for (size_t i = 0; i < 5; ++i) {
      float speed = 1.0f + ((rand() % 100) / 100.0f) * 1.5f;
      float phase = ((rand() % 100) / 100.0f) * 6.28f;
      seaweed.push_back({
        seaweed_objs_base[i],
        seaweed_objs_mid[i],
        seaweed_objs_top[i],
        base_xs[i],
        base_ws[i],
        mid_ws[i],
        top_ws[i],
        speed,
        phase
      });
      
      // Styling gradients across the three segments:
      // Base: Deep dark green to medium forest green
      lv_obj_set_style_bg_grad_dir(seaweed_objs_base[i], LV_GRAD_DIR_VER, LV_PART_MAIN);
      lv_obj_set_style_bg_color(seaweed_objs_base[i], lv_color_hex(0x1F8A4C), LV_PART_MAIN);
      lv_obj_set_style_bg_grad_color(seaweed_objs_base[i], lv_color_hex(0x0A3015), LV_PART_MAIN);
      
      // Mid: Forest green to glowing green
      lv_obj_set_style_bg_grad_dir(seaweed_objs_mid[i], LV_GRAD_DIR_VER, LV_PART_MAIN);
      lv_obj_set_style_bg_color(seaweed_objs_mid[i], lv_color_hex(0x2ECC71), LV_PART_MAIN);
      lv_obj_set_style_bg_grad_color(seaweed_objs_mid[i], lv_color_hex(0x1F8A4C), LV_PART_MAIN);
      
      // Top: Glowing green to pale neon green tip
      lv_obj_set_style_bg_grad_dir(seaweed_objs_top[i], LV_GRAD_DIR_VER, LV_PART_MAIN);
      lv_obj_set_style_bg_color(seaweed_objs_top[i], lv_color_hex(0x58D68D), LV_PART_MAIN);
      lv_obj_set_style_bg_grad_color(seaweed_objs_top[i], lv_color_hex(0x2ECC71), LV_PART_MAIN);
    }
    
    // Configure crab
    lv_obj_set_style_bg_color(crab_body_obj, lv_color_hex(0xE74C3C), LV_PART_MAIN); // Coral Red
    lv_obj_set_style_bg_color(crab_claw_l_obj, lv_color_hex(0xE74C3C), LV_PART_MAIN);
    lv_obj_set_style_bg_color(crab_claw_r_obj, lv_color_hex(0xE74C3C), LV_PART_MAIN);
    lv_obj_set_style_bg_color(crab_eye_l_obj, lv_color_hex(0x111111), LV_PART_MAIN); // black eye
    lv_obj_set_style_bg_color(crab_eye_r_obj, lv_color_hex(0x111111), LV_PART_MAIN); // black eye
    
    lv_obj_set_style_border_width(crab_body_obj, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(crab_claw_l_obj, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(crab_claw_r_obj, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(crab_eye_l_obj, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(crab_eye_r_obj, 0, LV_PART_MAIN);
    
    lv_obj_set_style_radius(crab_body_obj, 7, LV_PART_MAIN);
    lv_obj_set_style_radius(crab_claw_l_obj, 5, LV_PART_MAIN);
    lv_obj_set_style_radius(crab_claw_r_obj, 5, LV_PART_MAIN);
    lv_obj_set_style_radius(crab_eye_l_obj, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(crab_eye_r_obj, 2, LV_PART_MAIN);

    // Configure jellyfish styling (Semi-transparent pink/lavender)
    lv_color_t jelly_pink = lv_color_hex(0xF5B7B1);
    lv_color_t jelly_purple = lv_color_hex(0xD7BDE2);
    
    lv_obj_set_style_bg_color(jelly.bell_obj, jelly_pink, LV_PART_MAIN);
    lv_obj_set_style_bg_color(jelly.tentacle_l, jelly_purple, LV_PART_MAIN);
    lv_obj_set_style_bg_color(jelly.tentacle_c, jelly_pink, LV_PART_MAIN);
    lv_obj_set_style_bg_color(jelly.tentacle_r, jelly_purple, LV_PART_MAIN);
    
    lv_obj_set_style_border_width(jelly.bell_obj, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(jelly.tentacle_l, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(jelly.tentacle_c, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(jelly.tentacle_r, 0, LV_PART_MAIN);
    
    lv_obj_set_style_radius(jelly.bell_obj, 8, LV_PART_MAIN);
    lv_obj_set_style_radius(jelly.tentacle_l, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(jelly.tentacle_c, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(jelly.tentacle_r, 1, LV_PART_MAIN);
    
    // Configure shark
    lv_obj_set_style_bg_color(shark_body_obj, lv_color_hex(0x566573), LV_PART_MAIN); // Slate grey-blue
    lv_obj_set_style_bg_color(shark_tail_obj, lv_color_hex(0x566573), LV_PART_MAIN);
    lv_obj_set_style_bg_color(shark_fin_obj, lv_color_hex(0x566573), LV_PART_MAIN);
    lv_obj_set_style_bg_color(shark_eye_obj, lv_color_hex(0xE74C3C), LV_PART_MAIN); // Menacing red eye!
    
    lv_obj_set_style_border_width(shark_body_obj, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(shark_tail_obj, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(shark_fin_obj, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(shark_eye_obj, 0, LV_PART_MAIN);

    // Dynamic radius
    lv_obj_set_style_radius(shark_body_obj, 16, LV_PART_MAIN);
    lv_obj_set_style_radius(shark_tail_obj, 11, LV_PART_MAIN);
    lv_obj_set_style_radius(shark_fin_obj, 9, LV_PART_MAIN);
    lv_obj_set_style_radius(shark_eye_obj, 3, LV_PART_MAIN);
    
    lv_obj_add_flag(shark_body_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(shark_tail_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(shark_fin_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(shark_eye_obj, LV_OBJ_FLAG_HIDDEN);
    
    // Configure tap
    lv_obj_set_style_text_color(tap_obj, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_add_flag(tap_obj, LV_OBJ_FLAG_HIDDEN);
    
    // Configure Castle Components styling
    // Keep & Towers: Slate grey stone
    lv_obj_set_style_bg_color(castle_keep_obj, lv_color_hex(0x5D6D7E), LV_PART_MAIN);
    lv_obj_set_style_bg_color(castle_tower_l_obj, lv_color_hex(0x5D6D7E), LV_PART_MAIN);
    lv_obj_set_style_bg_color(castle_tower_r_obj, lv_color_hex(0x5D6D7E), LV_PART_MAIN);
    // Roofs: Deep Crimson
    lv_obj_set_style_bg_color(castle_roof_l_obj, lv_color_hex(0xB03A2E), LV_PART_MAIN);
    lv_obj_set_style_bg_color(castle_roof_r_obj, lv_color_hex(0xB03A2E), LV_PART_MAIN);
    // Gate: Dark Slate
    lv_obj_set_style_bg_color(castle_gate_obj, lv_color_hex(0x1A252F), LV_PART_MAIN);
    
    // Configure Treasure Chest Components styling
    // Body & Lid: Deep Rich Brown
    lv_obj_set_style_bg_color(chest_body_obj, lv_color_hex(0x6E473B), LV_PART_MAIN);
    lv_obj_set_style_bg_color(chest_lid_obj, lv_color_hex(0x6E473B), LV_PART_MAIN);
    // Gold trim / borders
    lv_obj_set_style_border_color(chest_body_obj, lv_color_hex(0xD4AF37), LV_PART_MAIN);
    lv_obj_set_style_border_color(chest_lid_obj, lv_color_hex(0xD4AF37), LV_PART_MAIN);
    // Lock: Solid Gold
    lv_obj_set_style_bg_color(chest_lock_obj, lv_color_hex(0xD4AF37), LV_PART_MAIN);
    
    // Style alert widgets
    lv_obj_set_style_bg_color(alert_banner_obj, lv_color_hex(0xE74C3C), LV_PART_MAIN);
    lv_obj_set_style_border_color(alert_banner_obj, lv_color_hex(0xF1C40F), LV_PART_MAIN);
    lv_obj_set_style_text_color(alert_label_obj, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_bg_color(alert_light_l_obj, lv_color_hex(0xF1C40F), LV_PART_MAIN);
    lv_obj_set_style_bg_color(alert_light_r_obj, lv_color_hex(0xF1C40F), LV_PART_MAIN);
  }
  
  void update(esphome::sensor::Sensor* fed_sensor) {
    float t_sec = millis() / 1000.0f;
    // 1. Update disco mode background and fish coloring
    if (disco_mode) {
      disco_hue = (disco_hue + 3) % 360;
      // Background cycles through dark neon colors
      lv_color_t bg_color = lv_color_hsv_to_rgb(disco_hue, 80, 18);
      lv_obj_set_style_bg_grad_dir(bg_obj, LV_GRAD_DIR_NONE, LV_PART_MAIN);
      lv_obj_set_style_bg_color(bg_obj, bg_color, LV_PART_MAIN);
      
      // Fish strobe colors
      for (size_t i = 0; i < fish.size(); ++i) {
        int fish_hue = (disco_hue + i * 30) % 360;
        lv_color_t color = lv_color_hsv_to_rgb(fish_hue, 95, 95);
        lv_obj_set_style_bg_color(fish[i].body_obj, color, LV_PART_MAIN);
        lv_obj_set_style_bg_color(fish[i].tail_obj, color, LV_PART_MAIN);
      }
    } else {
      // Normal background: Deep deep water blue vertical gradient
      lv_obj_set_style_bg_grad_dir(bg_obj, LV_GRAD_DIR_VER, LV_PART_MAIN);
      lv_obj_set_style_bg_color(bg_obj, lv_color_hex(0x0A2B4E), LV_PART_MAIN); // Lighter top (Aqua-Blue)
      lv_obj_set_style_bg_grad_color(bg_obj, lv_color_hex(0x020814), LV_PART_MAIN); // Darker bottom (Deep Navy)
      
      // Normal fish colors (using depth-shaded base colors)
      for (auto& f : fish) {
        lv_obj_set_style_bg_color(f.body_obj, f.base_color, LV_PART_MAIN);
        if (f.type == 1) {
          // Neon Tetra has a bright red tail!
          // Depth-shade the tail color
          lv_color_t red_tail = lv_color_hex(0xFF1744);
          if (f.depth == 1) red_tail = lv_color_hex(0xB0122E);
          else if (f.depth == 2) red_tail = lv_color_hex(0x6E0A1D);
          lv_obj_set_style_bg_color(f.tail_obj, red_tail, LV_PART_MAIN);
        } else {
          lv_obj_set_style_bg_color(f.tail_obj, f.base_color, LV_PART_MAIN);
        }
      }
    }
    
    // 2. Decrement timers
    if (panic_timer > 0) panic_timer--;
    if (tap_timer > 0) {
      tap_timer--;
      if (tap_timer == 0) {
        lv_obj_add_flag(tap_obj, LV_OBJ_FLAG_HIDDEN);
      }
    }
    
    // 2b. Update Alert State and Animation
    if (alert_active) {
      if (alert_duration_ms > 0 && (millis() - alert_start_time > alert_duration_ms)) {
        clear_alert();
      } else {
        // Pulse banner background between red (0xE74C3C) and deep maroon (0x78281F)
        float pulse = (std::sin(t_sec * 6.28f * 1.5f) + 1.0f) / 2.0f; // 1.5 Hz pulse
        lv_color_t c = lv_color_mix(lv_color_hex(0xE74C3C), lv_color_hex(0x78281F), (uint8_t)(pulse * 255));
        lv_obj_set_style_bg_color(alert_banner_obj, c, LV_PART_MAIN);
        
        // Blink left and right warning lights alternately at 4Hz
        bool blink_state = ((int)(t_sec * 4.0f) % 2) == 0;
        if (blink_state) {
          lv_obj_set_style_bg_color(alert_light_l_obj, lv_color_hex(0xF1C40F), LV_PART_MAIN); // Yellow
          lv_obj_set_style_bg_color(alert_light_r_obj, lv_color_hex(0xE74C3C), LV_PART_MAIN); // Red
        } else {
          lv_obj_set_style_bg_color(alert_light_l_obj, lv_color_hex(0xE74C3C), LV_PART_MAIN); // Red
          lv_obj_set_style_bg_color(alert_light_r_obj, lv_color_hex(0xF1C40F), LV_PART_MAIN); // Yellow
        }
      }
    }
    
    // 3. Sway Seaweed (organic multi-segment bending wave)
    for (auto& s : seaweed) {
      float sway_base = std::sin(t_sec * s.speed + s.phase) * 1.5f;
      float sway_mid = std::sin(t_sec * s.speed + s.phase + 0.3f) * 4.0f;
      float sway_top = std::sin(t_sec * s.speed + s.phase + 0.6f) * 8.0f;
      
      float x_base = s.base_x + sway_base;
      float x_mid = s.base_x + sway_mid + (s.base_width - s.mid_width) / 2.0f;
      float x_top = s.base_x + sway_top + (s.base_width - s.top_width) / 2.0f;
      
      lv_obj_set_x(s.base_obj, (int)x_base);
      lv_obj_set_x(s.mid_obj, (int)x_mid);
      lv_obj_set_x(s.top_obj, (int)x_top);
    }
    
    // 3b. Update Chest Lid opening state & bubble vibration
    if (bubbler_on) {
      // Bobbing lid when open
      float vibration = std::sin(t_sec * 18.0f) * 1.0f;
      lv_obj_set_pos(chest_lid_obj, 353, 283 + (int)vibration);
    } else {
      // Sealed lid resting on body
      lv_obj_set_pos(chest_lid_obj, 350, 290);
    }
    
    // 4. Update Fish
    for (auto& f : fish) {
      float speed_mult = 1.0f;
      if (panic_timer > 0) {
        speed_mult = 3.2f;
      } else if (alert_active) {
        speed_mult = 2.6f; // Alert speed boost!
      } else if (f.state == 2) {
        speed_mult = 2.0f; // chasing food
      }
      
      // Food detection logic (if not panicked)
      if (panic_timer == 0) {
        float nearest_dist = 99999.0f;
        int target_food_idx = -1;
        
        for (size_t i = 0; i < food.size(); ++i) {
          if (food[i].active) {
            // Calculate distance to food particle
            float dx = food[i].x - f.x;
            float dy = food[i].y - f.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist < nearest_dist) {
              nearest_dist = dist;
              target_food_idx = (int)i;
            }
          }
        }
        
        if (target_food_idx != -1) {
          f.state = 2; // Chase food
          float tx = food[target_food_idx].x;
          float ty = food[target_food_idx].y;
          float dx = tx - f.x;
          float dy = ty - f.y;
          float len = std::sqrt(dx * dx + dy * dy);
          if (len > 0) {
            f.vx = (f.vx * 0.88f) + (dx / len * 1.6f * 0.12f);
            f.vy = (f.vy * 0.88f) + (dy / len * 1.2f * 0.12f);
          }
        } else {
          f.state = 0; // Return to normal swim
        }
      } else {
        f.state = 1; // Panicked
      }
      
      // Add extra scatter/jitter when alert is active
      if (alert_active && (rand() % 100 < 10)) {
        f.vx += ((rand() % 100) / 50.0f - 1.0f) * 0.8f;
        f.vy += ((rand() % 100) / 50.0f - 1.0f) * 0.6f;
      }
      
      // Update fish position with depth-based parallax speed multiplier
      float depth_mult = (f.depth == 0) ? 1.0f : ((f.depth == 1) ? 0.75f : 0.55f);
      f.x += f.vx * speed_mult * depth_mult;
      f.y += f.vy * speed_mult * depth_mult;
      
      // Keep within vertical constraints (y: 20 to 280) and horizontal constraints
      int max_x = 480 - f.width;
      int max_y = 280;
      int min_y = 20;
      
      if (f.x < 0) {
        f.x = 0;
        f.vx = -f.vx;
        f.facing_right = true;
      } else if (f.x > max_x) {
        f.x = (float)max_x;
        f.vx = -f.vx;
        f.facing_right = false;
      }
      
      if (f.y < min_y) {
        f.y = (float)min_y;
        f.vy = -f.vy;
      } else if (f.y > max_y) {
        f.y = (float)max_y;
        f.vy = -f.vy;
      }
      
      // Random organic movements (occasional velocity changes)
      if (rand() % 100 < 4) {
        if (f.type == 3) { // Seahorse bobbing
          f.vx += ((rand() % 200 - 100) / 100.0f) * 0.12f;
          f.vy += ((rand() % 200 - 100) / 100.0f) * 0.35f;
          
          float speed = std::sqrt(f.vx * f.vx + f.vy * f.vy);
          float max_speed = 0.8f;
          float min_speed = 0.2f;
          if (speed > max_speed) {
            f.vx = (f.vx / speed) * max_speed;
            f.vy = (f.vy / speed) * max_speed;
          } else if (speed < min_speed) {
            f.vx = (f.vx / speed) * min_speed;
            f.vy = (f.vy / speed) * min_speed;
          }
        } else {
          float dvx = ((rand() % 200 - 100) / 100.0f) * 0.4f;
          float dvy = ((rand() % 200 - 100) / 100.0f) * 0.3f;
          f.vx += dvx;
          f.vy += dvy;
          
          // Clamp speed
          float speed = std::sqrt(f.vx * f.vx + f.vy * f.vy);
          float max_speed = (f.type == 1) ? 2.3f : 1.3f; // Tetra is speedier
          float min_speed = 0.4f;
          
          if (speed > max_speed) {
            f.vx = (f.vx / speed) * max_speed;
            f.vy = (f.vy / speed) * max_speed;
          } else if (speed < min_speed) {
            f.vx = (f.vx / speed) * min_speed;
            f.vy = (f.vy / speed) * min_speed;
          }
        }
        
        f.facing_right = (f.vx > 0);
      }
      
      // Calculate dynamic component sizes
      float body_w = 0.0f;
      float body_h = 0.0f;
      float tail_w = 0.0f;
      float tail_h = 0.0f;
      float eye_size = 0.0f;
      
      // Check puffing for Blowfish
      bool puffed = false;
      if (f.type == 2 && f.puff_timer > 0) {
        f.puff_timer--;
        puffed = true;
      }
      
      if (f.type == 0) { // Goldfish
        body_w = f.width * 0.70f;
        body_h = f.height;
        tail_w = f.width * 0.25f;
        tail_h = f.height * 0.75f;
        eye_size = f.height * 0.20f;
      } else if (f.type == 1) { // Neon Tetra
        body_w = f.width * 0.75f;
        body_h = f.height * 0.85f;
        tail_w = f.width * 0.25f;
        tail_h = f.height * 0.65f;
        eye_size = f.height * 0.20f;
      } else if (f.type == 2) { // Blowfish
        if (puffed) {
          body_w = f.width * 1.10f;
          body_h = f.height * 1.40f;
          tail_w = f.width * 0.20f;
          tail_h = f.height * 0.50f;
          eye_size = f.height * 0.35f;
        } else {
          body_w = f.width * 0.80f;
          body_h = f.height;
          tail_w = f.width * 0.20f;
          tail_h = f.height * 0.50f;
          eye_size = f.height * 0.25f;
        }
      } else if (f.type == 3) { // Seahorse
        body_w = f.width * 0.50f;
        body_h = f.height * 0.90f;
        tail_w = f.width * 0.30f;
        tail_h = f.height * 0.40f;
        eye_size = f.height * 0.15f;
      }
      
      // Update sizes and pill roundings dynamically
      lv_obj_set_size(f.body_obj, (int)body_w, (int)body_h);
      lv_obj_set_size(f.tail_obj, (int)tail_w, (int)tail_h);
      lv_obj_set_size(f.eye_obj, (int)eye_size, (int)eye_size);
      
      lv_obj_set_style_radius(f.body_obj, (int)(body_h / 2), LV_PART_MAIN);
      lv_obj_set_style_radius(f.tail_obj, (int)(tail_h / 2), LV_PART_MAIN);
      lv_obj_set_style_radius(f.eye_obj, (int)(eye_size / 2), LV_PART_MAIN);
      
      // Procedural Positioning based on direction
      float bx = 0.0f, by = 0.0f;
      float tx = 0.0f, ty = 0.0f;
      float ex = 0.0f, ey = 0.0f;
      
      if (f.type == 3) { // Seahorse is vertical layout
        bx = f.x + (f.width - body_w) / 2.0f;
        by = f.y;
        tx = bx - 2;
        ty = f.y + body_h - 2;
        ex = bx + (f.facing_right ? body_w - eye_size - 2 : 2);
        ey = f.y + 4;
      } else { // Horizontal layout
        if (f.facing_right) {
          bx = f.x + tail_w * 0.7f;
          by = f.y + (f.height - body_h) / 2.0f;
          tx = f.x;
          ty = f.y + (f.height - tail_h) / 2.0f;
          ex = bx + body_w - eye_size - (body_h * 0.2f);
          ey = by + (body_h * 0.2f);
        } else {
          bx = f.x;
          by = f.y + (f.height - body_h) / 2.0f;
          tx = f.x + body_w - tail_w * 0.3f;
          ty = f.y + (f.height - tail_h) / 2.0f;
          ex = bx + (body_h * 0.2f);
          ey = by + (body_h * 0.2f);
        }
      }
      
      lv_obj_set_pos(f.body_obj, (int)bx, (int)by);
      lv_obj_set_pos(f.tail_obj, (int)tx, (int)ty);
      lv_obj_set_pos(f.eye_obj, (int)ex, (int)ey);
    }
    
    // 5. Update Crab (scuttles back and forth at bottom)
    crab_x += crab_vx;
    if (crab_x < 15.0f) {
      crab_x = 15.0f;
      crab_vx = -crab_vx;
    } else if (crab_x > 415.0f) {
      crab_x = 415.0f;
      crab_vx = -crab_vx;
    }
    
    if (rand() % 100 < 3) {
      crab_vx = (crab_vx > 0) ? -0.6f : 0.6f;
    }
    
    // Position crab parts
    {
      float cbx = crab_x;
      float cby = crab_y;
      
      // Animate claws
      float cclx_l = cbx - 6.0f;
      float ccly_l = cby + 2.0f;
      float cclx_r = cbx + 20.0f;
      float ccly_r = cby + 2.0f;
      
      if ((int)(crab_x * 0.25f) % 2 == 0) {
        ccly_l -= 4.0f; // Raise left claw
      } else {
        ccly_r -= 4.0f; // Raise right claw
      }
      
      // Position eyes
      float cex_l = cbx + 5.0f;
      float cey_l = cby + 3.0f;
      float cex_r = cbx + 15.0f;
      float cey_r = cby + 3.0f;
      
      lv_obj_set_pos(crab_body_obj, (int)cbx, (int)cby);
      lv_obj_set_pos(crab_claw_l_obj, (int)cclx_l, (int)ccly_l);
      lv_obj_set_pos(crab_claw_r_obj, (int)cclx_r, (int)ccly_r);
      lv_obj_set_pos(crab_eye_l_obj, (int)cex_l, (int)cey_l);
      lv_obj_set_pos(crab_eye_r_obj, (int)cex_r, (int)cey_r);
    }
    
    // 5.5 Update Jellyfish (bobbing & drifting)
    jelly.phase += 0.05f;
    jelly.x += jelly.speed_x;
    jelly.y = jelly.base_y + std::sin(jelly.phase) * 15.0f;
    
    // Horizontal drift limits
    if (jelly.x < 20.0f) {
      jelly.x = 20.0f;
      jelly.speed_x = -jelly.speed_x;
    } else if (jelly.x > 440.0f) {
      jelly.x = 440.0f;
      jelly.speed_x = -jelly.speed_x;
    }
    
    // Vertical drift limits (slowly drifts base_y up and down)
    jelly.base_y += jelly.speed_y;
    if (jelly.base_y < 40.0f) {
      jelly.base_y = 40.0f;
      jelly.speed_y = -jelly.speed_y;
    } else if (jelly.base_y > 180.0f) {
      jelly.base_y = 180.0f;
      jelly.speed_y = -jelly.speed_y;
    }
    
    // Position bell
    lv_obj_set_pos(jelly.bell_obj, (int)jelly.x, (int)jelly.y);
    
    // Swaying tentacles
    {
      float sway_sin = std::sin(jelly.phase * 1.5f);
      float sway_cos = std::cos(jelly.phase * 1.5f);
      
      lv_obj_set_pos(jelly.tentacle_l, (int)(jelly.x + 4.0f + sway_sin * 3.0f), (int)(jelly.y + 12.0f));
      lv_obj_set_pos(jelly.tentacle_c, (int)(jelly.x + 10.0f), (int)(jelly.y + 14.0f + sway_cos * 2.0f));
      lv_obj_set_pos(jelly.tentacle_r, (int)(jelly.x + 16.0f - sway_sin * 3.0f), (int)(jelly.y + 12.0f));
    }
    
    // 6. Update Bubbles
    if (bubbler_on && (rand() % 100 < 18)) {
      // Spawn bubble at the treasure chest (located at x=350, y=300)
      for (auto& b : bubbles) {
        if (!b.active) {
          b.active = true;
          b.x = 358.0f + (rand() % 14);
          b.y = 295.0f;
          b.speed = 1.2f + ((rand() % 100) / 100.0f) * 1.6f;
          lv_obj_clear_flag(b.obj, LV_OBJ_FLAG_HIDDEN);
          break;
        }
      }
    }
    
    for (auto& b : bubbles) {
      if (b.active) {
        b.y -= b.speed;
        b.x += std::sin(b.y * 0.07f) * 0.7f; // sway sideways
        lv_obj_set_pos(b.obj, (int)b.x, (int)b.y);
        
        if (b.y < 12) {
          b.active = false;
          lv_obj_add_flag(b.obj, LV_OBJ_FLAG_HIDDEN);
        }
      }
    }
    
    // 7. Update Food
    for (auto& fd : food) {
      if (fd.active) {
        fd.y += fd.speed;
        fd.x += std::sin(fd.y * 0.05f) * 0.4f;
        lv_obj_set_pos(fd.obj, (int)fd.x, (int)fd.y);
        
        // Dissolve at bottom
        if (fd.y > 292.0f) {
          fd.active = false;
          lv_obj_add_flag(fd.obj, LV_OBJ_FLAG_HIDDEN);
          continue;
        }
        
        // Collision detection with fish
        for (auto& f : fish) {
          float center_x = f.x + 20.0f;
          float center_y = f.y + 10.0f;
          float dx = fd.x - center_x;
          float dy = fd.y - center_y;
          float dist = std::sqrt(dx * dx + dy * dy);
          
          float eat_radius = (f.depth == 0) ? 22.0f : ((f.depth == 1) ? 16.0f : 12.0f);
          if (dist < eat_radius) {
            // Food eaten!
            fd.active = false;
            lv_obj_add_flag(fd.obj, LV_OBJ_FLAG_HIDDEN);
            fed_count++;
            
            // Reaction
            if (f.type == 2) {
              f.puff_timer = 50; // Blowfish puffs up for 2.5s
            } else if (f.type == 0) {
              f.vx = -f.vx * 1.4f; // Goldfish darts forward happily
            }
            
            // Publish updated feed count sensor back to Home Assistant
            if (fed_sensor != nullptr) {
              fed_sensor->publish_state(fed_count);
            }
            break;
          }
        }
      }
    }
    
    // 8. Update Shark
    if (shark_active) {
      shark_x += shark_vx;
      
      // Position body parts dynamically
      float bx = 0.0f, tx = 0.0f, fx = 0.0f, ex = 0.0f;
      float by = shark_y, ty = shark_y + 5.0f, fy = shark_y - 12.0f, ey = shark_y + 8.0f;
      
      if (shark_vx > 0) {
        bx = shark_x + 18.0f;
        tx = shark_x;
        fx = bx + 30.0f;
        ex = bx + 70.0f;
      } else {
        bx = shark_x;
        tx = shark_x + 86.0f;
        fx = bx + 42.0f;
        ex = bx + 14.0f;
      }
      
      lv_obj_set_pos(shark_body_obj, (int)bx, (int)by);
      lv_obj_set_pos(shark_tail_obj, (int)tx, (int)ty);
      lv_obj_set_pos(shark_fin_obj, (int)fx, (int)fy);
      lv_obj_set_pos(shark_eye_obj, (int)ex, (int)ey);
      
      // Panic nearby fish
      for (auto& f : fish) {
        float dx = f.x - shark_x;
        float dy = f.y - shark_y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist < 130.0f) {
          // Direct fish away from shark
          f.vx = (dx > 0) ? 2.4f : -2.4f;
          f.vy = (dy > 0) ? 1.2f : -1.2f;
          panic_timer = std::max(panic_timer, 25);
        }
      }
      
      // Check offscreen exit
      if ((shark_vx > 0.0f && shark_x > 490.0f) || (shark_vx < 0.0f && shark_x < -180.0f)) {
        shark_active = false;
        lv_obj_add_flag(shark_body_obj, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(shark_tail_obj, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(shark_fin_obj, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(shark_eye_obj, LV_OBJ_FLAG_HIDDEN);
      }
    }
  }
  
  void trigger_alert(std::string message, int duration_seconds) {
    alert_active = true;
    alert_msg = message;
    alert_start_time = millis();
    alert_duration_ms = duration_seconds * 1000;
    
    // Set label text
    if (alert_label_obj != nullptr) {
      lv_label_set_text(alert_label_obj, message.c_str());
      // Center the label text roughly in the banner
      int label_len = message.length();
      int start_x = 240 - (label_len * 4); // Montserrat 14 width factor
      if (start_x < 90) start_x = 90;
      lv_obj_set_x(alert_label_obj, start_x);
    }
    
    // Show components
    if (alert_banner_obj != nullptr) lv_obj_clear_flag(alert_banner_obj, LV_OBJ_FLAG_HIDDEN);
    if (alert_label_obj != nullptr) lv_obj_clear_flag(alert_label_obj, LV_OBJ_FLAG_HIDDEN);
    if (alert_light_l_obj != nullptr) lv_obj_clear_flag(alert_light_l_obj, LV_OBJ_FLAG_HIDDEN);
    if (alert_light_r_obj != nullptr) lv_obj_clear_flag(alert_light_r_obj, LV_OBJ_FLAG_HIDDEN);
    
    // Startle fish (make them scatter immediately)
    for (auto& f : fish) {
      float angle = ((rand() % 360) / 180.0f) * 3.14159f;
      float speed = 4.0f + (rand() % 100) / 33.0f; // Fast scatter
      f.vx = cos(angle) * speed;
      f.vy = sin(angle) * speed;
      f.facing_right = f.vx > 0;
    }
  }
  
  void clear_alert() {
    alert_active = false;
    alert_msg = "";
    
    // Hide components
    if (alert_banner_obj != nullptr) lv_obj_add_flag(alert_banner_obj, LV_OBJ_FLAG_HIDDEN);
    if (alert_label_obj != nullptr) lv_obj_add_flag(alert_label_obj, LV_OBJ_FLAG_HIDDEN);
    if (alert_light_l_obj != nullptr) lv_obj_add_flag(alert_light_l_obj, LV_OBJ_FLAG_HIDDEN);
    if (alert_light_r_obj != nullptr) lv_obj_add_flag(alert_light_r_obj, LV_OBJ_FLAG_HIDDEN);
  }
  
  void feed() {
    // Spawns 3 food flakes at the top at random x positions
    int count = 0;
    for (auto& fd : food) {
      if (!fd.active) {
        fd.active = true;
        fd.x = 30.0f + (rand() % 410);
        fd.y = 15.0f;
        fd.speed = 0.8f + ((rand() % 100) / 100.0f) * 0.7f;
        lv_obj_clear_flag(fd.obj, LV_OBJ_FLAG_HIDDEN);
        count++;
        if (count >= 3) break;
      }
    }
  }
  
  void tap(int tx, int ty) {
    panic_timer = 90; // scare fish for 4.5 seconds
    
    // Ripple visual cue
    lv_obj_set_pos(tap_obj, tx - 15, ty - 10);
    lv_label_set_text(tap_obj, "*DINK*");
    lv_obj_clear_flag(tap_obj, LV_OBJ_FLAG_HIDDEN);
    tap_timer = 18; // show for 0.9s
    
    // Calculate scatter directions away from tap point
    for (auto& f : fish) {
      float dx = f.x - tx;
      float dy = f.y - ty;
      float len = std::sqrt(dx * dx + dy * dy);
      if (len > 0.0f) {
        f.vx = (dx / len) * 2.8f;
        f.vy = (dy / len) * 1.6f;
      } else {
        f.vx = 2.0f;
        f.vy = 1.0f;
      }
    }
  }
  
  void spawn_shark() {
    if (shark_active) return;
    shark_active = true;
    lv_obj_clear_flag(shark_body_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(shark_tail_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(shark_fin_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(shark_eye_obj, LV_OBJ_FLAG_HIDDEN);
    
    // Choose side to spawn and head direction
    if (rand() % 2 == 0) {
      shark_x = -170.0f;
      shark_vx = 2.4f;
    } else {
      shark_x = 490.0f;
      shark_vx = -2.4f;
    }
    shark_y = 50.0f + (rand() % 130);
    
    // Position body parts initially
    float bx = 0.0f, tx = 0.0f, fx = 0.0f, ex = 0.0f;
    float by = shark_y, ty = shark_y + 5.0f, fy = shark_y - 12.0f, ey = shark_y + 8.0f;
    
    if (shark_vx > 0) {
      bx = shark_x + 18.0f;
      tx = shark_x;
      fx = bx + 30.0f;
      ex = bx + 70.0f;
    } else {
      bx = shark_x;
      tx = shark_x + 86.0f;
      fx = bx + 42.0f;
      ex = bx + 14.0f;
    }
    
    lv_obj_set_pos(shark_body_obj, (int)bx, (int)by);
    lv_obj_set_pos(shark_tail_obj, (int)tx, (int)ty);
    lv_obj_set_pos(shark_fin_obj, (int)fx, (int)fy);
    lv_obj_set_pos(shark_eye_obj, (int)ex, (int)ey);
    
    // Immediate panic
    panic_timer = 130;
  }
  
  void set_disco(bool on) {
    disco_mode = on;
  }
  
  void set_bubbler(bool on) {
    bubbler_on = on;
    // Clear any active bubbles if turned off
    if (!on) {
      for (auto& b : bubbles) {
        b.active = false;
        lv_obj_add_flag(b.obj, LV_OBJ_FLAG_HIDDEN);
      }
    }
  }
};

inline Aquarium global_aquarium;

