#include "JoystickControl.h"
#include "Config.h"

void JoystickControl::begin(uint8_t pinX, uint8_t pinY, uint8_t pinSwitch) {
  _pinX = pinX;
  _pinY = pinY;
  _pinSwitch = pinSwitch;

  pinMode(_pinSwitch, INPUT_PULLUP);
  analogReadResolution(12);

  _rawX = analogRead(_pinX);
  _rawY = analogRead(_pinY);
  _lastRawButtonState = digitalRead(_pinSwitch);
  _stableButtonState = _lastRawButtonState;

  Serial.printf("[Joystick] HW-504 pronto | VRX GPIO %u | VRY GPIO %u | SW GPIO %u\n",
                _pinX, _pinY, _pinSwitch);
}

void JoystickControl::handleButton(uint32_t now) {
  const bool rawState = digitalRead(_pinSwitch);

  if (rawState != _lastRawButtonState) {
    _lastRawButtonState = rawState;
    _lastButtonChangeMs = now;
  }

  if ((now - _lastButtonChangeMs) < Config::JOYSTICK_DEBOUNCE_MS) return;
  if (rawState == _stableButtonState) return;

  _stableButtonState = rawState;

  if (_stableButtonState == LOW) {
    if (_radar.manualMode()) {
      _radar.start();
      Serial.println("[Joystick] Modo AUTOMATICO");
    } else {
      _radar.enableManualMode();
      Serial.println("[Joystick] Modo MANUAL");
    }
  }
}

void JoystickControl::handleManualAxis(uint32_t now) {
  if (!_radar.manualMode()) return;
  if ((now - _lastAxisUpdateMs) < Config::JOYSTICK_UPDATE_MS) return;
  _lastAxisUpdateMs = now;

  _rawX = analogRead(_pinX);
  _rawY = analogRead(_pinY);

  const int delta = _rawX - static_cast<int>(Config::JOYSTICK_CENTER);
  const int magnitude = abs(delta);

  if (magnitude <= Config::JOYSTICK_DEADZONE) return;

  const int usable = max(1, static_cast<int>(Config::JOYSTICK_ADC_MAX / 2 - Config::JOYSTICK_DEADZONE));
  const int strength = constrain(magnitude - static_cast<int>(Config::JOYSTICK_DEADZONE), 0, usable);
  const int step = 1 + map(strength, 0, usable, 0, 5);

  int target = _radar.manualTargetAngle();
  if (delta < 0) {
    target -= step;
  } else {
    target += step;
  }

  _radar.setManualTarget(target);
}

void JoystickControl::update() {
  const uint32_t now = millis();
  handleButton(now);

  if (!_radar.manualMode()) {
    if ((now - _lastAxisUpdateMs) >= 150) {
      _lastAxisUpdateMs = now;
      _rawX = analogRead(_pinX);
      _rawY = analogRead(_pinY);
    }
    return;
  }

  handleManualAxis(now);
}
