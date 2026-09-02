#include "Config.h"
#include <Preferences.h>

namespace Config {
RuntimeSettings settings;
namespace {
constexpr char PREF_NAMESPACE[] = "radar";
template <typename T> T clampValue(T value, T low, T high) { return value < low ? low : (value > high ? high : value); }
String trimLength(String value, size_t maxLen) { value.trim(); if (value.length() > maxLen) value.remove(maxLen); return value; }
}

bool isValidCloudIdentity(const String& value) {
  if (value.length() < 3 || value.length() > 64) return false;
  for (size_t i = 0; i < value.length(); ++i) {
    const char c = value[i];
    const bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.';
    if (!ok) return false;
  }
  return true;
}

String normalizeCloudHost(String host) {
  host.trim(); host.replace("https://", ""); host.replace("http://", ""); host.replace("wss://", ""); host.replace("ws://", "");
  const int slash = host.indexOf('/'); if (slash >= 0) host.remove(slash);
  return trimLength(host, 120);
}

void sanitize() {
  settings.servoMinAngle = clampValue(settings.servoMinAngle, 0, 170);
  settings.servoMaxAngle = clampValue(settings.servoMaxAngle, 10, 180);
  if (settings.servoMinAngle >= settings.servoMaxAngle) { settings.servoMinAngle = 0; settings.servoMaxAngle = 180; }
  settings.servoStep = clampValue(settings.servoStep, 1, 15);
  settings.servoMoveIntervalMs = clampValue<uint16_t>(settings.servoMoveIntervalMs, 5, 100);
  settings.maxDistanceCm = clampValue(settings.maxDistanceCm, 30.0f, 400.0f);
  settings.alarmDistanceCm = clampValue(settings.alarmDistanceCm, 5.0f, settings.maxDistanceCm);
  settings.staSsid = trimLength(settings.staSsid, 32); settings.staPassword = trimLength(settings.staPassword, 63);
  settings.cloudHost = normalizeCloudHost(settings.cloudHost); settings.cloudDeviceId = trimLength(settings.cloudDeviceId, 64); settings.cloudToken = trimLength(settings.cloudToken, 64);
  if (!isValidCloudIdentity(settings.cloudDeviceId)) settings.cloudDeviceId = "smart-radar-01";
  if (!isValidCloudIdentity(settings.cloudToken) || settings.cloudToken.length() < 8) settings.cloudToken = "smart-radar-demo-2026";
  if (settings.cloudHost.length() == 0) settings.cloudEnabled = false;
}

void begin() {
  Preferences prefs; if (!prefs.begin(PREF_NAMESPACE, true)) { sanitize(); return; }
  settings.servoMinAngle = prefs.getInt("smin", settings.servoMinAngle); settings.servoMaxAngle = prefs.getInt("smax", settings.servoMaxAngle); settings.servoStep = prefs.getInt("sstep", settings.servoStep); settings.servoMoveIntervalMs = prefs.getUShort("sspeed", settings.servoMoveIntervalMs);
  settings.maxDistanceCm = prefs.getFloat("dmax", settings.maxDistanceCm); settings.alarmDistanceCm = prefs.getFloat("dalarm", settings.alarmDistanceCm);
  settings.buzzerEnabled = prefs.getBool("buzz", settings.buzzerEnabled); settings.ledEnabled = prefs.getBool("led", settings.ledEnabled); settings.alarmEnabled = prefs.getBool("alarm", settings.alarmEnabled);
  settings.staSsid = prefs.getString("sta_ssid", settings.staSsid); settings.staPassword = prefs.getString("sta_pass", settings.staPassword);
  settings.cloudEnabled = prefs.getBool("cloud_en", settings.cloudEnabled); settings.cloudHost = prefs.getString("cloud_host", settings.cloudHost); settings.cloudDeviceId = prefs.getString("device_id", settings.cloudDeviceId); settings.cloudToken = prefs.getString("token", settings.cloudToken);
  prefs.end(); sanitize();
}

bool save() {
  sanitize(); Preferences prefs; if (!prefs.begin(PREF_NAMESPACE, false)) return false; bool ok = true;
  ok &= prefs.putInt("smin", settings.servoMinAngle) > 0; ok &= prefs.putInt("smax", settings.servoMaxAngle) > 0; ok &= prefs.putInt("sstep", settings.servoStep) > 0; ok &= prefs.putUShort("sspeed", settings.servoMoveIntervalMs) > 0;
  ok &= prefs.putFloat("dmax", settings.maxDistanceCm) > 0; ok &= prefs.putFloat("dalarm", settings.alarmDistanceCm) > 0;
  ok &= prefs.putBool("buzz", settings.buzzerEnabled) > 0; ok &= prefs.putBool("led", settings.ledEnabled) > 0; ok &= prefs.putBool("alarm", settings.alarmEnabled) > 0;
  ok &= prefs.putString("sta_ssid", settings.staSsid) > 0 || settings.staSsid.isEmpty(); ok &= prefs.putString("sta_pass", settings.staPassword) > 0 || settings.staPassword.isEmpty();
  ok &= prefs.putBool("cloud_en", settings.cloudEnabled) > 0; ok &= prefs.putString("cloud_host", settings.cloudHost) > 0 || settings.cloudHost.isEmpty(); ok &= prefs.putString("device_id", settings.cloudDeviceId) > 0; ok &= prefs.putString("token", settings.cloudToken) > 0;
  prefs.end(); return ok;
}

void resetToDefaults() { settings = RuntimeSettings(); save(); }
}  // namespace Config
