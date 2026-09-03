#pragma once

#include <Arduino.h>
#include "Radar.h"

class JoystickControl {
 public:
  explicit JoystickControl(Radar& radar) : _radar(radar) {}

  void begin(uint8_t pinX, uint8_t pinY, uint8_t pinSwitch);
  void update();

  int rawX() const { return _rawX; }
  int rawY() const { return _rawY; }
  bool buttonPressed() const { return _stableButtonState == LOW; }

 private:
  void handleButton(uint32_t now);
  void handleManualAxis(uint32_t now);

  Radar& _radar;
  uint8_t _pinX = 255;
  uint8_t _pinY = 255;
  uint8_t _pinSwitch = 255;

  int _rawX = 2048;
  int _rawY = 2048;

  bool _lastRawButtonState = HIGH;
  bool _stableButtonState = HIGH;
  uint32_t _lastButtonChangeMs = 0;
  uint32_t _lastAxisUpdateMs = 0;
};
