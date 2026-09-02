#include <Arduino.h>
#include <esp_task_wdt.h>
#include "Alarm.h"
#include "Config.h"
#include "Logger.h"
#include "Radar.h"
#include "ServoControl.h"
#include "Ultrasonic.h"
#include "WebServer.h"
#include "WebSocket.h"
#include "WifiManager.h"
ServoControl servoControl;Ultrasonic ultrasonic;Alarm alarmSystem;Logger logger;WifiManager wifiManager;Radar radar(servoControl,ultrasonic,alarmSystem,logger);RadarWebServer webServer;RadarWebSocket webSocket(radar,alarmSystem,logger,wifiManager);uint32_t lastStatusBroadcastMs=0;
void setup(){Serial.begin(115200);Config::begin();servoControl.begin(Config::PIN_SERVO);ultrasonic.begin(Config::PIN_TRIG,Config::PIN_ECHO);alarmSystem.begin(Config::PIN_BUZZER,Config::PIN_LED);radar.begin();radar.setReadingCallback([](const RadarReading& reading){webSocket.broadcastReading(reading);});wifiManager.begin();if(!webServer.begin())Serial.println("[FATAL] LittleFS indisponível. Grave a imagem com uploadfs.");webSocket.begin(webServer.native());webServer.start();esp_task_wdt_init(8,true);esp_task_wdt_add(nullptr);radar.start();Serial.printf("[Sistema] Heap livre: %u bytes | PSRAM: %u bytes\n",ESP.getFreeHeap(),ESP.getPsramSize());}
void loop(){esp_task_wdt_reset();wifiManager.update();radar.update(wifiManager.rssi());alarmSystem.updateOutputs();webSocket.update();const uint32_t now=millis();if(now-lastStatusBroadcastMs>=Config::STATUS_BROADCAST_MS){lastStatusBroadcastMs=now;webSocket.broadcastStatus();}yield();}
