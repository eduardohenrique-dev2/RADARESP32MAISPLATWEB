#pragma once
#include <Arduino.h>
#include <WiFi.h>
class WifiManager { public: void begin(); void update(); void requestStationReconnect(); bool stationConnected()const{return WiFi.status()==WL_CONNECTED;} int16_t rssi()const; String apIp()const; String stationIp()const; uint8_t apClients()const; private: void connectStation(); bool _stationConfigured=false; bool _reconnectRequested=false; uint32_t _lastStationAttemptMs=0; };
