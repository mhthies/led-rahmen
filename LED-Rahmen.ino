// Copyright 2019 by Michael Thies
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
// the License. You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>

#include "patterns.hpp"
#include "settings.hpp"
#include "webdata.h"
#include "constants.h"
#include "config.h"

/* FastLED setup */
#define FASTLED_ESP8266_D1_PIN_ORDER
FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

// static setup
const size_t JSON_CAPACITY = 256;
const size_t JSON_BUF = 1024;


/* *****************************************************************************
 * Global data
 * ****************************************************************************/
CRGB leds[NUM_LEDS];
Settings SETTINGS;
ESP8266WebServer server(80);

extern SimplePatternList gPatterns;
extern size_t gPatternsLen;


/* *****************************************************************************
 * HTTP server handler functions
 * ****************************************************************************/
void handleRoot() {
  digitalWrite(LED_BUILTIN, 0);
  server.send(200, "text/html", webdata_index_htm, webdata_index_htm_len);
  digitalWrite(LED_BUILTIN, 1);
}

void handleData() {
  // GET-Request: Return data
  digitalWrite(LED_BUILTIN, 0);
  if (server.method() == HTTP_GET) {
    StaticJsonDocument<JSON_CAPACITY> doc;
    doc["power"] = SETTINGS.power;
    doc["brightness"] = SETTINGS.brightness;
    doc["currentPattern"] = SETTINGS.currentPattern;
    doc["colorful_speed"] = SETTINGS.colorful_speed;
    doc["colorful_sat"] = SETTINGS.colorful_sat;
    doc["rainbow_fade_speed"] = SETTINGS.rainbow_fade_speed;
    doc["rainbow_fade_sat"] = SETTINGS.rainbow_fade_sat;
    doc["rainbow_fade_size"] = SETTINGS.rainbow_fade_size;
    doc["rainbow_fade_direction"] = SETTINGS.rainbow_fade_direction;
    doc["sparkles_fade"] = SETTINGS.sparkles_fade;
    doc["sparkles_sat"] = SETTINGS.sparkles_sat;
    doc["sparkles_rate"] = SETTINGS.sparkles_rate;
    char buf[JSON_BUF];
    serializeJson(doc, buf);
    server.send(200, "application/json", buf);

  // POST-Request: Update data
  } else if (server.method() == HTTP_POST) {

    // Deserialize JSON data
    StaticJsonDocument<JSON_CAPACITY> doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
  
    // Test if parsing succeeded.
    if (error) {
      char buf[512];
      snprintf(buf, 512, "deserializeJson() failed: %s", error.c_str());
      server.send(500, "text/plain", buf);
      digitalWrite(LED_BUILTIN, 1);
      return;
    }

    // Iterate JSON object's items
    for (JsonPair kv : doc.as<JsonObject>()) {
      if (!strcmp(kv.key().c_str(), "power")) {
        if (kv.value().is<bool>()) {
          SETTINGS.power = kv.value().as<bool>();
          FastLED.setBrightness(SETTINGS.power ? (SETTINGS.brightness * (uint16_t)255 / 100) : 0);
        }
      } else if (!strcmp(kv.key().c_str(), "brightness")) {
        if (kv.value().is<uint8_t>()) {
          uint8_t value = kv.value().as<uint8_t>();
          if (value >= 0 && value <= 100) {
            SETTINGS.brightness = kv.value().as<uint8_t>();
            FastLED.setBrightness(SETTINGS.power ? (SETTINGS.brightness * (uint16_t)255 / 100) : 0);
          }
        }
      } else if (!strcmp(kv.key().c_str(), "currentPattern")) {
        if (kv.value().is<uint8_t>()) {
          uint8_t value = kv.value().as<uint8_t>();
          if (value >= 0 && value < gPatternsLen)
            SETTINGS.currentPattern = kv.value().as<uint8_t>();
        }

      } else if (!strcmp(kv.key().c_str(), "colorful_speed")) {
        if (kv.value().is<uint8_t>()) {
          uint8_t value = kv.value().as<uint8_t>();
          if (value >= 0 && value <= 100)
            SETTINGS.colorful_speed = kv.value().as<uint8_t>();
        }
      } else if (!strcmp(kv.key().c_str(), "colorful_sat")) {
        if (kv.value().is<uint8_t>()) {
          uint8_t value = kv.value().as<uint8_t>();
          if (value >= 0 && value <= 100)
            SETTINGS.colorful_sat = kv.value().as<uint8_t>();
        }

      } else if (!strcmp(kv.key().c_str(), "rainbow_fade_speed")) {
        if (kv.value().is<uint8_t>()) {
          uint8_t value = kv.value().as<uint8_t>();
          if (value >= 0 && value <= 100)
            SETTINGS.rainbow_fade_speed = kv.value().as<uint8_t>();
        }
      } else if (!strcmp(kv.key().c_str(), "rainbow_fade_sat")) {
        if (kv.value().is<uint8_t>()) {
          uint8_t value = kv.value().as<uint8_t>();
          if (value >= 0 && value <= 100)
            SETTINGS.rainbow_fade_sat = kv.value().as<uint8_t>();
        }
      } else if (!strcmp(kv.key().c_str(), "rainbow_fade_size")) {
        if (kv.value().is<uint8_t>()) {
          uint8_t value = kv.value().as<uint8_t>();
          if (value >= 0 && value <= 100)
            SETTINGS.rainbow_fade_size = kv.value().as<uint8_t>();
        }
      } else if (!strcmp(kv.key().c_str(), "rainbow_fade_direction")) {
        if (kv.value().is<uint16_t>()) {
          uint16_t value = kv.value().as<uint16_t>();
          if (value >= 0 && value <= 360)
            SETTINGS.rainbow_fade_direction = kv.value().as<uint16_t>();
        }

      } else if (!strcmp(kv.key().c_str(), "sparkles_fade")) {
        if (kv.value().is<uint8_t>()) {
          uint8_t value = kv.value().as<uint8_t>();
          if (value >= 0 && value <= 100)
            SETTINGS.sparkles_fade = kv.value().as<uint8_t>();
        }
      } else if (!strcmp(kv.key().c_str(), "sparkles_sat")) {
        if (kv.value().is<uint8_t>()) {
          uint8_t value = kv.value().as<uint8_t>();
          if (value >= 0 && value <= 100)
            SETTINGS.sparkles_sat = kv.value().as<uint8_t>();
        }
      } else if (!strcmp(kv.key().c_str(), "sparkles_rate")) {
        if (kv.value().is<uint8_t>()) {
          uint8_t value = kv.value().as<uint8_t>();
          if (value >= 0 && value <= 100)
            SETTINGS.sparkles_rate = kv.value().as<uint8_t>();
        }
      }
    }
    
    server.send(200, "text/plain", "Success!");

  // Any other request: Fail
  } else {
    server.send(405, "text/plain", "Method Not Allowed");
  }
  digitalWrite(LED_BUILTIN, 1);
}


