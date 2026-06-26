# Interactive Space Panel (ESPHome + LVGL)

An interactive virtual space simulator running on the ESP32-S3 touchscreen panel (`JC3248W535C_I`) utilizing ESPHome, LVGL, and custom C++ astrophysics simulation, with rich integrations for Home Assistant.

![Aesthetic Virtual Space](https://images.unsplash.com/photo-1506318137071-a8e063b4bec0?q=80&w=480&auto=format&fit=crop)

## Features

- **Astronaut & Vessel Physics:** 12 active objects (Rockets, UFOs, Astronauts, Satellites) drifting with custom velocities, zero-gravity damping, and border wrapping.
  - **Rockets:** Move linearly. Occasionally fire thrusters, accelerating rapidly with a trail animation (`|=I=>~`).
  - **UFOs:** Cruise in organic wave curves, executing occasional local coordinate hyper-jumps.
  - **Astronauts:** Drift slowly in deep zero-g, tumbling/rotating (`[o]`, `\o/`, `_o_`, `/o\`).
  - **Satellites:** Orbit slowly and flash communication beacons.
- **Twinkling Starfield:** 16 stars across three depth layers (foreground, midground, background) shimmering organic brightness pulses using depth-based sizing and randomized phases.
- **Orbital Drifting Planets:** Vector spheres representing **Jupiter** (banded orange-brown), **Saturn** (gold core + rings), **Earth** (blue/green), and **Mars** (red-orange) bobbing slowly via trigonometry-based orbital drift.
- **Gravity Shockwave (Interactive Touch):** Tapping the screen or triggering via Home Assistant spawns a visual warp shockwave (`*WARP*`), repelling all vessels and astronauts away from the touch coordinates.
- **Space Scrap Collection:** Spawns energy crystals from the upper screen. Vessels detect, chase, and collect the crystals, incrementing a Home Assistant sensor counter.
- **Wormhole Portal:** Toggles a pulsing neon portal that ejects glowing cosmic dust particles spiraling into the cosmos.
- **Supernova / Nebula Storm:** Cycles the space background through glowing neon nebula colors (cyan, magenta, electric purple) and accelerates star twinkling rates.
- **Red Alert Mode:** Triggers a warning overlay banner, pulses alternating red and cyan beacons at 4Hz, and scares vessels/astronauts, sending them into panic flight speeds.

## Hardware Configuration (JC3248W535C_I)

- **SoC:** ESP32-S3 (with Octal PSRAM)
- **Display Platform:** `qspi_dbi` (CUSTOM model, CS on GPIO45, Data on GPIO21/48/40/39, CLK on GPIO47)
- **Touch Platform:** `axs15231` (I2C on GPIO4/8)
- **Backlight Platform:** `ledc` PWM on GPIO1

---

## Home Assistant Integration

Once flashed and connected, the panel exposes the following entities and actions.

### Exposed Entities
* **`sensor.space_panel_space_scrap_collected`**: Tracks total crystals collected by the spaceships.
* **`switch.space_panel_wormhole_portal`**: Turns on/off the portal's cosmic dust generator.
* **`switch.space_panel_space_supernova_mode`**: Activates supernova color cycling.

### Exposed Actions (Services)
* **`esphome.space_panel_drop_space_crystals`**: Drops energy crystals from the top.
* **`esphome.space_panel_gravity_pulse`**: Triggers a central shockwave repelling vessels.
* **`esphome.space_panel_spawn_alien_boss`**: Releases a giant alien boss to cross the screen.
* **`esphome.space_panel_show_alert`**: Activates Red Alert mode with custom message and duration.
* **`esphome.space_panel_clear_alert`**: Clears the alert banner.

---

## Setup and Quick Start

1. **Verify Secrets:**
   Ensure `secrets.yaml` contains your wifi details and `api_encryption_key`.

2. **Validate Syntax:**
   ```bash
   esphome config space.yml
   ```

3. **Compile and Upload:**
   Connect the ESP32-S3 panel to your computer and run:
   ```bash
   esphome run space.yml
   ```
