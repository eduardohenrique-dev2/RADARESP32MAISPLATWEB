#pragma once

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

struct AlarmTransition {
  bool active = false;
  bool activated = false;
  bool cleared = false;
};

class Alarm {
 public:
  void begin(uint8_t buzzerPin, uint8_t ledPin);
  void updateOutputs();
  AlarmTransition process(float distanceCm, bool valid);
  void clear();
  void setDetectionEnabled(bool enabled);
  void setBuzzerEnabled(bool enabled);
  void setLedEnabled(bool enabled);
  void setManualLed(bool on);
  void setThreshold(float thresholdCm);

  bool active() const { return _active; }
  bool detectionEnabled() const { return _detectionEnabled; }
  bool buzzerEnabled() const { return _buzzerEnabled; }
  bool ledEnabled() const { return _ledEnabled; }

 private:
  void applyBuzzer(bool on);
  void setRgb(uint8_t red, uint8_t green, uint8_t blue);

  uint8_t _buzzerPin = 255;
  uint8_t _ledPin = 255;
  Adafruit_NeoPixel _pixel{1, -1, NEO_GRB + NEO_KHZ800};

  bool _active = false;
  bool _detectionEnabled = true;
  bool _buzzerEnabled = true;
  bool _ledEnabled = true;
  bool _manualLed = false;

  float _thresholdCm = 30.0f;
  float _hysteresisCm = 5.0f;
  uint32_t _lastAlarmStateChangeMs = 0;
};