/* *****************************************************************************
 * Arduino main functions
 * ****************************************************************************/

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 1);
  delay(1000); // 1 second delay for recovery
  digitalWrite(LED_BUILTIN, 0);

  // Setup serial port for debugging
  Serial.begin(115200);

  // Setup WIFI and wait for connection
  WiFi.begin(ssid, password);
  WiFi.hostname(hostname);
  Serial.println("");
  bool ledState = false;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    digitalWrite(LED_BUILTIN, ledState);
    ledState = !ledState;
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
  FastLED.addLeds<WS2812,4,GRB>(leds, NUM_LEDS).setCorrection(0xFFE0F0);

  // set master brightness control
  FastLED.setBrightness(SETTINGS.power ? (SETTINGS.brightness * (uint16_t)255 / 100) : 0);
  
  Serial.println("Startup finished.");
  digitalWrite(LED_BUILTIN, 1);
}


void loop()
{
  unsigned long tic = millis();
  // Render LED animation
  gPatterns[SETTINGS.currentPattern]();
  FastLED.show();

  // Handle mDNS and HTTP server
  MDNS.update();
  server.handleClient();

  // Wait remaining time to get constant framerate
  unsigned long duration = millis() - tic;
  FastLED.delay((FRAME_LENGTH > duration) ? FRAME_LENGTH - duration : 0);
}
