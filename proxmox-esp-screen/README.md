# Proxmox ESP Screen (ESPHome + LVGL)

Small touchscreen dashboard for a Proxmox cluster, powered by ESPHome and Home Assistant entities.

This folder contains two ready-to-use configs:

- `proxmox.yml`: stable overview screen (cluster summary table).
- `proxmox-extended.yml`: overview + swipeable per-node detail pages.

## Hardware target

These configs are built for the **JC3248W535C_I** style ESP32-S3 panel (QSPI display + AXS15231 touch):

- ESP32-S3 (`esp32-s3-devkitc-1`)
- 320x480 display (`qspi_dbi`, rotation `90` for landscape UI)
- AXS15231 touchscreen over I2C

If your panel differs, update pins/display/touch sections first.

## Files in this folder

- `proxmox.yml`:
  - main dashboard table (node, CPU, RAM, disk, VM, CT)
  - status line (`OK`, `NODE DOWN`, `DISK PROBLEM`, `UPDATES PENDING`)
- `proxmox-extended.yml`:
  - all of the above
  - swipe navigation between main page and node pages
  - row tap on main page to jump to a node page
  - logo image (`proxmox-panelbg.png`)
- `proxmox-panelbg.png`:
  - logo image used by `proxmox-extended.yml`

## Prerequisites

1. ESPHome installed.
2. Home Assistant with the Proxmox integration (or equivalent sensors) exposing the entities used in YAML.
3. `secrets.yaml` available for:
   - `wifi_ssid`
   - `wifi_password`
   - `api_encryption_key`

In this repo, `secrets.yaml` is a symlink to `../secrets.yml`.

## Quick start

From this folder:

```bash
# Validate
esphome config proxmox.yml
esphome config proxmox-extended.yml

# Flash one of them
esphome run proxmox.yml
# or
esphome run proxmox-extended.yml
```

## Home Assistant entities expected

Both files assume node/disk entities following this style:

- Node up status: `binary_sensor.node_proxmox_<n>_status`
- Updates pending: `binary_sensor.node_proxmox_<n>_updates_packages`
- CPU/RAM/disk%: `sensor.node_proxmox_<n>_cpu_used`, `...memory_used_percentage`, `...disk_used_percentage`
- VM/CT counts: `sensor.node_proxmox_<n>_virtual_machines_running`, `...containers_running`
- Extended file also uses:
  - memory used/free (`sensor.node_proxmox_<n>_memory_used`, `...memory_free`)
  - total updates (`sensor.node_proxmox_<n>_total_updates`)

Disk health is read from `binary_sensor.disk_*_health` entities with `device_class: problem`.

Important semantics in these configs:

- `ON` = disk problem
- `OFF` = healthy

If your sensors use opposite semantics, invert the disk logic in the `interval` lambda.

## Customization guide

### 1) Your Wi-Fi/API settings

Update `secrets.yaml` values used by `!secret`.

### 2) Your panel hardware

Adjust:

- `spi` pins
- `i2c` pins
- `display` (`dimensions`, `rotation`, model/settings)
- `touchscreen` transform (`swap_xy`, `mirror_x`, `mirror_y`)

### 3) Your Home Assistant entity names

Edit `binary_sensor:` and `sensor:` entries in the YAML to match your entity IDs.

### 4) Number of nodes

Current layout assumes 5 nodes. To change:

- add/remove sensor and binary sensor IDs
- adjust row labels/widgets
- update formatting and status lambda logic
- for extended file, add/remove page definitions and swipe script transitions

### 5) Styling

Main theme and colors are under `lvgl.theme`.
Fonts are configured in `font:`.

## Troubleshooting

- `No Data` image placeholder:
  - Ensure `proxmox-panelbg.png` exists and path matches `image.file`.
  - Run `esphome config proxmox-extended.yml` after edits.
- Touch/swipe direction wrong:
  - tune touchscreen `transform` and swipe `dx` mapping in `on_update`.
- Disk status appears inverted:
  - verify your HA disk health sensor semantics (`problem` class should be `ON` when failing).
- Empty table values:
  - check API connection to Home Assistant and entity IDs.

## Notes for open source

- Remove or rotate private values in secrets before publishing.
- Keep screenshots/docs aligned with the currently shipped YAML.
- Mention your exact panel model in project description to avoid pin mismatch issues.
