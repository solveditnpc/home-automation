#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

// WiFi credentials
#define WIFI_SSID "your wifi name"
#define WIFI_PASSWORD "your wifi password"

// Pin definitions
#define TOUCH_SENSOR_PIN T0  // GPIO4
#define LED_PIN 2
const int relayPins[4] = {16, 17, 18, 19};  // Relay pins

// Constants
#define TOUCH_THRESHOLD 40
#define SLEEP_TIMEOUT 3600000  // 1 hour in milliseconds
#define TOUCH_WAKE_DURATION 3000  // 3 seconds to wake up

// Global variables
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
bool deviceStates[4] = {false, false, false, false};
unsigned long lastActivityTime = 0;
bool isAsleep = false;
unsigned long touchStartTime = 0;

void notifyClients() {
    StaticJsonDocument<200> doc;
    doc["type"] = "states";
    JsonArray states = doc["states"].to<JsonArray>();
    for (int i = 0; i < 4; i++) {
        states.add(deviceStates[i]);
    }
    
    String output;
    serializeJson(doc, output);
    ws.textAll(output);
}

void handleWebSocketMessage(void* arg, uint8_t* data, size_t len) {
    AwsFrameInfo* info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        StaticJsonDocument<200> doc;
        deserializeJson(doc, data);

        if (doc["type"] == "toggle") {
            int relay = doc["relay"];
            if (relay >= 0 && relay < 4) {
                deviceStates[relay] = !deviceStates[relay];
                digitalWrite(relayPins[relay], deviceStates[relay] ? HIGH : LOW);
                
                // Print device status change
                String deviceType = (relay < 2) ? "Light" : "Fan";
                String location = (relay % 2 == 0) ? "Living Room" : "Bedroom";
                String status = deviceStates[relay] ? "ON" : "OFF";
                
                Serial.print("Device: ");
                Serial.print(deviceType);
                Serial.print(" (");
                Serial.print(location);
                Serial.print(") -> ");
                Serial.println(status);
                
                lastActivityTime = millis();
                notifyClients();
            }
        }
    }
}

void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type,
            void* arg, uint8_t* data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WebSocket client #%u connected\n", client->id());
            notifyClients();
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

void connectToWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    Serial.println("Connecting to WiFi...");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));  // Toggle LED
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to WiFi");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        digitalWrite(LED_PIN, HIGH);  // LED on when connected
    } else {
        Serial.println("\nFailed to connect to WiFi");
        digitalWrite(LED_PIN, LOW);
    }
}

void setup() {
    Serial.begin(115200);
    
    // Initialize pins
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    for (int i = 0; i < 4; i++) {
        pinMode(relayPins[i], OUTPUT);
        digitalWrite(relayPins[i], LOW);
    }
    
    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS initialization failed!");
        return;
    }
    
    // Connect to WiFi
    connectToWiFi();
    
    // Setup WebSocket
    ws.onEvent(onEvent);
    server.addHandler(&ws);
    
    // Setup web server routes
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(SPIFFS, "/index.html", "text/html");
    });
    
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(SPIFFS, "/style.css", "text/css");
    });
    
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(SPIFFS, "/script.js", "text/javascript");
    });
    
    server.on("/light-bulb.png", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(SPIFFS, "/light-bulb.png", "image/png");
    });
    
    server.on("/fan.png", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(SPIFFS, "/fan.png", "image/png");
    });
    
    // Start server
    server.begin();
}

void loop() {
    static unsigned long lastCheck = 0;
    unsigned long currentMillis = millis();
    
    // Check WiFi connection every 5 seconds
    if (currentMillis - lastCheck > 5000) {
        lastCheck = currentMillis;
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi connection lost. Reconnecting...");
            connectToWiFi();
        }
    }
    
    // Check touch sensor
    if (isAsleep) {
        if (touchRead(TOUCH_SENSOR_PIN) < TOUCH_THRESHOLD) {
            if (touchStartTime == 0) {
                touchStartTime = millis();
            } else if (millis() - touchStartTime > TOUCH_WAKE_DURATION) {
                Serial.println("\n=== Waking up from sleep mode ===");
                isAsleep = false;
                lastActivityTime = millis();
                digitalWrite(LED_PIN, HIGH);
                touchStartTime = 0;
            }
        } else {
            touchStartTime = 0;
        }
    }
    
    // Check for sleep condition
    if (!isAsleep && currentMillis - lastActivityTime > SLEEP_TIMEOUT) {
        Serial.println("\n=== Entering sleep mode ===");
        Serial.println("Turning off all devices:");
        isAsleep = true;
        digitalWrite(LED_PIN, LOW);
        // Turn off all relays
        for (int i = 0; i < 4; i++) {
            digitalWrite(relayPins[i], LOW);
            deviceStates[i] = false;
            
            // Print status for each device
            String deviceType = (i < 2) ? "Light" : "Fan";
            String location = (i % 2 == 0) ? "Living Room" : "Bedroom";
            Serial.print("- ");
            Serial.print(deviceType);
            Serial.print(" (");
            Serial.print(location);
            Serial.println(") -> OFF");
        }
        Serial.println("=========================");
        notifyClients();
    }
    
    delay(50);  // Small delay to prevent watchdog issues
}