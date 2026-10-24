# Wiring notes: baseline bench rig

Pin map used by `firmware/clear_flow_baseline/clear_flow_baseline.ino`.

| Signal | ESP32 pin | Note |
| --- | --- | --- |
| TRIG (AJ-SR04M) | GPIO 19 | Output pulse (10 microseconds) |
| ECHO (AJ-SR04M) | GPIO 21 | Input; must be 3.3 V level at the ESP32 |
| VBAT sense | GPIO 34 | ADC input, battery voltage through a 2:1 resistor divider |
| GND | GND | Sensor, battery and ESP32 share a common ground |

## Notes

- **Battery:** one 18650 Li-ion cell (3.7 V nominal, about 3.0 V to 4.2 V in use). The firmware maps 3.0 V to 0% and 4.2 V to 100%.
- **Battery divider:** two equal resistors halve the battery voltage so it stays under 3.3 V at the ADC pin. The firmware multiplies the reading by 2. ESP32 ADC readings are approximate, so the battery percentage is an estimate.
- **ECHO level:** if the probe is powered from 5 V, its echo output can reach about 5 V. Divide it down or use a level shifter before it reaches GPIO 21. ESP32 inputs are 3.3 V devices.
- **Probe:** only the AJ-SR04M probe and its cable are waterproof. The small control board must be kept dry (this is why a sealed enclosure is in the Stage 2 design). The blind zone is about 20 cm, so alarm levels must sit beyond it.
- **Planned (not built):** LoRa SPI pins will be fixed when the PCB layout is made.

Photos of the baseline rig are in `../photos/`.

## Blynk setup

| Item | Setting |
| --- | --- |
| Template name | Manhole monitor |
| V1 | Battery, integer 0 to 100 (%) |
| V3 | Water level, 0 to 100 (%) |
| V13 | LED widget, 255 = red (alert), 0 = off |
| Event `flood_a` | Flash flood alert, push notification on |
| Event `block_a` | Blockage alert, push notification on |
