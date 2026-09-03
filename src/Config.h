#pragma once

#include <Arduino.h>

namespace Config {

constexpr uint8_t PIN_TRIG = 4;
constexpr uint8_t PIN_ECHO = 5;
constexpr uint8_t PIN_SERVO = 6;
constexpr uint8_t PIN_BUZZER = 7;
constexpr uint8_t PIN_LED = 21;  // WS2812 RGB onboard da Waveshare ESP32-S3-Zero

constexpr char AP_SSID[] = "SmartRadar";
constexpr char AP_PASSWORD[] = "SmartRadar2026";
constexpr uint8_t AP_CHANNEL = 6;
constexpr uint8_t AP_MAX_CLIENTS = 4;

constexpr uint16_t SERVO_MIN_US = 500;
constexpr uint16_t SERVO_MAX_US = 2500;
constexpr uint16_t SERVO_SETTLE_MS = 28;

constexpr uint32_t ULTRASONIC_TIMEOUT_US = 30000;
constexpr float ULTRASONIC_MIN_CM = 2.0f;
constexpr uint8_t ULTRASONIC_FILTER_SIZE = 5;

constexpr uint16_t MAX_LOG_ENTRIES = 300;
constexpr uint16_t CSV_CHUNK_SIZE = 900;

constexpr uint32_t WIFI_RETRY_MS = 15000;
constexpr uint32_t STATUS_BROADCAST_MS = 1000;
constexpr uint32_t CLOUD_RETRY_MS = 5000;

struct RuntimeSettings {
  int servoMinAngle = 0;
  int servoMaxAngle = 180;
  int servoStep = 3;
  uint16_t servoMoveIntervalMs = 15;

  float maxDistanceCm = 300.0f;
  float alarmDistanceCm = 30.0f;

  bool buzzerEnabled = true;
  bool ledEnabled = true;
  bool alarmEnabled = true;

  String staSsid = "";
  String staPassword = "";

  bool cloudEnabled = false;
  String cloudHost = "";
  String cloudDeviceId = "smart-radar-01";
  String cloudToken = "smart-radar-demo-2026";
};

extern RuntimeSettings settings;

void begin();
void sanitize();
bool save();
void resetToDefaults();

bool isValidCloudIdentity(const String& value);
String normalizeCloudHost(String host);

}  // namespace Config
