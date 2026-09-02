#pragma once
#include <Arduino.h>
#include "Config.h"
struct LogEntry{uint32_t timestamp=0;int16_t angle=0;int16_t distanceCm=-1;bool alarm=false;char event[24]={0};int16_t rssi=0;};
class Logger{public:void clear();void log(uint32_t timestamp,int angle,float distanceCm,bool alarm,const char* event,int16_t rssi);size_t size()const{return _count;}bool getEntry(size_t chronologicalIndex,LogEntry& out)const;String csvHeader()const;String csvLine(const LogEntry& entry)const;private:LogEntry _entries[Config::MAX_LOG_ENTRIES];size_t _head=0,_count=0;};
