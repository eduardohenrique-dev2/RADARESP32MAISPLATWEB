#pragma once
#include <Arduino.h>
struct UltrasonicResult { bool valid=false; float distanceCm=-1.0f; float rawDistanceCm=-1.0f; uint32_t timestampMs=0; };
class Ultrasonic {
 public: void begin(uint8_t trigPin,uint8_t echoPin); void setMaxDistance(float maxDistanceCm); bool requestMeasurement(); void update(); bool busy() const { return _waiting; } bool hasNewResult() const { return _newResult; } UltrasonicResult consumeResult();
 private: static void IRAM_ATTR isrRouter(void* arg); void IRAM_ATTR onEchoChange(); float filteredDistance(float value); void finishInvalid(); uint8_t _trigPin=255,_echoPin=255; volatile uint32_t _echoRiseUs=0,_echoFallUs=0; volatile bool _pulseReady=false; volatile bool _waiting=false; bool _newResult=false; uint32_t _triggerAtUs=0; float _maxDistanceCm=300.0f; float _filter[5]={0,0,0,0,0}; uint8_t _filterCount=0,_filterIndex=0; UltrasonicResult _result;
};
