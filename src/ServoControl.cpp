#include "ServoControl.h"
#include "Config.h"
void ServoControl::begin(uint8_t pin) { _pin = pin; _servo.setPeriodHertz(50); _servo.attach(_pin, Config::SERVO_MIN_US, Config::SERVO_MAX_US); _attached = _servo.attached(); _currentAngle = 90; _targetAngle = 90; _moveIntervalMs = Config::settings.servoMoveIntervalMs; if (_attached) _servo.write(_currentAngle); _lastMoveMs = millis(); }
void ServoControl::setMoveInterval(uint16_t intervalMs) { _moveIntervalMs = constrain(intervalMs, 5, 100); }
void ServoControl::setTarget(int angle) { _targetAngle = constrain(angle, 0, 180); }
void ServoControl::center() { setTarget(90); }
void ServoControl::update() { if (!_attached || atTarget()) return; const uint32_t now = millis(); if (now - _lastMoveMs < _moveIntervalMs) return; _lastMoveMs = now; _currentAngle += (_targetAngle > _currentAngle) ? 1 : -1; _servo.write(_currentAngle); }
