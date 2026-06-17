# Interactive Aquarium Panel (ESPHome + LVGL)

An interactive virtual aquarium simulator running on the ESP32-S3 touchscreen panel (`JC3248W535C_I`) utilizing ESPHome, LVGL, and custom C++ physics, with rich integrations for Home Assistant.

![Aesthetic Virtual Aquarium](https://images.unsplash.com/photo-1544551763-46a013bb70d5?q=80&w=480&auto=format&fit=crop)

## Features

- **Organic Fish Physics:** 3 distinct fish (Goldfish, Neon Tetra, Blowfish) swimming with randomized organic velocities, border bouncing, and directional flipping.
- **Swaying Seaweed:** 5 decorative stalks of seaweed at the bottom that sway back and forth with wave frequency calculations.
- **Crab Scuttler:** A little crab (`(\[o.o]/)`) that crawls back and forth along the aquarium bed.
- **Tap the Glass (Interactive Touch):** Physically tapping the screen or triggering it from Home Assistant displays a visual ripple and causes all the fish to scatter in panic at 3x speed.
- **Feeding Mechanics:** Triggering the feeding action drops flakes from the top. Fish detect, chase, and consume the food flakes, incrementing a Home Assistant counter. The blowfish puff-animates `(  O  )` when it eats.
- **Shark Predator Event:** Spawns a large shark that swims across the screen, panicking and scaring away the fish.
- **Treasure Chest Bubbler:** Toggleable bubble stream rising from a chest ornament.
- **Disco Mode:** Cycles the water background through neon hues and strobes the fish colors.
- **Alert / Meeting Mode:** Flashes a high-contrast red warning banner with custom message text, blinks yellow and red siren lights, and startles the fish (making them scatter and swim rapidly).

## Hardware Configuration (JC3248W535C_I)

- **SoC:** ESP32-S3 (with Octal PSRAM)
- **Display Platform:** `qspi_dbi` (CUSTOM model, CS on GPIO45, Data on GPIO21/48/40/39, CLK on GPIO47)
- **Touch Platform:** `axs15231` (I2C on GPIO4/8)
- **Backlight Platform:** `ledc` PWM on GPIO1

---

## Home Assistant Integration

Once flashed and connected, the panel automatically registers the following entities and actions in Home Assistant.

### Exposed Entities
* **`sensor.aquarium_panel_fish_fed_count`**: Tracks total times the fish have successfully eaten a food flake.
* **`switch.aquarium_panel_aquarium_bubbler`**: Turns on/off the rising bubble stream.
* **`switch.aquarium_panel_aquarium_disco_mode`**: Activates color cycling neon mode.

### Exposed Actions (Services)
* **`esphome.aquarium_panel_feed_fish`**: Drops 3 food particles from the top of the screen.
* **`esphome.aquarium_panel_tap_glass`**: Triggers a ripple and scares the fish.
* **`esphome.aquarium_panel_spawn_shark`**: Releases a shark to swim across the aquarium.
* **`esphome.aquarium_panel_show_alert`**: Triggers a pulsing alert overlay banner on the panel with custom message and blinking sirens, and scares the fish. Accepts `message` and `duration_seconds` variables.
* **`esphome.aquarium_panel_clear_alert`**: Dismisses the alert overlay.

---

## Creative Automation Ideas in Home Assistant

### 1. Feed the Fish on Real Actions
Trigger a feeding in the virtual aquarium whenever someone uses a smart appliance or feeds a real pet:
```yaml
alias: "Aquarium: Feed Fish when coffee is brewed"
trigger:
  - platform: state
    entity_id: switch.smart_coffee_maker
    to: "on"
action:
  - action: esphome.aquarium_panel_feed_fish
```

### 2. Tap the Glass on Motion Detection
Simulate tapping the glass when somebody walks into the room:
```yaml
alias: "Aquarium: Motion Glass Tap"
trigger:
  - platform: state
    entity_id: binary_sensor.living_room_motion
    to: "on"
action:
  - action: esphome.aquarium_panel_tap_glass
```

### 3. Alarm / Ring Alert (Release the Shark)
Release the shark when there's an active alert (like someone ringing the doorbell or a security alarm):
```yaml
alias: "Aquarium: Doorbell Shark Alert"
trigger:
  - platform: state
    entity_id: binary_sensor.doorbell_ringing
    to: "on"
action:
  - action: esphome.aquarium_panel_spawn_shark
```

### 4. Meeting starting / Calendar event alert
Display a banner and startle the fish when a calendar event (like a meeting) is starting:
```yaml
alias: "Aquarium: Meeting Alert"
trigger:
  - platform: calendar
    event: start
    entity_id: calendar.work_calendar
action:
  - action: esphome.aquarium_panel_show_alert
    data:
      message: "Meeting Starting!"
      duration_seconds: 60
```

### 5. Synced bubbler with real filters
Sync the virtual bubble generator with a real aquarium filter plug:
```yaml
alias: "Aquarium: Sync Bubblers"
trigger:
  - platform: state
    entity_id: switch.real_aquarium_filter_plug
  sensor_template: "{{ trigger.to_state.state }}"
action:
  - service: "switch.turn_{{ sensor_template }}"
    target:
      entity_id: switch.aquarium_panel_aquarium_bubbler
```

---

## Setup and Quick Start

1. **Verify Secrets:**
   Ensure `secrets.yaml` is pointing to your root `secrets.yml`. It must contain your `wifi_ssid`, `wifi_password`, and `api_encryption_key`.

2. **Validate Syntax:**
   ```bash
   esphome config aquarium.yml
   ```

3. **Compile and Upload:**
   Connect the ESP32-S3 panel to your computer and run:
   ```bash
   esphome run aquarium.yml
   ```

---

## Troubleshooting: Cross-Subnet Integration & Discovery

If your Home Assistant instance or ESPHome dashboard is on a different subnet than the aquarium panel (e.g., Home Assistant on `192.168.10.x` and the panel on `192.168.1.x`), auto-discovery via mDNS will not work automatically.

### 1. Manually Add to Home Assistant
Since standard TCP/IP routing is functional across the subnets, you can add it manually:
1. Go to **Settings** -> **Devices & Services** in Home Assistant.
2. Click **Add Integration** in the bottom-right corner and select **ESPHome**.
3. Enter the device IP in the **Host** field: `192.168.1.190`
4. When prompted, enter the API encryption key from your `secrets.yaml`:
   ```text
   7XDRs5d5Udo3NzUcLHaCqVzCa9KeO+vQsbQgTYPU5NU=
   ```

### 2. Manual Integration in ESPHome Dashboard
* The ESPHome dashboard only displays devices that have their configuration `.yaml` files in the dashboard's local configuration directory (normally `/config/esphome/` on the Home Assistant system).
* To see and manage the panel via the Home Assistant add-on dashboard, copy `aquarium.yml`, `aquarium.h`, and `secrets.yaml` (or append the keys to your existing secrets) to your Home Assistant's `config/esphome/` directory.
* A `use_address: 192.168.1.190` directive has been added under the `wifi:` block in `aquarium.yml`. This forces the host tools to connect directly to the IP address, bypassing cross-subnet mDNS resolution issues.
