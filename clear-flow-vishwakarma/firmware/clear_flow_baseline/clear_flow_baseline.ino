/*
  Clear-Flow: baseline firmware (Stage 1 lab prototype)
  Vishwakarma Awards 2026-27  |  Application ID: VK26-IITH-SC-0059

  What this sketch does (the features completed so far):
    1. Measures the distance from the sensor to the water surface with an
       AJ-SR04M waterproof ultrasonic probe.
    2. Takes 5 readings per measurement and uses the MEDIAN to remove noise
       caused by splashes and ripples.
    3. Converts the distance to a water level (% of manhole depth).
    4. Reads the battery voltage on an ADC pin (through a 2:1 divider) and
       converts it to a battery percentage.
    5. Sends water level (V3) and battery (V1) to the Blynk cloud over Wi-Fi
       every 2 seconds.
    6. Flash flood alert: level >= 60% -> red LED (V13) + Blynk event "flood_a".
    7. Blockage alert: level >= 40% held for 20 s -> red LED (V13) + Blynk
       event "block_a".
    Each alert is sent once, and is re-armed when the level drops below 40%.

  Not built yet (planned in the Stage 2 design): LoRaWAN link, deep sleep,
  sealed enclosure, own dashboard, rain-aware and per-manhole adaptive logic.

  IMPORTANT: the values below are PLACEHOLDERS. Put your real Blynk token and
  Wi-Fi details only in the copy you flash to the board. Never upload them.

  The 20 s hold, 2 s interval and 40% / 60% limits are bench-test values,
  chosen for demonstration. They are not field settings.
*/

// ---- Blynk template (create the same template and datastreams in Blynk) ----
#define BLYNK_TEMPLATE_ID   "TMPL3UO_DhQaK"
#define BLYNK_TEMPLATE_NAME "Manhole monitor"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// ---- Credentials (placeholders: replace locally, do not commit real ones) ----
char auth[] = "YOUR_BLYNK_AUTH_TOKEN";
char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";

// ---- Hardware pins ----
const int trigPin    = 19;   // AJ-SR04M TRIG
const int echoPin    = 21;   // AJ-SR04M ECHO
const int batteryPin = 34;   // ADC input, battery voltage via 2:1 divider

// ---- Physical setup ----
const float maxManholeDepth = 100.0;  // depth of the test setup, in cm

// ---- Bench-test alert settings ----
const float FLOOD_LEVEL_PERCENT    = 60.0;    // immediate flash flood alert
const float BLOCKAGE_LEVEL_PERCENT = 40.0;    // blockage if held this high ...
const unsigned long BLOCKAGE_HOLD_MS = 20000; // ... for this long (20 s)
const long SEND_INTERVAL_MS = 2000L;          // measure and upload every 2 s

BlynkTimer timer;

// State flags for alerts
unsigned long blockageStartTime = 0;
bool blockageAlertSent = false;
bool floodAlertSent = false;

// Median filter: takes 5 readings and returns the middle value
float getMedianDistance() {
  float samples[5];
  for (int i = 0; i < 5; i++) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 30000);  // 30 ms time-out
    if (duration == 0) {
      samples[i] = maxManholeDepth;                 // no echo = treat as empty
    } else {
      samples[i] = duration * 0.034 / 2.0;          // distance in cm
    }
    delay(50);
  }

  // Sort the 5 samples (simple exchange sort)
  for (int i = 0; i < 4; i++) {
    for (int j = i + 1; j < 5; j++) {
      if (samples[j] < samples[i]) {
        float temp = samples[i];
        samples[i] = samples[j];
        samples[j] = temp;
      }
    }
  }
  return samples[2];  // middle value = median
}

void sendSensorData() {
  // 1. Water level
  float distance = getMedianDistance();
  float waterLevelHeight = maxManholeDepth - distance;
  if (waterLevelHeight < 0) waterLevelHeight = 0;
  float waterLevelPercent = (waterLevelHeight / maxManholeDepth) * 100.0;

  // 2. Battery level (divider halves the voltage, so multiply by 2)
  int rawADC = analogRead(batteryPin);
  float voltage = (rawADC / 4095.0) * 3.3 * 2.0;
  int batteryPercent = map(voltage * 100, 300, 420, 0, 100);  // 3.0 V to 4.2 V
  batteryPercent = constrain(batteryPercent, 0, 100);

  // Debug output
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm | Water Level: ");
  Serial.print(waterLevelPercent);
  Serial.print("% | Battery: ");
  Serial.print(batteryPercent);
  Serial.println("%");

  // Push data to the Blynk dashboard
  Blynk.virtualWrite(V3, waterLevelPercent);
  Blynk.virtualWrite(V1, batteryPercent);

  // 3. Alert logic
  if (waterLevelPercent >= FLOOD_LEVEL_PERCENT) {
    Blynk.virtualWrite(V13, 255);  // red LED on
    if (!floodAlertSent) {
      Serial.println("Triggering Flash Flood Alert...");
      Blynk.logEvent("flood_a", "CRITICAL: Flash flood detected in Manhole A!");
      floodAlertSent = true;
    }
  }
  else if (waterLevelPercent >= BLOCKAGE_LEVEL_PERCENT) {
    if (blockageStartTime == 0) {
      blockageStartTime = millis();
      Serial.println("Water level above 40%. Starting 20 s blockage timer...");
    }
    else if (millis() - blockageStartTime >= BLOCKAGE_HOLD_MS) {
      Blynk.virtualWrite(V13, 255);  // red LED on
      if (!blockageAlertSent) {
        Serial.println("Triggering Persistent Blockage Alert...");
        Blynk.logEvent("block_a", "WARNING: Persistent blockage detected in Manhole A!");
        blockageAlertSent = true;
      }
    }
  }
  else {
    // Below 40%: reset everything
    Blynk.virtualWrite(V13, 0);    // LED off
    blockageStartTime = 0;
    blockageAlertSent = false;
    floodAlertSent = false;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(batteryPin, INPUT);

  Serial.println("Initializing Blynk connection...");
  Blynk.begin(auth, ssid, pass);

  timer.setInterval(SEND_INTERVAL_MS, sendSensorData);
}

void loop() {
  Blynk.run();
  timer.run();
}
