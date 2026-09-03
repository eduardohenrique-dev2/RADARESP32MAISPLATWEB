#include "WebSocket.h"
#include "Config.h"

namespace {
String toJson(JsonDocument& document) {
  String output;
  serializeJson(document, output);
  return output;
}

bool validInt(JsonVariantConst value, int minimum, int maximum, int& output) {
  if (!value.is<int>() && !value.is<long>()) return false;
  output = value.as<int>();
  return output >= minimum && output <= maximum;
}

bool validFloat(JsonVariantConst value, float minimum, float maximum, float& output) {
  if (!value.is<float>() && !value.is<int>() && !value.is<long>()) return false;
  output = value.as<float>();
  return isfinite(output) && output >= minimum && output <= maximum;
}
}  // namespace

RadarWebSocket* RadarWebSocket::_instance = nullptr;

RadarWebSocket::RadarWebSocket(Radar& radar, Alarm& alarm, Logger& logger, WifiManager& wifi)
    : _localWs("/ws"), _radar(radar), _alarm(alarm), _logger(logger), _wifi(wifi) {}

void RadarWebSocket::begin(AsyncWebServer& server) {
  _instance = this;

  _localWs.onEvent([this](AsyncWebSocket* ws,
                          AsyncWebSocketClient* client,
                          AwsEventType type,
                          void* arg,
                          uint8_t* data,
                          size_t len) {
    onLocalEvent(ws, client, type, arg, data, len);
  });

  server.addHandler(&_localWs);

  _cloudWs.onEvent(RadarWebSocket::cloudEventRouter);
  _cloudWs.setReconnectInterval(Config::CLOUD_RETRY_MS);
  _cloudWs.enableHeartbeat(15000, 3000, 2);
}

void RadarWebSocket::onLocalEvent(AsyncWebSocket*,
                                  AsyncWebSocketClient* client,
                                  AwsEventType type,
                                  void* arg,
                                  uint8_t* data,
                                  size_t len) {
  if (type == WS_EVT_CONNECT) {
    sendConfig(client, false);
    broadcastStatus();
    return;
  }

  if (type != WS_EVT_DATA) return;

  AwsFrameInfo* info = static_cast<AwsFrameInfo*>(arg);
  if (!info || !info->final || info->index != 0 || info->len != len ||
      info->opcode != WS_TEXT || len == 0 || len > 4096) {
    sendError("Mensagem WebSocket inválida.", client, false);
    return;
  }

  handleIncoming(data, len, client, false);
}

void RadarWebSocket::handleIncoming(const uint8_t* data,
                                    size_t len,
                                    AsyncWebSocketClient* client,
                                    bool cloud) {
  JsonDocument document;
  if (deserializeJson(document, data, len)) {
    sendError("JSON inválido.", client, cloud);
    return;
  }

  const String type = document["type"] | "";

  if (type == "command") {
    handleCommand(document["command"] | "", client, cloud);
  } else if (type == "settings") {
    JsonObjectConst settings = document["settings"].as<JsonObjectConst>();
    if (settings.isNull()) {
      sendError("Objeto settings ausente.", client, cloud);
    } else {
      handleSettings(settings, client, cloud);
    }
  } else if (type == "ping") {
    JsonDocument pong;
    pong["type"] = "pong";
    pong["timestamp"] = millis();
    sendText(toJson(pong), client, cloud);
  } else {
    sendError("Tipo de mensagem não suportado.", client, cloud);
  }
}

void RadarWebSocket::handleCommand(const String& command,
                                   AsyncWebSocketClient* client,
                                   bool cloud) {
  if (command == "start") {
    _radar.start();
  } else if (command == "stop") {
    _radar.stop();
  } else if (command == "center") {
    _radar.center();
  } else if (command == "restart") {
    _radar.restart();
  } else if (command == "alarm_on") {
    _alarm.setDetectionEnabled(true);
    Config::save();
  } else if (command == "alarm_off") {
    _alarm.setDetectionEnabled(false);
    Config::save();
  } else if (command == "led_on") {
    _alarm.setManualLed(true);
  } else if (command == "led_off") {
    _alarm.setManualLed(false);
  } else if (command == "clear_logs") {
    _logger.clear();
  } else if (command == "export_logs") {
    exportLogs(client, cloud);
    return;
  } else if (command == "get_config") {
    sendConfig(client, cloud);
    return;
  } else if (command != "clear_radar") {
    sendError("Comando não reconhecido.", client, cloud);
    return;
  }

  sendAck("Comando executado.", client, cloud);
  broadcastStatus();
}

