#include "Alarm.h"
#include "Config.h"

void Alarm::begin(uint8_t buzzerPin, uint8_t ledPin) {
  _buzzerPin = buzzerPin;
  _ledPin = ledPin;

  pinMode(_buzzerPin, OUTPUT);
  digitalWrite(_buzzerPin, LOW);

  _pixel.setPin(_ledPin);
  _pixel.begin();
  _pixel.setBrightness(48);
  _pixel.clear();
  _pixel.show();

  _detectionEnabled = Config::settings.alarmEnabled;
  _buzzerEnabled = Config::settings.buzzerEnabled;
  _ledEnabled = Config::settings.ledEnabled;
  _thresholdCm = Config::settings.alarmDistanceCm;
}

void Alarm::setDetectionEnabled(bool enabled) {
  _detectionEnabled = enabled;
  Config::settings.alarmEnabled = enabled;
  if (!enabled) clear();
}

void Alarm::setBuzzerEnabled(bool enabled) {
  _buzzerEnabled = enabled;
  Config::settings.buzzerEnabled = enabled;
  if (!enabled) applyBuzzer(false);
}

void Alarm::setLedEnabled(bool enabled) {
  _ledEnabled = enabled;
  Config::settings.ledEnabled = enabled;
  if (!enabled) setRgb(0, 0, 0);
}

void Alarm::setManualLed(bool on) {
  _manualLed = on;
}

void Alarm::setThreshold(float thresholdCm) {
  _thresholdCm = constrain(thresholdCm, 5.0f, 400.0f);
}

AlarmTransition Alarm::process(float distanceCm, bool valid) {
  AlarmTransition transition;

  if (!_detectionEnabled || !valid) {
    transition.active = _active;
    return transition;
  }

  if (!_active && distanceCm <= _thresholdCm) {
    _active = true;
    _lastAlarmStateChangeMs = millis();
    transition.activated = true;
  } else if (_active && distanceCm > (_thresholdCm + _hysteresisCm)) {
    _active = false;
    _lastAlarmStateChangeMs = millis();
    transition.cleared = true;
  }

  transition.active = _active;
  return transition;
}

void Alarm::clear() {
  _active = false;
  _lastAlarmStateChangeMs = millis();
  applyBuzzer(false);

  if (_ledEnabled && _manualLed) {
    setRgb(0, 255, 0);
  } else {
    setRgb(0, 0, 0);
  }
}

void Alarm::applyBuzzer(bool on) {
  digitalWrite(_buzzerPin, on ? HIGH : LOW);
}

void Alarm::setRgb(uint8_t red, uint8_t green, uint8_t blue) {
  _pixel.setPixelColor(0, _pixel.Color(red, green, blue));
  _pixel.show();
}

void Alarm::updateOutputs() {
  const uint32_t now = millis();

  bool buzzerOn = false;
  if (_active && _buzzerEnabled) {
    buzzerOn = ((now - _lastAlarmStateChangeMs) % 700) < 160;
  }
  applyBuzzer(buzzerOn);

  if (!_ledEnabled) {
    setRgb(0, 0, 0);
    return;
  }

  if (_active) {
    const bool flashOn = ((now - _lastAlarmStateChangeMs) % 400) < 200;
    if (flashOn) {
      setRgb(255, 0, 0);
    } else {
      setRgb(0, 0, 0);
    }
    return;
  }

  if (_manualLed) {
    setRgb(0, 255, 0);
  } else {
    setRgb(0, 0, 0);
  }
}
