#include "aqicn.h"

AQICN::AQICN(const char *id, const char *name, float latitude, float longitude, const char *token, unsigned long interval) {
  _id = id;
  _name = name;
  _token = token;
  _latitude = latitude;
  _longitude = longitude;
  _lastUpload = millis();
  _interval = interval * 60 * 1000;
  for (size_t i = 0; i < TREND_LEN; i++)
  {
    _trend[i] = NAN;
  }
}

AQICN::AQICN(const char *name, float latitude, float longitude, const char *token, unsigned long interval) {
  static char stationID[20] = { 0 };
  sprintf(stationID, "%x", ESP.getChipId());
  AQICN(stationID, name, latitude, longitude, token, interval);
}

bool AQICN::timeout() {
  return millis() - _lastUpload > _interval;
}

void AQICN::reset() {
  for (size_t i = 0; i < sizeof(_acc) / sizeof(_acc[0]); i++)
  {
    _acc[i].reset();
  }
}

void AQICN::feed(DATA& data, float trend) {
  _trend[_trend_index] = trend;
  _trend_index = (_trend_index + 1) % TREND_LEN;
  if (!timeout()) {
    return;
  }
  if (isnan(data.pm_2_5)) {
    reset();
    return;
  }
  if (!isnan(data.pm_1_0)) _acc[0].add(data.pm_1_0);
  if (!isnan(data.pm_2_5)) _acc[1].add(data.pm_2_5);
  if (!isnan(data.pm_10_0)) _acc[2].add(data.pm_10_0);

  #ifdef USE_TVOC
  if (!isnan(data.tvoc)) _acc[3].add(data.tvoc);
  #endif
  
  if (_acc[1].full()) {
    if ((stable() || millis() - _lastUpload > MAX_UPLOAD_INTERVAL) && upload()) {
      _lastUpload = millis();
    }
    reset();
  }
}

bool AQICN::stable() {
  float maxTrend = NAN;
  for (size_t i = 0; i < TREND_LEN; i++)
  {
    if (!isnan(_trend[i])) {
      float trend = _trend[i] < 0 ? -_trend[i] : _trend[i];
      if (isnan(maxTrend) || trend > maxTrend) {
        maxTrend = trend;
      }
    }
  }
  return !isnan(maxTrend) && maxTrend < TREND_LIMIT;
}

void setReading(JsonDocument& doc, int i, const char *specie, Accumulator& acc, const char *unit) {
  doc["readings"][i]["specie"] = specie;
  doc["readings"][i]["value"] = acc.avg();
  doc["readings"][i]["min"] = acc.min();
  doc["readings"][i]["max"] = acc.max();
  doc["readings"][i]["median"] = acc.median();
  doc["readings"][i]["stddev"] = acc.stddev();
  doc["readings"][i]["averaging"] = acc.count();
  doc["readings"][i]["unit"] = unit;
}

bool AQICN::upload() {
  WiFiClientSecure client;
  HTTPClient https;
  client.setInsecure();
  https.setTimeout(3000);
  https.setReuse(false);
  https.addHeader("Content-Type", "application/json");
  JsonDocument doc;
  
  doc["station"]["id"] = _id;
  doc["station"]["name"] = _name;
  doc["station"]["location"]["latitude"] = _latitude;
  doc["station"]["location"]["longitude"] = _longitude;

  int i = 0;
  setReading(doc, i++, "pm2.5", _acc[1], "µg/m3");
  setReading(doc, i++, "pm10", _acc[2], "µg/m3");
  setReading(doc, i++, "pm1.0", _acc[0], "µg/m3");

  #ifdef USE_TVOC
  if (_acc[3].count() > 0) {
    setReading(doc, i++, "tvoc", _acc[3], "ppb");
  }
  #endif

  doc["token"] = _token;

  static char body[1024 * 2];
  serializeJson(doc, body);
  
  bool success = false;
  if (https.begin(client, "https://aqicn.org/sensor/upload/")) {
    int status = https.POST(body);
    if (status == HTTP_CODE_OK) {
      Serial.println(https.getString());
      success = true;
    } else {
      Serial.println(https.errorToString(status).c_str());
    }
  }
  https.end();
  return success;
}
