/**
 * ESP32 IoT Node Template
 *
 * Features: WiFi, MQTT, OTA updates, deep sleep, JSON config, watchdog
 * Copy this for any sensor node project.
 *
 * platformio.ini deps:
 *   knolleary/PubSubClient@^2.8
 *   bblanchon/ArduinoJson@^6.21.0
 *   arduino-libraries/NTPClient@^3.2.1
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <ArduinoOTA.h>
#include <esp_task_wdt.h>
#include <esp_sleep.h>

// ─── Configuration ────────────────────────────────────────────────────────────

struct Config {
    char wifi_ssid[32]     = "your-ssid";
    char wifi_pass[64]     = "your-password";
    char mqtt_host[64]     = "192.168.1.100";
    uint16_t mqtt_port     = 1883;
    char mqtt_user[32]     = "";
    char mqtt_pass[32]     = "";
    char device_id[32]     = "esp32-node-01";
    uint32_t sleep_sec     = 0;     // 0 = no deep sleep
    uint32_t report_ms     = 5000;  // publish interval
    bool ota_enabled       = true;
};

Config config;
const char* CONFIG_FILE = "/config.json";

// ─── Globals ──────────────────────────────────────────────────────────────────

WiFiClient wifi_client;
PubSubClient mqtt(wifi_client);

String topic_telemetry;
String topic_cmd;
String topic_status;

unsigned long last_report_ms = 0;

// ─── Config Load/Save ─────────────────────────────────────────────────────────

void loadConfig() {
    if (!SPIFFS.begin(true)) {
        Serial.println("[config] SPIFFS mount failed, using defaults");
        return;
    }

    if (!SPIFFS.exists(CONFIG_FILE)) {
        Serial.println("[config] No config file, using defaults");
        return;
    }

    File f = SPIFFS.open(CONFIG_FILE, "r");
    StaticJsonDocument<512> doc;
    if (deserializeJson(doc, f)) {
        Serial.println("[config] JSON parse failed");
        return;
    }
    f.close();

    strlcpy(config.wifi_ssid,  doc["wifi_ssid"]  | config.wifi_ssid,  sizeof(config.wifi_ssid));
    strlcpy(config.wifi_pass,  doc["wifi_pass"]  | config.wifi_pass,  sizeof(config.wifi_pass));
    strlcpy(config.mqtt_host,  doc["mqtt_host"]  | config.mqtt_host,  sizeof(config.mqtt_host));
    config.mqtt_port    = doc["mqtt_port"]    | config.mqtt_port;
    strlcpy(config.mqtt_user,  doc["mqtt_user"]  | config.mqtt_user,  sizeof(config.mqtt_user));
    strlcpy(config.mqtt_pass,  doc["mqtt_pass"]  | config.mqtt_pass,  sizeof(config.mqtt_pass));
    strlcpy(config.device_id,  doc["device_id"]  | config.device_id,  sizeof(config.device_id));
    config.sleep_sec    = doc["sleep_sec"]    | config.sleep_sec;
    config.report_ms    = doc["report_ms"]    | config.report_ms;
    config.ota_enabled  = doc["ota_enabled"]  | config.ota_enabled;

    Serial.printf("[config] Loaded. Device: %s\n", config.device_id);
}

// ─── WiFi ─────────────────────────────────────────────────────────────────────

bool connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(config.wifi_ssid, config.wifi_pass);

    Serial.printf("[wifi] Connecting to %s", config.wifi_ssid);
    uint8_t attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\n[wifi] Connected. IP: %s, RSSI: %ddBm\n",
                      WiFi.localIP().toString().c_str(), WiFi.RSSI());
        return true;
    }

    Serial.println("\n[wifi] Failed to connect");
    return false;
}

// ─── MQTT ─────────────────────────────────────────────────────────────────────

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
    payload[length] = '\0';  // null-terminate
    String msg = String((char*)payload);
    Serial.printf("[mqtt] Received on %s: %s\n", topic, msg.c_str());

    StaticJsonDocument<256> doc;
    if (deserializeJson(doc, msg)) return;

    // Handle commands
    if (String(topic) == topic_cmd) {
        if (doc.containsKey("reboot") && (bool)doc["reboot"]) {
            Serial.println("[cmd] Rebooting...");
            delay(100);
            ESP.restart();
        }
        if (doc.containsKey("sleep_sec")) {
            config.sleep_sec = doc["sleep_sec"];
        }
        if (doc.containsKey("report_ms")) {
            config.report_ms = doc["report_ms"];
        }
    }
}

bool connectMqtt() {
    mqtt.setServer(config.mqtt_host, config.mqtt_port);
    mqtt.setCallback(onMqttMessage);
    mqtt.setKeepAlive(60);
    mqtt.setBufferSize(512);

    String client_id = String(config.device_id) + "-" + String(random(0xffff), HEX);

    bool connected = strlen(config.mqtt_user) > 0
        ? mqtt.connect(client_id.c_str(), config.mqtt_user, config.mqtt_pass,
                       topic_status.c_str(), 1, true, "{\"online\":false}")
        : mqtt.connect(client_id.c_str(),
                       topic_status.c_str(), 1, true, "{\"online\":false}");

    if (connected) {
        // Online message (retained)
        StaticJsonDocument<128> doc;
        doc["online"] = true;
        doc["ip"] = WiFi.localIP().toString();
        doc["rssi"] = WiFi.RSSI();
        String status_msg;
        serializeJson(doc, status_msg);
        mqtt.publish(topic_status.c_str(), status_msg.c_str(), true);

        // Subscribe to commands
        mqtt.subscribe(topic_cmd.c_str());
        Serial.printf("[mqtt] Connected to %s:%d\n", config.mqtt_host, config.mqtt_port);
        return true;
    }

    Serial.printf("[mqtt] Failed, state=%d\n", mqtt.state());
    return false;
}

// ─── OTA ──────────────────────────────────────────────────────────────────────

void setupOTA() {
    ArduinoOTA.setHostname(config.device_id);
    ArduinoOTA.onStart([]() {
        Serial.println("[ota] Starting update");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("[ota] Progress: %u%%\r", progress / (total / 100));
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\n[ota] Done");
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("[ota] Error[%u]\n", error);
    });
    ArduinoOTA.begin();
}

// ─── Sensor Reading (replace with your sensor) ────────────────────────────────

struct SensorData {
    float temperature = 0;
    float humidity    = 0;
    float voltage     = 0;
};

SensorData readSensors() {
    SensorData data;

    // Replace with actual sensor reads:
    // data.temperature = dht.readTemperature();
    // data.humidity = dht.readHumidity();

    // Battery voltage via ADC (example: resistor divider on GPIO34)
    // uint32_t raw = analogRead(34);
    // data.voltage = raw * (3.3 / 4095.0) * 2.0;  // × 2 for divider

    // Placeholder
    data.temperature = 25.0 + random(-50, 50) / 100.0f;
    data.humidity    = 60.0 + random(-100, 100) / 100.0f;
    data.voltage     = 3.7 + random(-10, 10) / 100.0f;

    return data;
}

// ─── Publish Telemetry ────────────────────────────────────────────────────────

void publishTelemetry() {
    SensorData data = readSensors();

    StaticJsonDocument<256> doc;
    doc["device"] = config.device_id;
    doc["ts"]     = millis();
    doc["temp"]   = round(data.temperature * 100) / 100.0;
    doc["hum"]    = round(data.humidity * 100) / 100.0;
    doc["vbat"]   = round(data.voltage * 100) / 100.0;
    doc["rssi"]   = WiFi.RSSI();
    doc["uptime"] = millis() / 1000;

    String payload;
    serializeJson(doc, payload);

    if (mqtt.publish(topic_telemetry.c_str(), payload.c_str())) {
        Serial.printf("[mqtt] Published: %s\n", payload.c_str());
    } else {
        Serial.println("[mqtt] Publish failed");
    }
}

// ─── Deep Sleep ───────────────────────────────────────────────────────────────

void goToSleep(uint32_t seconds) {
    Serial.printf("[sleep] Sleeping for %d seconds\n", seconds);

    // Disconnect cleanly
    mqtt.disconnect();
    WiFi.disconnect(true);
    delay(100);

    esp_sleep_enable_timer_wakeup((uint64_t)seconds * 1000000ULL);
    esp_deep_sleep_start();
}

// ─── Setup ────────────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n[boot] ESP32 IoT Node starting");

    // Watchdog: reboot if stuck for 30 seconds
    esp_task_wdt_init(30, true);
    esp_task_wdt_add(NULL);

    loadConfig();

    // Build MQTT topics
    topic_telemetry = String("devices/") + config.device_id + "/telemetry";
    topic_cmd       = String("devices/") + config.device_id + "/cmd";
    topic_status    = String("devices/") + config.device_id + "/status";

    if (!connectWiFi()) {
        Serial.println("[boot] WiFi failed. Will retry after sleep.");
        goToSleep(60);
    }

    if (config.ota_enabled) {
        setupOTA();
    }

    connectMqtt();

    Serial.println("[boot] Ready");
}

// ─── Loop ─────────────────────────────────────────────────────────────────────

void loop() {
    esp_task_wdt_reset();  // feed watchdog

    // OTA check
    if (config.ota_enabled) {
        ArduinoOTA.handle();
    }

    // Reconnect WiFi if dropped
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[wifi] Lost connection, reconnecting...");
        connectWiFi();
    }

    // Reconnect MQTT if dropped
    if (!mqtt.connected()) {
        Serial.println("[mqtt] Disconnected, reconnecting...");
        connectMqtt();
    }

    mqtt.loop();

    // Publish telemetry on interval
    unsigned long now = millis();
    if (now - last_report_ms >= config.report_ms) {
        last_report_ms = now;
        publishTelemetry();
    }

    // Deep sleep if configured
    if (config.sleep_sec > 0) {
        publishTelemetry();  // publish once before sleep
        goToSleep(config.sleep_sec);
        // Never returns
    }
}
