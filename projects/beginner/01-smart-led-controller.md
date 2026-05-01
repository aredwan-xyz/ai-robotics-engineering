# Project: Smart LED Controller

**Difficulty:** ⬜ Beginner  
**Skills:** ESP32, WS2812B, WiFi, JSON API  
**Estimated Time:** 4–8 hours  
**BOM Cost:** ~$20

---

## What You'll Build

A WiFi-controlled addressable LED strip controller with a REST API. Control color, brightness, and animations from any device on your network.

## What You'll Learn

- WS2812B/NeoPixel protocol (800kHz single-wire data)
- FastLED library
- ESP32 WiFi setup
- HTTP REST API on embedded system
- JSON parsing with ArduinoJSON

## Bill of Materials

| Component | Qty | Cost | Where to Buy |
|-----------|-----|------|--------------|
| ESP32 DevKit (38-pin) | 1 | $5 | AliExpress / Amazon |
| WS2812B LED strip (60 LED/m, IP30) | 1m | $8 | Amazon |
| 5V 3A USB power supply | 1 | $5 | Amazon |
| 470Ω resistor | 1 | $0.10 | Kit |
| 1000μF 16V capacitor | 1 | $0.50 | Kit |
| Breadboard + jumpers | 1 | $3 | Kit |

## Circuit Diagram

```
5V PSU (+) ──────────────┬──── LED Strip VCC (+5V)
                         │
                        [1000μF cap across power rails]
                         │
5V PSU (−) ──────────────┼──── LED Strip GND
                         │
ESP32 GND ───────────────┘

ESP32 GPIO16 ──[470Ω]──── LED Strip DATA (DIN)

Note: The 470Ω resistor prevents signal reflection.
The 1000μF cap provides current surge reservoir — place near strip.
Power the LEDs SEPARATELY from the ESP32 USB — LEDs draw 60mA/LED at full white.
```

## Code

