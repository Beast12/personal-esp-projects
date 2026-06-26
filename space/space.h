#pragma once

#include "esphome.h"
#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>

// Space Actor structure
struct ActorState {
  lv_obj_t* obj;
  float x;
  float y;
  float vx;
  float vy;
  int width;
  int height;
  bool facing_right;
  int state;         // 0 = normal, 1 = panic, 2 = chasing crystal
  int type;          // 0 = rocket, 1 = ufo, 2 = astronaut, 3 = satellite
  int boost_timer;   // rocket thruster burst frames
  int tumble_timer;  // astronaut rotation frames
  int depth;         // 0 = FG, 1 = MG, 2 = BG
  lv_color_t base_color;
};

// Twinkling Star structure
struct StarState {
  lv_obj_t* obj;
  float x;
  float y;
  int depth;
  float phase;
  float speed;
};

// Cosmic Dust particle
struct DustState {
  lv_obj_t* obj;
  float x;
  float y;
  float vx;
  float vy;
  float speed;
  bool active;
};

// Energy Crystal / Space Debris
struct CrystalState {
  lv_obj_t* obj;
  float x;
  float y;
  float speed;
  bool active;
};

class Space {
 public:
  std::vector<ActorState> actors;
  std::vector<StarState> stars;
  std::vector<DustState> dust;
  std::vector<CrystalState> crystals;

  lv_obj_t* bg_obj = nullptr;
  lv_obj_t* monster_obj = nullptr;
  lv_obj_t* warp_obj = nullptr;

  // Planet components
  lv_obj_t* jupiter_obj = nullptr;
  lv_obj_t* saturn_body_obj = nullptr;
  lv_obj_t* saturn_ring_obj = nullptr;
  lv_obj_t* earth_obj = nullptr;
  lv_obj_t* mars_obj = nullptr;

  // Space Station components
  lv_obj_t* station_core_obj = nullptr;
  lv_obj_t* station_solar_l_obj = nullptr;
  lv_obj_t* station_solar_r_obj = nullptr;

  // Wormhole / Portal components
  lv_obj_t* portal_core_obj = nullptr;
  lv_obj_t* portal_ring_obj = nullptr;

  // Alert components
  lv_obj_t* alert_banner_obj = nullptr;
  lv_obj_t* alert_label_obj = nullptr;
  lv_obj_t* alert_light_l_obj = nullptr;
  lv_obj_t* alert_light_r_obj = nullptr;

  // State flags
  bool alert_active = false;
  std::string alert_msg = "";
  uint32_t alert_start_time = 0;
  uint32_t alert_duration_ms = 0;

  bool portal_active = true;
  bool supernova_mode = false;
  int supernova_hue = 0;

  int warp_timer = 0;
  int panic_timer = 0;
  int collected_count = 0;

  float monster_x = -180.0f;
  float monster_y = 120.0f;
  float monster_vx = 0.0f;
  bool monster_active = false;

