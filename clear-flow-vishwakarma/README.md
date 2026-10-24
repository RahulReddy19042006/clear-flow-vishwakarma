# Clear-Flow: Smart Manhole Monitoring and Overflow Prediction System

**Vishwakarma Awards 2026-27** | Application ID: `VK26-IITH-SC-0059`
**Track:** Sustainable Cities (IIT Hyderabad) | **Sub-theme:** Clean Water, Sanitation & Air Quality Monitoring

Clear-Flow is a low-cost node that sits under a manhole cover, measures the water level from above, and warns the city before the manhole overflows. Drainage maintenance today is reactive: a blockage is found on a routine round or after a complaint, when water is often already on the road.

## Status

**Built (Stage 1 baseline, lab prototype)**
- ESP32 + AJ-SR04M waterproof ultrasonic probe measuring distance to the water surface
- 5-sample median filter to remove splash and ripple noise
- Water level (%) and battery level (%) sent to a Blynk dashboard over Wi-Fi every 2 seconds
- Flash flood alert at 60% level, and blockage alert at 40% held for 20 seconds, both as push notifications
- Tested on a two-container bench rig (see `photos/`)

**Planned (Stage 2 design, not built yet)**
- LoRaWAN (IN865) link and deep-sleep firmware, replacing Wi-Fi
- Sealed IP67/IP68 enclosure and custom PCB
- Own dashboard with heat map, weather/rain flag and per-manhole adaptive thresholds
- Silting trend from daily night-minimum levels, to schedule cleaning

The 2 s interval, 40% / 60% limits and 20 s hold are **bench-test values**, chosen for demonstration. They are not field settings.

## Repository layout

| Folder | Contents |
| --- | --- |
| `firmware/clear_flow_baseline/` | Arduino sketch for the baseline rig |
| `hardware/` | Wiring notes, Blynk setup, planned bill of materials |
| `photos/` | Baseline rig photo and Blynk dashboard screenshot (Stage 1) |
| `docs/` | Stage 1 proposal and Stage 1 progress update (PDF) |

## Run the baseline firmware

1. Install the Arduino IDE and the **ESP32** board package.
2. Install the **Blynk** library (Library Manager).
3. In Blynk, create a template named `Manhole monitor` with datastreams V1 (battery), V3 (water level) and V13 (LED), and events `flood_a` and `block_a` (see `hardware/wiring_notes.md`).
4. Open `firmware/clear_flow_baseline/clear_flow_baseline.ino` and replace the placeholders `YOUR_BLYNK_AUTH_TOKEN`, `YOUR_WIFI_NAME` and `YOUR_WIFI_PASSWORD` **in your local copy only**.
5. Wire the board as in `hardware/wiring_notes.md`, then upload.

Real tokens and Wi-Fi passwords are intentionally not stored in this repository.

## Team

CMR College of Engineering and Technology, Hyderabad. Maker journey and design details are in the Stage 2 submission.