```cpp
// platformio.ini
// [env:esp32dev]
// platform = espressif32
// board = esp32dev
// framework = arduino
// lib_deps =
//     fastled/FastLED@^3.6.0
//     bblanchon/ArduinoJson@^6.21.0

#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#define LED_PIN     16
#define NUM_LEDS    60
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

const char* WIFI_SSID = "YourSSID";
const char* WIFI_PASS = "YourPassword";

CRGB leds[NUM_LEDS];
WebServer server(80);

// Current state
uint8_t brightness = 128;
CRGB solid_color = CRGB::Blue;
String animation = "solid";
uint8_t hue = 0;

void setup() {
    Serial.begin(115200);
    
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(brightness);
    
    // Connect WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
    
    // API endpoints
    server.on("/", HTTP_GET, handleRoot);
    server.on("/api/status", HTTP_GET, handleStatus);
    server.on("/api/color", HTTP_POST, handleSetColor);
    server.on("/api/brightness", HTTP_POST, handleSetBrightness);
    server.on("/api/animation", HTTP_POST, handleSetAnimation);
    server.on("/api/off", HTTP_POST, handleOff);
    server.enableCORS(true);
    server.begin();
    
    Serial.println("API server started");
}

void loop() {
    server.handleClient();
    
    // Update animation
    if (animation == "solid") {
        fill_solid(leds, NUM_LEDS, solid_color);
    } else if (animation == "rainbow") {
        fill_rainbow(leds, NUM_LEDS, hue++, 7);
    } else if (animation == "fire") {
        fireAnimation();
    } else if (animation == "pulse") {
        pulseAnimation();
    }
    
    FastLED.show();
    delay(20);  // ~50fps
}

void handleRoot() {
    server.send(200, "text/html", R"(
        <html><body>
        <h2>LED Controller API</h2>
        <p>POST /api/color    {"r":255,"g":0,"b":0}</p>
        <p>POST /api/brightness {"value":128}</p>
        <p>POST /api/animation {"name":"rainbow"}</p>
        <p>GET  /api/status</p>
        </body></html>
    )");
}

void handleStatus() {
    StaticJsonDocument<256> doc;
    doc["animation"] = animation;
    doc["brightness"] = brightness;
    doc["color"]["r"] = solid_color.r;
    doc["color"]["g"] = solid_color.g;
    doc["color"]["b"] = solid_color.b;
    doc["ip"] = WiFi.localIP().toString();
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void handleSetColor() {
    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "No body");
        return;
    }
    
    StaticJsonDocument<128> doc;
    DeserializationError err = deserializeJson(doc, server.arg("plain"));
    if (err) {
        server.send(400, "text/plain", "Bad JSON");
        return;
    }
    
    solid_color = CRGB(doc["r"] | 0, doc["g"] | 0, doc["b"] | 0);
    animation = "solid";
    server.send(200, "application/json", "{\"ok\":true}");
}

void handleSetBrightness() {
    StaticJsonDocument<64> doc;
    deserializeJson(doc, server.arg("plain"));
    brightness = constrain((int)doc["value"], 0, 255);
    FastLED.setBrightness(brightness);
    server.send(200, "application/json", "{\"ok\":true}");
}

void handleSetAnimation() {
    StaticJsonDocument<64> doc;
    deserializeJson(doc, server.arg("plain"));
    animation = doc["name"].as<String>();
    server.send(200, "application/json", "{\"ok\":true}");
}

void handleOff() {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    animation = "off";
    server.send(200, "application/json", "{\"ok\":true}");
}

// Fire animation
void fireAnimation() {
    static byte heat[NUM_LEDS];
    
    // Cool down
    for (int i = 0; i < NUM_LEDS; i++) {
        heat[i] = qsub8(heat[i], random8(0, 25));
    }
    
    // Heat rises
    for (int k = NUM_LEDS - 1; k >= 2; k--) {
        heat[k] = (heat[k-1] + heat[k-2] + heat[k-2]) / 3;
    }
    
    // Ignite
    if (random8() < 120) {
        int y = random8(7);
        heat[y] = qadd8(heat[y], random8(160, 255));
    }
    
    // Map to color
    for (int j = 0; j < NUM_LEDS; j++) {
        leds[j] = HeatColor(heat[j]);
    }
}

// Breathing/pulse animation
void pulseAnimation() {
    static float phase = 0;
    phase += 0.05;
    uint8_t b = (sin(phase) + 1.0) / 2.0 * 255;
    fill_solid(leds, NUM_LEDS, CRGB(0, 0, b));
}
```

## Testing Your Build

```bash
# Find ESP32 IP from serial monitor, then test:
curl http://192.168.1.xxx/api/status

# Set red color
curl -X POST http://192.168.1.xxx/api/color \
  -H "Content-Type: application/json" \
  -d '{"r":255,"g":0,"b":0}'

# Rainbow animation
curl -X POST http://192.168.1.xxx/api/animation \
  -H "Content-Type: application/json" \
  -d '{"name":"rainbow"}'

# Set brightness
curl -X POST http://192.168.1.xxx/api/brightness \
  -H "Content-Type: application/json" \
  -d '{"value":64}'

# Turn off
curl -X POST http://192.168.1.xxx/api/off
```

## Extensions

Once working, try these:

1. **Add MQTT support** — subscribe to Home Assistant / Node-RED
2. **mDNS** — Access as `led-controller.local` instead of IP
3. **Music reactive** — Add microphone, react to beat
4. **OTA updates** — Update firmware over WiFi (ArduinoOTA)
5. **Web UI** — Serve a color picker HTML page from the ESP32
6. **Scene scheduler** — Store scenes in SPIFFS, schedule with NTP time

## Troubleshooting

| Problem | Likely Cause | Fix |
|---------|-------------|-----|
| LEDs don't light | Bad power connection | Check 5V on strip, not just ESP32 |
| Wrong colors | COLOR_ORDER wrong | Try GRB, RGB, BGR until correct |
| Flickering | Insufficient power | Use 5V 5A PSU for 60+ LEDs |
| Crash on WiFi | Too many LEDs using power | Power strip independently |
| First LED wrong | No 470Ω resistor | Add resistor on data line |
