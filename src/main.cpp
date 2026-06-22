#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "mqtt.h"
#include "PMS.h"
#include "accumulator.h"

MQTT mqtt(MQTT_ADDR, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);

#ifdef USE_TVOC
#include "SGP30.h"
SGP30 sgp;
Trend tvocTrend;
#endif

#ifdef USE_AQICN
#include "aqicn.h"
AQICN aqicn(AQICN_ID, AQICN_NAME, LAT, LNG, AQICN_TOKEN, AQICN_INTERVAL);
AQICN::DATA aqi;
#endif

// PM Sensor
PMS::DATA pm;
SoftwareSerial pmSerial(D6, D7);
PMS pms(pmSerial);
Trend pm25Trend;

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(STASSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  mqtt.connect();

  // PMS
  pmSerial.begin(9600);
  pms.passiveMode();

  // TVOC
  #ifdef USE_TVOC
  Wire.begin();
  if (!sgp.begin()){
    Serial.println("TVOC Sensor not found");
  }
  #endif
}

void loop() {
  JsonDocument doc;

  while (pmSerial.available()) { pmSerial.read(); }
  pms.requestRead();
  bool ok = pms.readUntil(&pm);
  float trend = pm25Trend.calc(ok ? pm.PM_AE_UG_2_5 : NAN);
  doc["pm_1_0"] = ok ? pm.PM_AE_UG_1_0 : NAN;
  doc["pm_2_5"] = ok ? pm.PM_AE_UG_2_5 : NAN;
  doc["pm_10_0"] = ok ? pm.PM_AE_UG_10_0 : NAN;
  doc["pm_trend"] = trend;
  #ifdef USE_AQICN
  aqi.pm_1_0 = ok ? pm.PM_AE_UG_1_0 : NAN;
  aqi.pm_2_5 = ok ? pm.PM_AE_UG_2_5 : NAN;
  aqi.pm_10_0 = ok ? pm.PM_AE_UG_10_0 : NAN;
  #endif

  #ifdef USE_TVOC
  SGP30::DATA gas;
  ok = sgp.read(&gas);
  doc["tvoc"] = ok ? gas.TVOC_PPB : NAN;
  doc["tvoc_trend"] = tvocTrend.calc(ok ? gas.TVOC_PPB : NAN);
  #ifdef USE_AQICN
  aqi.tvoc = ok ? gas.TVOC_PPB : NAN;
  #endif
  #endif

  static char body[500];
  serializeJson(doc, body);
  mqtt.publish("data", body);

  #ifdef USE_AQICN
  aqicn.feed(aqi, trend);
  #endif
  
  delay(1000);
}
