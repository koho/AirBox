#ifndef AQICN_H
#define AQICN_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "accumulator.h"
#include "config.h"

const int TREND_LEN = 300;
const int TREND_LIMIT = 5;
const int MAX_UPLOAD_INTERVAL = 25 * 60 * 1000;

class AQICN
{
public:
  struct DATA {
    float pm_1_0;
    float pm_2_5;
    float pm_10_0;
    #ifdef USE_TVOC
    float tvoc;
    #endif
  };
  AQICN(const char *name, float latitude, float longitude, const char *token, unsigned long interval = 30);
  AQICN(const char *id, const char *name, float latitude, float longitude, const char *token, unsigned long interval = 30);
  bool timeout();
  void feed(DATA& data, float trend);
  void reset();

private:
  const char *_id, *_name, *_token;
  float _latitude, _longitude;
  unsigned long _lastUpload;
  unsigned long _interval;
  float _trend[TREND_LEN];
  int _trend_index = 0;
  Accumulator _acc[6];
  bool upload();
  bool stable();
};
#endif