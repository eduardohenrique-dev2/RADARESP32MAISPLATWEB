#pragma once
#include <Arduino.h>
#include <functional>
#include "Alarm.h"
#include "Logger.h"
#include "ServoControl.h"
#include "Ultrasonic.h"
struct RadarReading { uint32_t timestamp=0; int angle=90; float distanceCm=-1.0f; bool valid=false; bool alarm=false; int16_t rssi=0; const char* event="SCAN"; };
struct RadarStats { uint32_t readings=0; uint32_t alarms=0; uint32_t objectsDetected=0; float minDistanceCm=0.0f; float maxDistanceCm=0.0f; float lastDistanceCm=-1.0f; int lastAngle=90; bool sensorOk=false; };
class Radar { public: Radar(ServoControl& servo,Ultrasonic& ultrasonic,Alarm& alarm,Logger& logger); void begin(); void update(int16_t rssi); void start(); void stop(); void center(); void restart(); void clearStats(); void applySettings(); void setReadingCallback(std::function<void(const RadarReading&)> callback){_readingCallback=callback;} bool running()const{return _running;} int direction()const{return _direction;} const RadarStats& stats()const{return _stats;} private: void processMeasurement(const UltrasonicResult& result,int16_t rssi); void advanceTarget(); ServoControl& _servo; Ultrasonic& _ultrasonic; Alarm& _alarm; Logger& _logger; bool _running=false; int _direction=1; int _nextTarget=90; bool _measurementRequested=false; RadarStats _stats; std::function<void(const RadarReading&)> _readingCallback; };