void RadarWebSocket::handleSettings(JsonObjectConst settings,
                                    AsyncWebSocketClient* client,
                                    bool cloud) {
  int intValue;
  float floatValue;

  int servoMin = Config::settings.servoMinAngle;
  int servoMax = Config::settings.servoMaxAngle;

  if (!settings["servoMinAngle"].isNull() &&
      !validInt(settings["servoMinAngle"], 0, 170, servoMin)) {
    sendError("Ângulo mínimo inválido.", client, cloud);
    return;
  }

  if (!settings["servoMaxAngle"].isNull() &&
      !validInt(settings["servoMaxAngle"], 10, 180, servoMax)) {
    sendError("Ângulo máximo inválido.", client, cloud);
    return;
  }

  if (servoMin >= servoMax) {
    sendError("Ângulo mínimo deve ser menor que o máximo.", client, cloud);
    return;
  }

  Config::settings.servoMinAngle = servoMin;
  Config::settings.servoMaxAngle = servoMax;

  if (!settings["servoStep"].isNull()) {
    if (!validInt(settings["servoStep"], 1, 15, intValue)) {
      sendError("Passo inválido.", client, cloud);
      return;
    }
    Config::settings.servoStep = intValue;
  }

  if (!settings["servoMoveIntervalMs"].isNull()) {
    if (!validInt(settings["servoMoveIntervalMs"], 5, 100, intValue)) {
      sendError("Velocidade inválida.", client, cloud);
      return;
    }
    Config::settings.servoMoveIntervalMs = intValue;
  }

  if (!settings["maxDistanceCm"].isNull()) {
    if (!validFloat(settings["maxDistanceCm"], 30, 400, floatValue)) {
      sendError("Distância máxima inválida.", client, cloud);
      return;
    }
    Config::settings.maxDistanceCm = floatValue;
  }

  if (!settings["alarmDistanceCm"].isNull()) {
    if (!validFloat(settings["alarmDistanceCm"], 5, 400, floatValue)) {
      sendError("Distância de alarme inválida.", client, cloud);
      return;
    }
    Config::settings.alarmDistanceCm = floatValue;
  }

  if (Config::settings.alarmDistanceCm > Config::settings.maxDistanceCm) {
    sendError("Alarme não pode superar distância máxima.", client, cloud);
    return;
  }

  if (!settings["buzzerEnabled"].isNull()) {
    Config::settings.buzzerEnabled = settings["buzzerEnabled"].as<bool>();
  }
  if (!settings["ledEnabled"].isNull()) {
    Config::settings.ledEnabled = settings["ledEnabled"].as<bool>();
  }
  if (!settings["alarmEnabled"].isNull()) {
    Config::settings.alarmEnabled = settings["alarmEnabled"].as<bool>();
  }

  bool wifiChanged = false;
  bool cloudChanged = false;

  if (!settings["staSsid"].isNull()) {
    String value = settings["staSsid"].as<String>();
    if (value.length() > 32) {
      sendError("SSID inválido.", client, cloud);
      return;
    }
    wifiChanged = value != Config::settings.staSsid;
    Config::settings.staSsid = value;
  }

  if (!settings["staPassword"].isNull()) {
    String value = settings["staPassword"].as<String>();
    if (!value.isEmpty() && (value.length() < 8 || value.length() > 63)) {
      sendError("Senha Wi-Fi inválida.", client, cloud);
      return;
    }
    if (!value.isEmpty()) {
      wifiChanged |= value != Config::settings.staPassword;
      Config::settings.staPassword = value;
    }
  }

  if (!settings["cloudEnabled"].isNull()) {
    const bool value = settings["cloudEnabled"].as<bool>();
    cloudChanged |= value != Config::settings.cloudEnabled;
    Config::settings.cloudEnabled = value;
  }

  if (!settings["cloudHost"].isNull()) {
    String value = Config::normalizeCloudHost(settings["cloudHost"].as<String>());
    cloudChanged |= value != Config::settings.cloudHost;
    Config::settings.cloudHost = value;
  }

  if (!settings["cloudDeviceId"].isNull()) {
    String value = settings["cloudDeviceId"].as<String>();
    if (!Config::isValidCloudIdentity(value)) {
      sendError("Device ID inválido.", client, cloud);
      return;
    }
    cloudChanged |= value != Config::settings.cloudDeviceId;
    Config::settings.cloudDeviceId = value;
  }

  if (!settings["cloudToken"].isNull()) {
    String value = settings["cloudToken"].as<String>();
    if (value.length() < 8 || !Config::isValidCloudIdentity(value)) {
      sendError("Token inválido.", client, cloud);
      return;
    }
    cloudChanged |= value != Config::settings.cloudToken;
    Config::settings.cloudToken = value;
  }

  Config::sanitize();

  if (!Config::save()) {
    sendError("Falha ao salvar configurações.", client, cloud);
    return;
  }

  _radar.applySettings();

  if (wifiChanged) {
    _wifi.requestStationReconnect();
  }

  if (cloudChanged) {
    _cloudEnabledSnapshot = !Config::settings.cloudEnabled;
  }

  sendAck("Configurações salvas.", client, cloud);
  sendConfig(client, cloud);
  broadcastStatus();
}

