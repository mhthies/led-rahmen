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

struct Settings {
  bool power = true;
  uint8_t brightness = 50;
  uint8_t currentPattern = 0;

  uint8_t colorful_speed = 50;
  uint8_t colorful_sat = 96;

  uint8_t rainbow_fade_speed = 50;
  uint8_t rainbow_fade_sat = 96;
  uint8_t rainbow_fade_size = 50;
  uint16_t rainbow_fade_direction = 45;

  uint8_t sparkles_sat = 80;
  uint8_t sparkles_fade = 3;
  uint8_t sparkles_rate = 30;
};
