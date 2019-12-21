#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>

#include "webdata.h"
#include "config.h"

/* FastLED setup */
#define FASTLED_ESP8266_D1_PIN_ORDER
FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    4
//#define CLK_PIN   4
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
const uint16_t NUM_LEDS = 28;
const size_t JSON_CAPACITY = 256;
const size_t JSON_BUF = 1024;

#define FRAMES_PER_SECOND  120


/* Custom data structures */
struct Settings {
  bool power = true;
  uint8_t brightness = 120;

  uint8_t currentPattern = 0;

  uint8_t colorful_speed = 50;
  uint8_t colorful_sat = 96;
  uint8_t rainbow_fade_speed = 50;
  uint8_t rainbow_fade_sat = 96;
  uint8_t rainbow_fade_size = 50;
  uint8_t rainbow_fade_direction = 0;
  uint8_t sparkles_sat = 96;
  uint8_t sparkles_fade = 96;
  uint8_t sparkles_rate = 96;
};


/* Global data */
CRGB leds[NUM_LEDS];
Settings SETTINGS;
ESP8266WebServer server(80);


/* HTTP server callback functions */
void handleRoot() {
  digitalWrite(LED_BUILTIN, 1);
  server.send(200, "text/html", webdata_index_htm, webdata_index_htm_len);
  digitalWrite(LED_BUILTIN, 0);
}

void handleData() {
  // GET-Request: Return data
  if (server.method() == HTTP_GET) {
    digitalWrite(LED_BUILTIN, 1);
    StaticJsonDocument<JSON_CAPACITY> doc;
    doc["power"] = SETTINGS.power;
    doc["brightness"] = SETTINGS.brightness;
    doc["colorful_speed"] = SETTINGS.colorful_speed;
    doc["colorful_sat"] = SETTINGS.colorful_sat;
    char buf[JSON_BUF];
    serializeJson(doc, buf);
    server.send(200, "application/json", buf)  ;
    // TODO send current data
    digitalWrite(LED_BUILTIN, 0);

  // POST-Request: Update data
  } else if (server.method() == HTTP_POST) {
    digitalWrite(LED_BUILTIN, 1);

    // Deserialize JSON data
    StaticJsonDocument<JSON_CAPACITY> doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
  
    // Test if parsing succeeded.
    if (error) {
      char buf[512];
      snprintf(buf, 512, "deserializeJson() failed: %s", error.c_str());
      server.send(500, "text/plain", buf);
      digitalWrite(LED_BUILTIN, 0);
      return;
    }

    // Iterate JSON object's items
    for (JsonPair kv : doc.as<JsonObject>()) {
      if (!strcmp(kv.key().c_str(), "power")) {
        if (kv.value().is<bool>()) {
          SETTINGS.power = kv.value().as<bool>();
          FastLED.setBrightness(SETTINGS.power ? SETTINGS.brightness : 0);
        }
      } else if (!strcmp(kv.key().c_str(), "brightness")) {
        if (kv.value().is<uint8_t>()) {
          SETTINGS.brightness = kv.value().as<uint8_t>();
          FastLED.setBrightness(SETTINGS.power ? SETTINGS.brightness : 0);
        }
      } else if (!strcmp(kv.key().c_str(), "colorful_speed")) {
        if (kv.value().is<uint8_t>())
          SETTINGS.colorful_speed = kv.value().as<uint8_t>();
      } else if (!strcmp(kv.key().c_str(), "colorful_sat")) {
        if (kv.value().is<uint8_t>())
          SETTINGS.colorful_sat = kv.value().as<uint8_t>();
      }
    }
    
    server.send(200, "text/plain", "Success!");
    digitalWrite(LED_BUILTIN, 0);

  // Any other request: Fail
  } else {
    digitalWrite(LED_BUILTIN, 1);
    server.send(405, "text/plain", "Method Not Allowed");
    digitalWrite(LED_BUILTIN, 0);
  }
}


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);
  delay(1000); // 1 second delay for recovery

  // Setup serial port for debugging
  Serial.begin(115200);

  // Setup WIFI and wait for connection
  WiFi.begin(ssid, password);
  WiFi.hostname(hostname);
  Serial.println("");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Start mDNS for network discovery
  if (MDNS.begin(hostname)) {
    Serial.println("MDNS responder started");
  }
  MDNS.addService("http", "tcp", 80);

  // Add HTTP server endpoints and start server
  server.on("/", handleRoot);
  server.on("/api/data/", handleData);
  server.begin();
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(0xFFE0F0);

  // set master brightness control
  FastLED.setBrightness(SETTINGS.brightness);
}

// List of animation patterns
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { colorful, rainbow_fade, sparkles };
  
void loop()
{
  MDNS.update();
  server.handleClient();
  // TODO adapt dynamically
  FastLED.delay(1000/FRAMES_PER_SECOND);
  gPatterns[SETTINGS.currentPattern]();
  FastLED.show();
}

void colorful() 
{
  CHSV hsv;
  hsv.hue = SETTINGS.colorful_speed * millis();
  hsv.val = 255;
  hsv.sat = ((uint16_t)SETTINGS.colorful_sat) * 5 / 2;
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = hsv;
    hsv.hue += 255 / NUM_LEDS;
  }
}

void rainbow_fade()
{
  // TODO
  return;
}

void sparkles() {
  // TODO 
  return;
}