void RadarWebSocket::sendText(String payload, AsyncWebSocketClient* client, bool cloud) {
  if (cloud) {
    if (_cloudConnected) {
      _cloudWs.sendTXT(payload);
    }
    return;
  }

  if (client) {
    client->text(payload);
  } else {
    _localWs.textAll(payload);
  }
}

void RadarWebSocket::sendAck(const String& message,
                             AsyncWebSocketClient* client,
                             bool cloud) {
  JsonDocument document;
  document["type"] = "ack";
  document["message"] = message;
  sendText(toJson(document), client, cloud);
}

void RadarWebSocket::sendError(const String& message,
                               AsyncWebSocketClient* client,
                               bool cloud) {
  JsonDocument document;
  document["type"] = "error";
  document["message"] = message;
  sendText(toJson(document), client, cloud);
}

void RadarWebSocket::sendConfig(AsyncWebSocketClient* client, bool cloud) {
  JsonDocument document;
  document["type"] = "config";
  JsonObject config = document["config"].to<JsonObject>();

  config["servoMinAngle"] = Config::settings.servoMinAngle;
  config["servoMaxAngle"] = Config::settings.servoMaxAngle;
  config["servoStep"] = Config::settings.servoStep;
  config["servoMoveIntervalMs"] = Config::settings.servoMoveIntervalMs;
  config["maxDistanceCm"] = Config::settings.maxDistanceCm;
  config["alarmDistanceCm"] = Config::settings.alarmDistanceCm;
  config["buzzerEnabled"] = Config::settings.buzzerEnabled;
  config["ledEnabled"] = Config::settings.ledEnabled;
  config["alarmEnabled"] = Config::settings.alarmEnabled;
  config["staSsid"] = Config::settings.staSsid;
  config["cloudEnabled"] = Config::settings.cloudEnabled;
  config["cloudHost"] = Config::settings.cloudHost;
  config["cloudDeviceId"] = Config::settings.cloudDeviceId;
  config["cloudToken"] = Config::settings.cloudToken;

  sendText(toJson(document), client, cloud);
}

void RadarWebSocket::exportLogs(AsyncWebSocketClient* client, bool cloud) {
  const String transferId = String(millis(), HEX);

  JsonDocument begin;
  begin["type"] = "csv_begin";
  begin["transferId"] = transferId;
  begin["entries"] = _logger.size();
  sendText(toJson(begin), client, cloud);

  String chunk = _logger.csvHeader();
  chunk.reserve(Config::CSV_CHUNK_SIZE + 128);

  for (size_t index = 0; index < _logger.size(); ++index) {
    LogEntry entry;
    if (!_logger.getEntry(index, entry)) continue;

    const String line = _logger.csvLine(entry);

    if (chunk.length() + line.length() >= Config::CSV_CHUNK_SIZE) {
      JsonDocument data;
      data["type"] = "csv_chunk";
      data["transferId"] = transferId;
      data["data"] = chunk;
      sendText(toJson(data), client, cloud);
      chunk = "";
    }

    chunk += line;
  }

  if (!chunk.isEmpty()) {
    JsonDocument data;
    data["type"] = "csv_chunk";
    data["transferId"] = transferId;
    data["data"] = chunk;
    sendText(toJson(data), client, cloud);
  }

  JsonDocument end;
  end["type"] = "csv_end";
  end["transferId"] = transferId;
  sendText(toJson(end), client, cloud);
}

void RadarWebSocket::broadcastReading(const RadarReading& reading) {
  JsonDocument document;
  document["type"] = "radar";
  document["angle"] = reading.angle;
  document["distance"] = reading.valid ? reading.distanceCm : -1.0f;
  document["valid"] = reading.valid;
  document["alarm"] = reading.alarm;
  document["timestamp"] = reading.timestamp;
  document["event"] = reading.event;
  document["rssi"] = reading.rssi;

  String payload = toJson(document);
  _localWs.textAll(payload);

  if (_cloudConnected) {
    _cloudWs.sendTXT(payload);
  }
}

