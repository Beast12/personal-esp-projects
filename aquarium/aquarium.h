#pragma once

#include "esphome.h"
#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>

// Define structures for our aquarium actors
struct FishState {
  lv_obj_t* obj;
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

struct FoodState {
  lv_obj_t* obj;
  float x;
  float y;
  float speed;
  bool active;
};

struct SeaweedState {
  lv_obj_t* obj;
  float base_x;
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
  lv_obj_t* crab_obj = nullptr;
  lv_obj_t* shark_obj = nullptr;
  lv_obj_t* tap_obj = nullptr;
  lv_obj_t* castle_obj = nullptr;
  
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
      std::vector<lv_obj_t*> seaweed_objs,
      lv_obj_t* crab,
      lv_obj_t* shark,
      lv_obj_t* tap,
      lv_obj_t* castle,
      esphome::font::Font* font_large,
      esphome::font::Font* font_medium,
      esphome::font::Font* font_small) {
    
    bg_obj = bg;
    crab_obj = crab;
    shark_obj = shark;
    tap_obj = tap;
    castle_obj = castle;
    
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
    
    for (size_t i = 0; i < fish_objs.size() && i < init_data.size(); ++i) {
      auto& data = init_data[i];
      float start_x = 30.0f + (rand() % 360);
      float start_y = 40.0f + (rand() % 180);
      fish.push_back({
        fish_objs[i],
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
      
      // Set depth-based font sizing
      if (data.depth == 0) {
        lv_obj_set_style_text_font(fish_objs[i], font_large, LV_PART_MAIN);
      } else if (data.depth == 1) {
        lv_obj_set_style_text_font(fish_objs[i], font_medium, LV_PART_MAIN);
      } else {
        lv_obj_set_style_text_font(fish_objs[i], font_small, LV_PART_MAIN);
      }
      lv_obj_set_style_text_color(fish_objs[i], data.color, LV_PART_MAIN);
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
    
    // Initialize seaweed states (5 stalks) with beautiful vertical green gradients
    float base_xs[5] = {35.0f, 85.0f, 190.0f, 290.0f, 435.0f};
    for (size_t i = 0; i < seaweed_objs.size() && i < 5; ++i) {
      float speed = 1.0f + ((rand() % 100) / 100.0f) * 1.5f;
      float phase = ((rand() % 100) / 100.0f) * 6.28f;
      seaweed.push_back({seaweed_objs[i], base_xs[i], speed, phase});
      
      lv_obj_set_style_bg_grad_dir(seaweed_objs[i], LV_GRAD_DIR_VER, LV_PART_MAIN);
      lv_obj_set_style_bg_color(seaweed_objs[i], lv_color_hex(0x2ECC71), LV_PART_MAIN);      // Top (Lime Green)
      lv_obj_set_style_bg_grad_color(seaweed_objs[i], lv_color_hex(0x0A3015), LV_PART_MAIN); // Bottom (Dark Forest)
    }
    
    // Configure crab
    lv_obj_set_style_text_color(crab_obj, lv_color_hex(0xE74C3C), LV_PART_MAIN); // Coral Red
    
    // Configure shark
    lv_obj_set_style_text_color(shark_obj, lv_color_hex(0x7F8C8D), LV_PART_MAIN); // Grey
    lv_obj_add_flag(shark_obj, LV_OBJ_FLAG_HIDDEN);
    
    // Configure tap
    lv_obj_set_style_text_color(tap_obj, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_add_flag(tap_obj, LV_OBJ_FLAG_HIDDEN);
    
    // Configure castle ruins ornament (Ancient Stone Grey)
    lv_obj_set_style_text_color(castle_obj, lv_color_hex(0x7F8C8D), LV_PART_MAIN);
  }
  
  void update(esphome::sensor::Sensor* fed_sensor) {
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
        lv_obj_set_style_text_color(fish[i].obj, lv_color_hsv_to_rgb(fish_hue, 95, 95), LV_PART_MAIN);
      }
    } else {
      // Normal background: Deep deep water blue vertical gradient
      lv_obj_set_style_bg_grad_dir(bg_obj, LV_GRAD_DIR_VER, LV_PART_MAIN);
      lv_obj_set_style_bg_color(bg_obj, lv_color_hex(0x0A2B4E), LV_PART_MAIN); // Lighter top (Aqua-Blue)
      lv_obj_set_style_bg_grad_color(bg_obj, lv_color_hex(0x020814), LV_PART_MAIN); // Darker bottom (Deep Navy)
      
      // Normal fish colors (using depth-shaded base colors)
      for (auto& f : fish) {
        lv_obj_set_style_text_color(f.obj, f.base_color, LV_PART_MAIN);
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
    
    // 3. Sway Seaweed
    float t_sec = millis() / 1000.0f;
    for (auto& s : seaweed) {
      float sway = std::sin(t_sec * s.speed + s.phase) * 5.0f;
      lv_obj_set_x(s.obj, (int)(s.base_x + sway));
    }
    
    // 4. Update Fish
    for (auto& f : fish) {
      float speed_mult = 1.0f;
      if (panic_timer > 0) {
        speed_mult = 3.2f;
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
      
      // Write new coordinate to screen
      lv_obj_set_pos(f.obj, (int)f.x, (int)f.y);
      
      // Animate fish character representation
      if (f.type == 0) { // Goldfish
        if (f.facing_right) {
          lv_label_set_text(f.obj, "><((((o>");
        } else {
          lv_label_set_text(f.obj, "<o))))><");
        }
      } else if (f.type == 1) { // Neon Tetra
        if (f.facing_right) {
          lv_label_set_text(f.obj, "><((o>");
        } else {
          lv_label_set_text(f.obj, "<o))><");
        }
      } else if (f.type == 2) { // Blowfish
        if (f.puff_timer > 0) {
          f.puff_timer--;
          lv_label_set_text(f.obj, "(  O  )"); // Puffed up!
        } else {
          if (f.facing_right) {
            lv_label_set_text(f.obj, "(o.o)>");
          } else {
            lv_label_set_text(f.obj, "<(o.o)");
          }
        }
      } else if (f.type == 3) { // Seahorse
        if (f.facing_right) {
          lv_label_set_text(f.obj, "S~");
        } else {
          lv_label_set_text(f.obj, "~S");
        }
      }
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
    
    // Alternate crab crawling claws
    if ((int)(crab_x * 0.25f) % 2 == 0) {
      lv_label_set_text(crab_obj, "(\\[o.o]/)");
    } else {
      lv_label_set_text(crab_obj, "(/o.o\\)");
    }
    lv_obj_set_pos(crab_obj, (int)crab_x, (int)crab_y);
    
    // 6. Update Bubbles
    if (bubbler_on && (rand() % 100 < 18)) {
      // Spawn bubble at the bubble generator (located at x=370, y=295)
      for (auto& b : bubbles) {
        if (!b.active) {
          b.active = true;
          b.x = 365.0f + (rand() % 15);
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
      lv_obj_set_pos(shark_obj, (int)shark_x, (int)shark_y);
      
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
        lv_obj_add_flag(shark_obj, LV_OBJ_FLAG_HIDDEN);
      }
    }
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
    lv_obj_clear_flag(shark_obj, LV_OBJ_FLAG_HIDDEN);
    
    // Choose side to spawn and head direction
    if (rand() % 2 == 0) {
      shark_x = -170.0f;
      shark_vx = 2.4f;
      lv_label_set_text(shark_obj, "><((((((((o>");
    } else {
      shark_x = 490.0f;
      shark_vx = -2.4f;
      lv_label_set_text(shark_obj, "<o))))))))><");
    }
    shark_y = 50.0f + (rand() % 130);
    lv_obj_set_pos(shark_obj, (int)shark_x, (int)shark_y);
    
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

