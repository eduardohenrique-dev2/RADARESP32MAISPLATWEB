#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>
class ServoControl {
 public:
  void begin(uint8_t pin); void update(); void setTarget(int angle); void center(); void setMoveInterval(uint16_t intervalMs);
  int currentAngle() const { return _currentAngle; } int targetAngle() const { return _targetAngle; }
  bool atTarget() const { return _currentAngle == _targetAngle; } bool attached() const { return _attached; }
  uint32_t lastMoveMillis() const { return _lastMoveMs; }
 private:
  Servo _servo; uint8_t _pin = 255; bool _attached = false; int _currentAngle = 90; int _targetAngle = 90; uint16_t _moveIntervalMs = 15; uint32_t _lastMoveMs = 0;
};