  void setup(
      lv_obj_t* bg,
      std::vector<lv_obj_t*> actor_objs,
      std::vector<lv_obj_t*> star_objs,
      std::vector<lv_obj_t*> dust_objs,
      std::vector<lv_obj_t*> crystal_objs,
      std::vector<lv_obj_t*> planet_objs,       // [jupiter, saturn_body, saturn_ring, earth, mars]
      std::vector<lv_obj_t*> station_objs,      // [core, solar_l, solar_r]
      std::vector<lv_obj_t*> portal_objs,       // [core, ring]
      std::vector<lv_obj_t*> alert_objs,        // [banner, label, light_l, light_r]
      lv_obj_t* monster,
      lv_obj_t* warp,
      esphome::font::Font* font_large,
      esphome::font::Font* font_medium,
      esphome::font::Font* font_small) {

    bg_obj = bg;
    monster_obj = monster;
    warp_obj = warp;

    // Planets
    jupiter_obj = planet_objs[0];
    saturn_body_obj = planet_objs[1];
    saturn_ring_obj = planet_objs[2];
    earth_obj = planet_objs[3];
    mars_obj = planet_objs[4];

    // Space Station
    station_core_obj = station_objs[0];
    station_solar_l_obj = station_objs[1];
    station_solar_r_obj = station_objs[2];

    // Wormhole / Portal
    portal_core_obj = portal_objs[0];
    portal_ring_obj = portal_objs[1];

    // Alert overlay
    alert_banner_obj = alert_objs[0];
    alert_label_obj = alert_objs[1];
    alert_light_l_obj = alert_objs[2];
    alert_light_r_obj = alert_objs[3];

    lv_obj_add_flag(alert_banner_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(alert_label_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(alert_light_l_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(alert_light_r_obj, LV_OBJ_FLAG_HIDDEN);

    // Styling Planets with premium gradients
    // Jupiter: Red-Orange banded gradient
    lv_obj_set_style_bg_grad_dir(jupiter_obj, LV_GRAD_DIR_VER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(jupiter_obj, lv_color_hex(0xE67E22), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(jupiter_obj, lv_color_hex(0x943126), LV_PART_MAIN);

    // Saturn Body: Golden Yellow
    lv_obj_set_style_bg_color(saturn_body_obj, lv_color_hex(0xF4D03F), LV_PART_MAIN);
    // Saturn Ring: Sleek semi-transparent pastel orange-brown ellipse
    lv_obj_set_style_bg_color(saturn_ring_obj, lv_color_hex(0xD35400), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(saturn_ring_obj, 180, LV_PART_MAIN);

    // Earth: Blue and green-accented sphere
    lv_obj_set_style_bg_grad_dir(earth_obj, LV_GRAD_DIR_VER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(earth_obj, lv_color_hex(0x3498DB), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(earth_obj, lv_color_hex(0x27AE60), LV_PART_MAIN);

    // Mars: Dusty Red-Orange
    lv_obj_set_style_bg_grad_dir(mars_obj, LV_GRAD_DIR_HOR, LV_PART_MAIN);
    lv_obj_set_style_bg_color(mars_obj, lv_color_hex(0xE74C3C), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(mars_obj, lv_color_hex(0xBA4A00), LV_PART_MAIN);

    // Space Station core & solar panels
    lv_obj_set_style_bg_color(station_core_obj, lv_color_hex(0xBDC3C7), LV_PART_MAIN);
    lv_obj_set_style_bg_color(station_solar_l_obj, lv_color_hex(0x2E86C1), LV_PART_MAIN);
    lv_obj_set_style_bg_color(station_solar_r_obj, lv_color_hex(0x2E86C1), LV_PART_MAIN);
    lv_obj_set_style_border_color(station_solar_l_obj, lv_color_hex(0x1B4F72), LV_PART_MAIN);
    lv_obj_set_style_border_color(station_solar_r_obj, lv_color_hex(0x1B4F72), LV_PART_MAIN);
    lv_obj_set_style_border_width(station_solar_l_obj, 1, LV_PART_MAIN);
    lv_obj_set_style_border_width(station_solar_r_obj, 1, LV_PART_MAIN);

    // Wormhole Portal
    // Core: vibrant magenta-purple gradient
    lv_obj_set_style_bg_grad_dir(portal_core_obj, LV_GRAD_DIR_VER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(portal_core_obj, lv_color_hex(0x8E44AD), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(portal_core_obj, lv_color_hex(0xFD79A8), LV_PART_MAIN);
    // Outer Ring: glowing neon cyan
    lv_obj_set_style_border_color(portal_ring_obj, lv_color_hex(0x00CEC9), LV_PART_MAIN);
    lv_obj_set_style_border_width(portal_ring_obj, 2, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(portal_ring_obj, 0, LV_PART_MAIN); // hollow center

    // Initialize 16 stars with different coordinates, depths, and phases
    // Depth: 0 = Foreground (bright, fast), 1 = Midground, 2 = Background (dim, slow)
    for (size_t i = 0; i < star_objs.size(); ++i) {
      float sx = 10.0f + (rand() % 460);
      float sy = 10.0f + (rand() % 300);
      int depth = rand() % 3;
      float phase = ((rand() % 100) / 100.0f) * 6.28f;
      float speed = 1.0f + ((rand() % 100) / 100.0f) * 2.0f;
      
      stars.push_back({star_objs[i], sx, sy, depth, phase, speed});

      // Style stars based on depth layer
      if (depth == 0) {
        lv_obj_set_style_text_font(star_objs[i], font_medium, LV_PART_MAIN);
        lv_obj_set_style_text_color(star_objs[i], lv_color_hex(0xFFFFFF), LV_PART_MAIN);
      } else if (depth == 1) {
        lv_obj_set_style_text_font(star_objs[i], font_small, LV_PART_MAIN);
        lv_obj_set_style_text_color(star_objs[i], lv_color_hex(0xFCF3CF), LV_PART_MAIN); // pale yellow
      } else {
        lv_obj_set_style_text_font(star_objs[i], font_tiny, LV_PART_MAIN);
        lv_obj_set_style_text_color(star_objs[i], lv_color_hex(0x7F8C8D), LV_PART_MAIN); // dim grey
      }
      lv_obj_set_pos(star_objs[i], (int)sx, (int)sy);
    }

    // Initialize Cosmic Dust (8 items)
    for (auto* d : dust_objs) {
      dust.push_back({d, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, false});
      lv_obj_add_flag(d, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_style_text_color(d, lv_color_hex(0x76D7C4), LV_PART_MAIN); // Cyan-green dust
    }

    // Initialize Energy Crystals (5 items)
    for (auto* c : crystal_objs) {
      crystals.push_back({c, 0.0f, 0.0f, 0.0f, false});
      lv_obj_add_flag(c, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_style_text_color(c, lv_color_hex(0xBB8FCE), LV_PART_MAIN); // glowing purple crystals
    }

    // Configure Space Monster (comet/alien boss)
    lv_obj_set_style_text_color(monster_obj, lv_color_hex(0x9B59B6), LV_PART_MAIN); // Purple boss
    lv_obj_add_flag(monster_obj, LV_OBJ_FLAG_HIDDEN);

    // Configure Warp touch indicator
    lv_obj_set_style_text_color(warp_obj, lv_color_hex(0x00FFCC), LV_PART_MAIN);
    lv_obj_add_flag(warp_obj, LV_OBJ_FLAG_HIDDEN);

    // Initialize Space Actors
    // Types: 0 = Rocket, 1 = UFO, 2 = Astronaut, 3 = Satellite
    struct ActorInit {
      int type;
      int depth;
      lv_color_t color;
      int width;
      int height;
      float vx;
      float vy;
    };

    std::vector<ActorInit> init_data = {
      // Rockets (type 0)
      {0, 0, lv_color_hex(0xECF0F1), 60, 20, 1.4f, 0.2f},   // FG
      {0, 1, lv_color_hex(0xBDC3C7), 44, 16, -1.0f, -0.1f}, // MG
      {0, 2, lv_color_hex(0x7F8C8D), 32, 12, 0.7f, 0.1f},   // BG

      // UFOs (type 1)
      {1, 0, lv_color_hex(0x2ECC71), 48, 20, -1.8f, -0.3f}, // FG
      {1, 1, lv_color_hex(0x27AE60), 36, 14, 1.2f, 0.2f},   // MG
      {1, 1, lv_color_hex(0x27AE60), 36, 14, -1.1f, -0.2f}, // MG
      {1, 2, lv_color_hex(0x196F3D), 24, 10, 0.8f, 0.1f},   // BG

      // Astronauts (type 2)
      {2, 0, lv_color_hex(0xF5EEF8), 24, 20, 0.3f, 0.2f},   // FG
      {2, 1, lv_color_hex(0xD2B4DE), 18, 16, -0.2f, -0.4f}, // MG
      {2, 2, lv_color_hex(0x9B59B6), 14, 12, 0.1f, 0.3f},   // BG

      // Satellites (type 3)
      {3, 1, lv_color_hex(0xF39C12), 48, 16, -0.5f, 0.0f},  // MG
      {3, 2, lv_color_hex(0xB7950B), 32, 12, 0.3f, 0.0f}    // BG
    };

    for (size_t i = 0; i < actor_objs.size() && i < init_data.size(); ++i) {
      auto& data = init_data[i];
      float start_x = 30.0f + (rand() % 360);
      float start_y = 30.0f + (rand() % 160);
      actors.push_back({
        actor_objs[i],
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
        0,
        data.depth,
        data.color
      });

      // Depth based font sizing
      if (data.depth == 0) {
        lv_obj_set_style_text_font(actor_objs[i], font_large, LV_PART_MAIN);
      } else if (data.depth == 1) {
        lv_obj_set_style_text_font(actor_objs[i], font_medium, LV_PART_MAIN);
      } else {
        lv_obj_set_style_text_font(actor_objs[i], font_small, LV_PART_MAIN);
      }
      lv_obj_set_style_text_color(actor_objs[i], data.color, LV_PART_MAIN);
    }

    // Red Alert Styling defaults
    lv_obj_set_style_bg_color(alert_banner_obj, lv_color_hex(0xC0392B), LV_PART_MAIN);
    lv_obj_set_style_border_color(alert_banner_obj, lv_color_hex(0x00CEC9), LV_PART_MAIN); // cyan border
    lv_obj_set_style_text_color(alert_label_obj, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_bg_color(alert_light_l_obj, lv_color_hex(0xE74C3C), LV_PART_MAIN);
    lv_obj_set_style_bg_color(alert_light_r_obj, lv_color_hex(0x00CEC9), LV_PART_MAIN);
  }

  void update(esphome::sensor::Sensor* collected_sensor) {
    float t_sec = millis() / 1000.0f;

    // 1. Supernova (Nebula Storm) Mode color cycling
    if (supernova_mode) {
      supernova_hue = (supernova_hue + 2) % 360;
      lv_color_t bg_color = lv_color_hsv_to_rgb(supernova_hue, 75, 14); // rich neon space hues
      lv_obj_set_style_bg_grad_dir(bg_obj, LV_GRAD_DIR_NONE, LV_PART_MAIN);
      lv_obj_set_style_bg_color(bg_obj, bg_color, LV_PART_MAIN);

      // Rainbow cycle actors
      for (size_t i = 0; i < actors.size(); ++i) {
        int act_hue = (supernova_hue + i * 30) % 360;
        lv_obj_set_style_text_color(actors[i].obj, lv_color_hsv_to_rgb(act_hue, 90, 95), LV_PART_MAIN);
      }
    } else {
      // Normal: Deep Space Gradient (Dark Purple to Solid Black)
      lv_obj_set_style_bg_grad_dir(bg_obj, LV_GRAD_DIR_VER, LV_PART_MAIN);
      lv_obj_set_style_bg_color(bg_obj, lv_color_hex(0x0E091B), LV_PART_MAIN);
      lv_obj_set_style_bg_grad_color(bg_obj, lv_color_hex(0x030307), LV_PART_MAIN);

      for (auto& act : actors) {
        lv_obj_set_style_text_color(act.obj, act.base_color, LV_PART_MAIN);
      }
    }

    // 2. Decrement Timers
    if (panic_timer > 0) panic_timer--;
    if (warp_timer > 0) {
      warp_timer--;
      if (warp_timer == 0) {
        lv_obj_add_flag(warp_obj, LV_OBJ_FLAG_HIDDEN);
      }
    }

    // 3. Twinkle Stars
    for (auto& s : stars) {
      // Periodic brightness wave
      float brightness = std::sin(t_sec * s.speed + s.phase) * 110.0f + 145.0f;
      if (supernova_mode) {
        // rapid flashing
        brightness = std::sin(t_sec * s.speed * 3.0f + s.phase) * 120.0f + 135.0f;
      }
      lv_obj_set_style_text_opa(s.obj, (uint8_t)std::clamp(brightness, 0.0f, 255.0f), LV_PART_MAIN);
    }

    // 4. Bob Planets and Space Station (Orbital drift)
    float jup_y = 65.0f + std::sin(t_sec * 0.4f) * 3.0f;
    lv_obj_set_y(jupiter_obj, (int)jup_y);

    float sat_y = 45.0f + std::cos(t_sec * 0.3f) * 2.0f;
    lv_obj_set_y(saturn_body_obj, (int)sat_y);
    lv_obj_set_y(saturn_ring_obj, (int)sat_y - 2);

    float ear_y = 190.0f + std::sin(t_sec * 0.6f) * 4.0f;
    lv_obj_set_y(earth_obj, (int)ear_y);

    float mar_y = 110.0f + std::cos(t_sec * 0.7f) * 3.0f;
    lv_obj_set_y(mars_obj, (int)mar_y);

    float stat_y = 230.0f + std::sin(t_sec * 0.25f) * 1.5f;
    lv_obj_set_y(station_core_obj, (int)stat_y);
    lv_obj_set_y(station_solar_l_obj, (int)stat_y + 11);
    lv_obj_set_y(station_solar_r_obj, (int)stat_y + 11);

    // 5. Wormhole Pulse
    float portal_pulse = std::sin(t_sec * 3.5f) * 3.0f;
    if (portal_active) {
      lv_obj_set_size(portal_ring_obj, 44 + (int)portal_pulse, 44 + (int)portal_pulse);
      lv_obj_set_pos(portal_ring_obj, 368 - (int)(portal_pulse/2), 238 - (int)(portal_pulse/2));
    } else {
      lv_obj_set_size(portal_ring_obj, 44, 44);
      lv_obj_set_pos(portal_ring_obj, 368, 238);
    }

    // 6. Update Alert Warning Overlay
    if (alert_active) {
      if (alert_duration_ms > 0 && (millis() - alert_start_time > alert_duration_ms)) {
        clear_alert();
      } else {
        // Red alert pulse: mix Crimson (0xC0392B) with Deep Purple (0x2E0854)
        float pulse = (std::sin(t_sec * 6.28f * 1.5f) + 1.0f) / 2.0f;
        lv_color_t c = lv_color_mix(lv_color_hex(0xC0392B), lv_color_hex(0x2E0854), (uint8_t)(pulse * 255));
        lv_obj_set_style_bg_color(alert_banner_obj, c, LV_PART_MAIN);

        // Flash Red and Cyan sirens at 4Hz
        bool blink = ((int)(t_sec * 4.0f) % 2) == 0;
        if (blink) {
          lv_obj_set_style_bg_color(alert_light_l_obj, lv_color_hex(0xE74C3C), LV_PART_MAIN); // Red
          lv_obj_set_style_bg_color(alert_light_r_obj, lv_color_hex(0x00CEC9), LV_PART_MAIN); // Cyan
        } else {
          lv_obj_set_style_bg_color(alert_light_l_obj, lv_color_hex(0x00CEC9), LV_PART_MAIN); // Cyan
          lv_obj_set_style_bg_color(alert_light_r_obj, lv_color_hex(0xE74C3C), LV_PART_MAIN); // Red
        }
      }
    }

    // 7. Update Space Actors
    for (auto& act : actors) {
      float speed_mult = 1.0f;
      if (panic_timer > 0) {
        speed_mult = 3.2f;
      } else if (alert_active) {
        speed_mult = 2.6f; // Emergency alert speed
      } else if (act.state == 2) {
        speed_mult = 2.0f; // chasing energy crystal
      }

      // Debris/crystal collection logic
      if (panic_timer == 0) {
        float nearest_dist = 99999.0f;
        int target_crystal_idx = -1;

        for (size_t i = 0; i < crystals.size(); ++i) {
          if (crystals[i].active) {
            float dx = crystals[i].x - act.x;
            float dy = crystals[i].y - act.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist < nearest_dist) {
              nearest_dist = dist;
              target_crystal_idx = (int)i;
            }
          }
        }

        if (target_crystal_idx != -1) {
          act.state = 2; // Chase energy crystal
          float cx = crystals[target_crystal_idx].x;
          float cy = crystals[target_crystal_idx].y;
          float dx = cx - act.x;
          float dy = cy - act.y;
          float len = std::sqrt(dx * dx + dy * dy);
          if (len > 0) {
            act.vx = (act.vx * 0.88f) + (dx / len * 1.5f * 0.12f);
            act.vy = (act.vy * 0.88f) + (dy / len * 1.1f * 0.12f);
          }
        } else {
          act.state = 0; // standard drift
        }
      } else {
        act.state = 1; // Panicked
      }

      // Add emergency scattering jitter when Red Alert is active
      if (alert_active && (rand() % 100 < 10)) {
        act.vx += ((rand() % 100) / 50.0f - 1.0f) * 0.9f;
        act.vy += ((rand() % 100) / 50.0f - 1.0f) * 0.7f;
      }

      // Depth based parallax speed
      float depth_mult = (act.depth == 0) ? 1.0f : ((act.depth == 1) ? 0.72f : 0.48f);
      
      // Astronauts float slower in zero gravity
      if (act.type == 2) {
        depth_mult *= 0.45f;
      }

      // Apply rocket boosters
      float boost_speed = 1.0f;
      if (act.type == 0) {
        if (act.boost_timer > 0) {
          act.boost_timer--;
          boost_speed = 2.4f; // combustion speed boost!
        } else if (rand() % 250 < 1) {
          act.boost_timer = 30 + rand() % 30; // initiate 1.5s - 3s boost
        }
      }

      act.x += act.vx * speed_mult * depth_mult * boost_speed;
      act.y += act.vy * speed_mult * depth_mult * boost_speed;

      // Wrap coordinates
      int max_x = 480 - act.width;
      int max_y = 270;
      int min_y = 15;

      if (act.x < 0) {
        act.x = 0;
        act.vx = -act.vx;
        act.facing_right = true;
      } else if (act.x > max_x) {
        act.x = (float)max_x;
        act.vx = -act.vx;
        act.facing_right = false;
      }

      if (act.y < min_y) {
        act.y = (float)min_y;
        act.vy = -act.vy;
      } else if (act.y > max_y) {
        act.y = (float)max_y;
        act.vy = -act.vy;
      }

      // Organic drifting changes
      if (rand() % 100 < 4) {
        float dvx = ((rand() % 200 - 100) / 100.0f) * 0.35f;
        float dvy = ((rand() % 200 - 100) / 100.0f) * 0.25f;
        act.vx += dvx;
        act.vy += dvy;

        // Clamp speed
        float speed = std::sqrt(act.vx * act.vx + act.vy * act.vy);
        float max_spd = (act.type == 1) ? 2.0f : 1.2f; // UFOs are faster
        if (act.type == 3) max_spd = 0.5f;             // Satellites are very slow
        float min_spd = 0.2f;

        if (speed > max_spd) {
          act.vx = (act.vx / speed) * max_spd;
          act.vy = (act.vy / speed) * max_spd;
        } else if (speed < min_spd) {
          act.vx = (act.vx / speed) * min_spd;
          act.vy = (act.vy / speed) * min_spd;
        }

        act.facing_right = (act.vx > 0);
      }

      // UFO hyper-jumps
      if (act.type == 1 && rand() % 400 < 1) {
        act.x += (rand() % 60 - 30);
        act.y += (rand() % 40 - 20);
        act.x = std::clamp(act.x, 10.0f, 400.0f);
        act.y = std::clamp(act.y, 20.0f, 260.0f);
      }

      // Write pos
      lv_obj_set_pos(act.obj, (int)act.x, (int)act.y);

      // Animate text character representations
      if (act.type == 0) { // Rockets
        if (act.facing_right) {
          if (act.boost_timer > 0) {
            lv_label_set_text(act.obj, "|=I=>~"); // Thruster trail
          } else {
            lv_label_set_text(act.obj, "|=I=>");
          }
        } else {
          if (act.boost_timer > 0) {
            lv_label_set_text(act.obj, "~<=I=|");
          } else {
            lv_label_set_text(act.obj, "<=I=|");
          }
        }
      } else if (act.type == 1) { // UFOs
        lv_label_set_text(act.obj, "[-o-]");
      } else if (act.type == 2) { // Astronaut tumbling
        act.tumble_timer = (act.tumble_timer + 1) % 80;
        if (act.tumble_timer < 20) {
          lv_label_set_text(act.obj, "[o]");
        } else if (act.tumble_timer < 40) {
          lv_label_set_text(act.obj, "\\o/");
        } else if (act.tumble_timer < 60) {
          lv_label_set_text(act.obj, "_o_");
        } else {
          lv_label_set_text(act.obj, "/o\\");
        }
      } else if (act.type == 3) { // Satellites (beacons)
        act.tumble_timer = (act.tumble_timer + 1) % 40;
        if (act.tumble_timer < 20) {
          lv_label_set_text(act.obj, "o-I-o");
        } else {
          lv_label_set_text(act.obj, "o-*-o"); // active transmission
        }
      }
    }

    // 8. Update Cosmic Dust (Wormhole Portal ejections)
    if (portal_active && (rand() % 100 < 22)) {
      // Spawn dust particle at portal (x: 380, y: 250)
      for (auto& d : dust) {
        if (!d.active) {
          d.active = true;
          d.x = 380.0f;
          d.y = 250.0f;
          // Eject leftwards with spread
          float angle = 135.0f + (rand() % 90); // 135 to 225 deg
          float rad = angle * 3.14159f / 180.0f;
          d.speed = 1.0f + ((rand() % 100) / 100.0f) * 2.0f;
          d.vx = cos(rad) * d.speed;
          d.vy = sin(rad) * d.speed;
          lv_obj_clear_flag(d.obj, LV_OBJ_FLAG_HIDDEN);
          break;
        }
      }
    }

    for (auto& d : dust) {
      if (d.active) {
        d.x += d.vx;
        d.y += d.vy;
        lv_obj_set_pos(d.obj, (int)d.x, (int)d.y);

        // check bounds or portal distance to fade
        float dx = d.x - 380.0f;
        float dy = d.y - 250.0f;
        float dist = std::sqrt(dx*dx + dy*dy);
        if (dist > 180.0f || d.x < 10.0f || d.y < 10.0f || d.y > 300.0f) {
          d.active = false;
          lv_obj_add_flag(d.obj, LV_OBJ_FLAG_HIDDEN);
        }
      }
    }

    // 9. Update Energy Crystals
    for (auto& cry : crystals) {
      if (cry.active) {
        cry.y += cry.speed;
        cry.x += std::sin(cry.y * 0.05f) * 0.5f;
        lv_obj_set_pos(cry.obj, (int)cry.x, (int)cry.y);

        // dissolve at bottom of screen
        if (cry.y > 295.0f) {
          cry.active = false;
          lv_obj_add_flag(cry.obj, LV_OBJ_FLAG_HIDDEN);
          continue;
        }

        // Check collection collision
        for (auto& act : actors) {
          float cx = act.x + (act.width / 2);
          float cy = act.y + (act.height / 2);
          float dx = cry.x - cx;
          float dy = cry.y - cy;
          float dist = std::sqrt(dx*dx + dy*dy);

          float collect_radius = (act.depth == 0) ? 22.0f : ((act.depth == 1) ? 16.0f : 12.0f);
          if (dist < collect_radius) {
            cry.active = false;
            lv_obj_add_flag(cry.obj, LV_OBJ_FLAG_HIDDEN);
            collected_count++;

            // Reaction boosts
            if (act.type == 0) {
              act.boost_timer = 50; // rockets get long boost
            } else if (act.type == 1) {
              // UFO jumps instantly
              act.x += (rand() % 100 - 50);
            }

            if (collected_sensor != nullptr) {
              collected_sensor->publish_state(collected_count);
            }
            break;
          }
        }
      }
    }

    // 10. Update Space Monster / Asteroid Boss
    if (monster_active) {
      monster_x += monster_vx;
      lv_obj_set_pos(monster_obj, (int)monster_x, (int)monster_y);

      // Scare actors in path
      for (auto& act : actors) {
        float dx = act.x - monster_x;
        float dy = act.y - monster_y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist < 140.0f) {
          // Scatter away
          act.vx = (dx > 0) ? 2.5f : -2.5f;
          act.vy = (dy > 0) ? 1.4f : -1.4f;
          panic_timer = std::max(panic_timer, 30);
        }
      }

      // Exit screen detection
      if ((monster_vx > 0.0f && monster_x > 490.0f) || (monster_vx < 0.0f && monster_x < -190.0f)) {
        monster_active = false;
        lv_obj_add_flag(monster_obj, LV_OBJ_FLAG_HIDDEN);
      }
    }
  }

  void trigger_alert(std::string message, int duration_seconds) {
    alert_active = true;
    alert_msg = message;
    alert_start_time = millis();
    alert_duration_ms = duration_seconds * 1000;

    if (alert_label_obj != nullptr) {
      lv_label_set_text(alert_label_obj, message.c_str());
      int start_x = 240 - (message.length() * 4);
      if (start_x < 90) start_x = 90;
      lv_obj_set_x(alert_label_obj, start_x);
    }

    if (alert_banner_obj != nullptr) lv_obj_clear_flag(alert_banner_obj, LV_OBJ_FLAG_HIDDEN);
    if (alert_label_obj != nullptr) lv_obj_clear_flag(alert_label_obj, LV_OBJ_FLAG_HIDDEN);
    if (alert_light_l_obj != nullptr) lv_obj_clear_flag(alert_light_l_obj, LV_OBJ_FLAG_HIDDEN);
    if (alert_light_r_obj != nullptr) lv_obj_clear_flag(alert_light_r_obj, LV_OBJ_FLAG_HIDDEN);

    // Immediate scatter in panic
    for (auto& act : actors) {
      float angle = ((rand() % 360) / 180.0f) * 3.14159f;
      float speed = 4.2f + (rand() % 100) / 30.0f;
      act.vx = cos(angle) * speed;
      act.vy = sin(angle) * speed;
      act.facing_right = act.vx > 0;
    }
  }

  void clear_alert() {
    alert_active = false;
    alert_msg = "";

    if (alert_banner_obj != nullptr) lv_obj_add_flag(alert_banner_obj, LV_OBJ_FLAG_HIDDEN);
    if (alert_label_obj != nullptr) lv_obj_add_flag(alert_label_obj, LV_OBJ_FLAG_HIDDEN);
    if (alert_light_l_obj != nullptr) lv_obj_add_flag(alert_light_l_obj, LV_OBJ_FLAG_HIDDEN);
    if (alert_light_r_obj != nullptr) lv_obj_add_flag(alert_light_r_obj, LV_OBJ_FLAG_HIDDEN);
  }

  void drop_crystals() {
    int count = 0;
    for (auto& cry : crystals) {
      if (!cry.active) {
        cry.active = true;
        cry.x = 30.0f + (rand() % 410);
        cry.y = 15.0f;
        cry.speed = 0.9f + ((rand() % 100) / 100.0f) * 0.8f;
        lv_obj_clear_flag(cry.obj, LV_OBJ_FLAG_HIDDEN);
        count++;
        if (count >= 3) break;
      }
    }
  }

  void tap(int tx, int ty) {
    panic_timer = 90; // Scare actors for 4.5 seconds

    // Warp gravity ripple visual
    lv_obj_set_pos(warp_obj, tx - 20, ty - 10);
    lv_label_set_text(warp_obj, "*WARP*");
    lv_obj_clear_flag(warp_obj, LV_OBJ_FLAG_HIDDEN);
    warp_timer = 18; // 0.9 seconds

    for (auto& act : actors) {
      float dx = act.x - tx;
      float dy = act.y - ty;
      float len = std::sqrt(dx*dx + dy*dy);
      if (len > 0.0f) {
        act.vx = (dx / len) * 3.0f;
        act.vy = (dy / len) * 1.8f;
      } else {
        act.vx = 2.0f;
        act.vy = 1.0f;
      }
      act.facing_right = act.vx > 0;
    }
  }

  void spawn_monster() {
    if (monster_active) return;
    monster_active = true;
    lv_obj_clear_flag(monster_obj, LV_OBJ_FLAG_HIDDEN);

    if (rand() % 2 == 0) {
      monster_x = -180.0f;
      monster_vx = 2.6f;
      lv_label_set_text(monster_obj, "\\/\\(Oo)/\\/"); // Alien beast facing right
    } else {
      monster_x = 490.0f;
      monster_vx = -2.6f;
      lv_label_set_text(monster_obj, "\\/\\(oO)/\\/"); // Alien beast facing left
    }
    monster_y = 50.0f + (rand() % 130);
    lv_obj_set_pos(monster_obj, (int)monster_x, (int)monster_y);
    panic_timer = 130;
  }

  void set_supernova(bool on) {
    supernova_mode = on;
  }

  void set_portal(bool on) {
    portal_active = on;
    if (!on) {
      for (auto& d : dust) {
        d.active = false;
        lv_obj_add_flag(d.obj, LV_OBJ_FLAG_HIDDEN);
      }
    }
  }
};

inline Space global_space;
