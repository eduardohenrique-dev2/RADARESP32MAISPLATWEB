#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsClient.h>
#include "Alarm.h"
#include "Logger.h"
#include "Radar.h"
#include "WifiManager.h"

class RadarWebSocket {
 public:
  RadarWebSocket(Radar& radar, Alarm& alarm, Logger& logger, WifiManager& wifi);

  void begin(AsyncWebServer& server);
  void update();
  void broadcastReading(const RadarReading& reading);
  void broadcastStatus();
  bool cloudConnected() const { return _cloudConnected; }

 private:
  void onLocalEvent(AsyncWebSocket*, AsyncWebSocketClient*, AwsEventType, void*, uint8_t*, size_t);
  void handleIncoming(const uint8_t*, size_t, AsyncWebSocketClient*, bool);
  void handleCommand(const String&, AsyncWebSocketClient*, bool);
  void handleSettings(JsonObjectConst, AsyncWebSocketClient*, bool);

  void sendConfig(AsyncWebSocketClient* client = nullptr, bool cloudOnly = false);
  void sendAck(const String&, AsyncWebSocketClient*, bool);
  void sendError(const String&, AsyncWebSocketClient*, bool);
  void exportLogs(AsyncWebSocketClient*, bool);
  void sendText(String payload, AsyncWebSocketClient*, bool);

  void syncCloud();
  void startCloud();
  void stopCloud();
  static void cloudEventRouter(WStype_t, uint8_t*, size_t);
  void onCloudEvent(WStype_t, uint8_t*, size_t);

  static RadarWebSocket* _instance;
  AsyncWebSocket _localWs;
  WebSocketsClient _cloudWs;
  Radar& _radar;
  Alarm& _alarm;
  Logger& _logger;
  WifiManager& _wifi;

  bool _cloudInitialized = false;
  bool _cloudConnected = false;
  uint32_t _lastCloudAttemptMs = 0;
  String _cloudHostSnapshot;
  String _cloudDeviceSnapshot;
  String _cloudTokenSnapshot;
  bool _cloudEnabledSnapshot = false;
};