void RadarWebSocket::broadcastStatus() {
  const RadarStats& stats = _radar.stats();

  JsonDocument document;
  document["type"] = "status";
  document["running"] = _radar.running();
  document["manualMode"] = _radar.manualMode();
  document["alarmActive"] = _alarm.active();
  document["alarmEnabled"] = _alarm.detectionEnabled();
  document["cloudConnected"] = _cloudConnected;
  document["wifiStationConnected"] = _wifi.stationConnected();
  document["apClients"] = _wifi.apClients();
  document["rssi"] = _wifi.rssi();
  document["apIp"] = _wifi.apIp();
  document["stationIp"] = _wifi.stationIp();
  document["uptime"] = millis();
  document["freeHeap"] = ESP.getFreeHeap();
  document["psram"] = ESP.getPsramSize();
  document["freePsram"] = ESP.getFreePsram();
  document["readings"] = stats.readings;
  document["alarms"] = stats.alarms;
  document["objectsDetected"] = stats.objectsDetected;
  document["minDistance"] = stats.minDistanceCm;
  document["maxDistance"] = stats.maxDistanceCm;
  document["lastDistance"] = stats.lastDistanceCm;
  document["angle"] = stats.lastAngle;
  document["sensorOk"] = stats.sensorOk;

  String payload = toJson(document);
  _localWs.textAll(payload);

  if (_cloudConnected) {
    _cloudWs.sendTXT(payload);
  }
}

void RadarWebSocket::startCloud() {
  if (!_wifi.stationConnected() || !Config::settings.cloudEnabled ||
      Config::settings.cloudHost.isEmpty()) {
    return;
  }

  _cloudHostSnapshot = Config::settings.cloudHost;
  _cloudDeviceSnapshot = Config::settings.cloudDeviceId;
  _cloudTokenSnapshot = Config::settings.cloudToken;
  _cloudEnabledSnapshot = Config::settings.cloudEnabled;

  String path = "/api/ws?role=device&deviceId=" + _cloudDeviceSnapshot +
                "&token=" + _cloudTokenSnapshot;

  _cloudWs.beginSSL(_cloudHostSnapshot.c_str(), 443, path.c_str());
  _cloudWs.setReconnectInterval(Config::CLOUD_RETRY_MS);
  _cloudWs.enableHeartbeat(15000, 3000, 2);

  _cloudInitialized = true;
  _lastCloudAttemptMs = millis();
}

void RadarWebSocket::stopCloud() {
  if (_cloudInitialized) {
    _cloudWs.disconnect();
  }

  _cloudInitialized = false;
  _cloudConnected = false;
}

void RadarWebSocket::syncCloud() {
  const bool desired = Config::settings.cloudEnabled && !Config::settings.cloudHost.isEmpty();

  const bool changed = desired != _cloudEnabledSnapshot ||
                       Config::settings.cloudHost != _cloudHostSnapshot ||
                       Config::settings.cloudDeviceId != _cloudDeviceSnapshot ||
                       Config::settings.cloudToken != _cloudTokenSnapshot;

  if (!desired || !_wifi.stationConnected()) {
    if (_cloudInitialized) stopCloud();
    return;
  }

  if (changed && _cloudInitialized) {
    stopCloud();
  }

  if (!_cloudInitialized && millis() - _lastCloudAttemptMs >= 1000) {
    startCloud();
  }
}

void RadarWebSocket::cloudEventRouter(WStype_t type, uint8_t* payload, size_t length) {
  if (_instance) {
    _instance->onCloudEvent(type, payload, length);
  }
}

void RadarWebSocket::onCloudEvent(WStype_t type, uint8_t* payload, size_t length) {
  if (type == WStype_CONNECTED) {
    _cloudConnected = true;

    JsonDocument hello;
    hello["type"] = "device_hello";
    hello["deviceId"] = Config::settings.cloudDeviceId;
    hello["firmware"] = "1.1.0";
    hello["timestamp"] = millis();

    sendText(toJson(hello), nullptr, true);
    sendConfig(nullptr, true);
    broadcastStatus();
  } else if (type == WStype_DISCONNECTED || type == WStype_ERROR) {
    _cloudConnected = false;
  } else if (type == WStype_TEXT && payload && length > 0 && length <= 4096) {
    handleIncoming(payload, length, nullptr, true);
  }
}

void RadarWebSocket::update() {
  _localWs.cleanupClients();
  syncCloud();

  if (_cloudInitialized) {
    _cloudWs.loop();
  }
}
