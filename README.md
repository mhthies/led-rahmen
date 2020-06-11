
# LED-Rahmen – ESP8266 Arduino-based Software for the WS2912B Backlight of an Anlogue Photo Frame

This Arduino sketch has been build to control the backlight of an analogue photo frame which was a Christmas present for my parents.
The backlight consists of two WS2812B LED stripes (left and right), with 14 LEDS each, and a WeMos D1 board with an ESP8266 microcontroller.

The software supports multiple animations (“Patterns”) which can be switched and configured via a simple HTTP web interface on the controller.
It comprises a simple JSON-based REST API (one single GET/POST endpoint), as well as a static index page with some JavaScript to provide a UI to interact with the API.

Required Arduino libraries:
* `FastLED` for rendering and showing the animations
* `ArduinoJSON` for the REST API
* `ESP8266WiFi`, `ESP8266WebServer` and `ESP8266mDNS`, which are shipped with the ESP8266 Board support package, for networking and network services


## License

Feel free to use the Code under the terms of the Apache License 2.0.

In short: Take this software for whatever purpose you want. You may even redistribute it, but in this case, please include a notice which refers to me as the original author and mark all sections that you modified.
See NOTICE and LICENSE files for detailed information.


## Project Structure

The project is an Arduino sketch with a `.ino` main file, defining the `setup()` and `loop()` functions.
In a separate C++ file `patterns.cpp`, the animation patterns are defined.
They form an array of functions, named `gPatterns`, which is used in the main sketch as an `extern` variable.
In the opposite direction, the global `SETTINGS` struct from the main file is used by the pattern functions to access the current dynamic configuration.

Additional header files are used for auxiliary purposes:
* `config.h` contains the (hardcoded) WiFi connection data (SSID, PSK, hostname).
* `constants.h` system configuration: Number of LEDs and target FPS
* `settings.hpp` definition of the `SETTINGS` struct (which is reused in both compile units)

The static documents to be served by the HTTP server are contained in the `webdata/` directory.
To compile them into the firmware binary, a C source file `webdata.h` needs to be generated with a BLOB variable for the contents of each static file.
This can be achieved with the Unix utility program `xdd`.

During the `setup()` phase, the controller connects to the Wifi according to the specified configuration.
Afterwards, the mDNS responder and the Webserver are configured and started, including the HTTP endpoints and their handler functions.
These functions (`handleRoot()` and `handleData()`) handle delivery of the static index page and reading/updating the `SETTINGS` via the REST API.

The main `loop()` renders one frame of the LED animation per execution.
Afterwards, it lets the mDNS and HTTP servers do their business, i.e. respond to mDNS messages and handle one HTTP client request, if any.
At the end, the loop ensures a constant framerate by waiting for the remaining time of one frame duration.
This is done by `FastLED.delay()` instead of `delay()`, which does temporal dithering of LED values in the waiting period to mix dimmed colours more precisely.


## Building

Before building the Arduino sketch, the `webdata.h` file must be genered from the static web files `webdata/`:

```bash
xxd -i webdata/* > webdata.h
```

Additionally, the `config.h` file must be created and adapted to the WiFi environment.
For this purpose, copy the `config.h.sample` to `config.h` and change the variables accordingly.

After these two perparations, you can configure the Arduino IDE for the target board (in my case, a WeMos D1, 160 MHz, no logging), and build and upload the sketch.
Once, the initial setup of the Arduino IDE is done, the sketch can also be compiled from the command line: 

```bash
arduino --verify LED-Rahmen.ino
# or
arduino --upload LED-Rahmen.ino
```

To simplify the two steps of generating the `webdata.h` and building the sketch, a Makefile is provided.
Using GNU Make, the following commands will work:

```bash
make
# or
make upload
```


## HTTP API

The HTTP JSON REST API has a single endpoint with `GET` and `POST` methods supported:

### `GET /api/data`

returns all current settings in a JSON object:

```json
{
    "power": true,
    "brightness": 100,
    "currentPattern": 3,
    "colorful_speed": 100,
    "colorful_sat": 100,
    "rainbow_fade_speed": 100,
    "rainbow_fade_sat": 100,
    "rainbow_fade_size": 100,
    "rainbow_fade_direction": 360,
    "sparkles_fade": 100,
    "sparkles_sat": 100,
    "sparkles_rate": 100,
}
```

The attributes have the following semantics:

* `power`: Global switch to power on the LED stripes. If `false` the LEDs are statically set to black
* `brightness`: Master brightness control (0–100)
* `currentPattern`: Pattern selection. Index (0–3) of the pattern in the `gPatterns` array
* `colorful_speed`: Speed of the "colorful" pattern, 0–100
* `colorful_sat`: Color satuation of the "colorful" pattern, 0–100
* `rainbow_fade_speed`: Speed of the "rainbow fade" pattern, 0–100
* `rainbow_fade_sat`: Color satuation of the "rainbow fade" pattern, 0–100
* `rainbow_fade_size`: Width of color gradient of the "rainbow fade" pattern, in stripe length per tens of full rainbows, 0–100
* `rainbow_fade_direction`: Angle of color gradient in "rainbow fade" pattern, 0–360 degrees
* `sparkles_fade`: Speed of fade out of "sparkles" pattern, 0–100
* `sparkles_sat`: Color satuation of sparkles in "sparkles" pattern
* `sparkles_rate`: Probability of creation of a new sparkle per frame, 0–100%


### `POST /api/data`

is used to update any subset of the settings.
For this purpose a JSON object in the same format as shown above with the attributes to be updated must be sent as form data in the request.
