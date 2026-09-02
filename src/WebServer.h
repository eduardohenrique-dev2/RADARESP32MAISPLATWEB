#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
class RadarWebServer { public: RadarWebServer(); bool begin(); void start(); AsyncWebServer& native(){return _server;} bool filesystemReady()const{return _filesystemReady;} private: AsyncWebServer _server; bool _filesystemReady=false; };
