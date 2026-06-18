#include "mqtt.h"

MQTT::MQTT(const char *domain, uint16_t port, const char *user, const char *pass) {
  _client.setClient(_wifiClient);
  _client.setServer(domain, port);
  _client.setBufferSize(500);
  _prefix = WiFi.macAddress();
  _user = user;
  _pass = pass;
}

bool MQTT::connect() {
  static char clientId[20] = { 0 };
  sprintf(clientId, "ESP-%X", ESP.getChipId());
  String willTopic = _prefix + "/status";
  if (_client.connect(clientId, _user, _pass, willTopic.c_str(), 0, true, "offline", true)) { 
    Serial.println("MQTT Server Connected.");
    Serial.print("ClientId: ");
    Serial.println(clientId);
    _client.publish(willTopic.c_str(), "online", true);
    sendHADiscovery();
  }
  return _client.connected();
}

bool MQTT::publish(const char* topic, const char* payload) {
  if (!_client.connected()) {
    if (millis() - _lastReconnectAttempt > 5000) {
      bool success = connect();
      _lastReconnectAttempt = millis();
      if (!success) return false;
    } else {
      return false;
    }
  }
  return _client.publish((_prefix + "/" + topic).c_str(), payload);
}

void MQTT::sendHADiscovery() {
  static char devId[20] = { 0 };
  sprintf(devId, "AIRBOX_%X", ESP.getChipId());
  static char devName[20] = { 0 };
  sprintf(devName, "Air Box %X", (uint16_t)ESP.getChipId());
  uint8_t mac[6];
  char macStr[18] = { 0 };
  WiFi.macAddress(mac);
  sprintf(macStr, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  String mdl = "PM";
  #ifdef USE_TVOC
  mdl += "VOC";
  #endif
  JsonDocument doc;
  doc["dev"]["ids"] = devId;
  doc["dev"]["name"] = devName;
  doc["dev"]["mf"] = "OEM";
  doc["dev"]["mdl"] = mdl;
  doc["dev"]["sw"] = "1.0";
  doc["dev"]["sn"] = macStr;
  doc["dev"]["hw"] = "1.0rev1";
  doc["o"]["name"] = "built-in";
  doc["stat_t"] = _prefix + "/data";
  doc["avty_t"] = _prefix + "/status";
  doc["qos"] = 0;

  String status = String(devId) + "_STATUS";
  doc["cmps"][status]["p"] = "binary_sensor";
  doc["cmps"][status]["dev_cla"] = "connectivity";
  doc["cmps"][status]["payload_on"] = "online";
  doc["cmps"][status]["payload_off"] = "offline";
  doc["cmps"][status]["stat_t"] = _prefix + "/status";
  doc["cmps"][status]["avty_tpl"] = "{{ 'online' if value in ['online', 'offline'] else 'offline' }}";
  doc["cmps"][status]["uniq_id"] = status;
  String pm1 = String(devId) + "_PM1";
  doc["cmps"][pm1]["p"] = "sensor";
  doc["cmps"][pm1]["dev_cla"] = "pm1";
  doc["cmps"][pm1]["unit_of_meas"] = "µg/m³";
  doc["cmps"][pm1]["val_tpl"] = "{{ value_json.pm_1_0 }}";
  doc["cmps"][pm1]["uniq_id"] = pm1;
  String pm25 = String(devId) + "_PM25";
  doc["cmps"][pm25]["p"] = "sensor";
  doc["cmps"][pm25]["dev_cla"] = "pm25";
  doc["cmps"][pm25]["unit_of_meas"] = "µg/m³";
  doc["cmps"][pm25]["val_tpl"] = "{{ value_json.pm_2_5 }}";
  doc["cmps"][pm25]["sug_dsp_prc"] = 0;
  doc["cmps"][pm25]["uniq_id"] = pm25;
  String pm10 = String(devId) + "_PM10";
  doc["cmps"][pm10]["p"] = "sensor";
  doc["cmps"][pm10]["dev_cla"] = "pm10";
  doc["cmps"][pm10]["unit_of_meas"] = "µg/m³";
  doc["cmps"][pm10]["val_tpl"] = "{{ value_json.pm_10_0 }}";
  doc["cmps"][pm10]["uniq_id"] = pm10;
  String pmTrend = String(devId) + "_PM_TREND";
  doc["cmps"][pmTrend]["p"] = "sensor";
  doc["cmps"][pmTrend]["dev_cla"] = "pm25";
  doc["cmps"][pmTrend]["unit_of_meas"] = "µg/m³";
  doc["cmps"][pmTrend]["val_tpl"] = "{{ value_json.pm_trend }}";
  doc["cmps"][pmTrend]["ic"] = "mdi:trending-up";
  doc["cmps"][pmTrend]["sug_dsp_prc"] = 2;
  doc["cmps"][pmTrend]["name"] = "PM 趋势";
  doc["cmps"][pmTrend]["uniq_id"] = pmTrend;
  #ifdef USE_TVOC
  String tvoc = String(devId) + "_TVOC";
  doc["cmps"][tvoc]["p"] = "sensor";
  doc["cmps"][tvoc]["dev_cla"] = "volatile_organic_compounds_parts";
  doc["cmps"][tvoc]["unit_of_meas"] = "ppb";
  doc["cmps"][tvoc]["val_tpl"] = "{{ value_json.tvoc }}";
  doc["cmps"][tvoc]["uniq_id"] = tvoc;
  String tvocTrend = String(devId) + "_TVOC_TREND";
  doc["cmps"][tvocTrend]["p"] = "sensor";
  doc["cmps"][tvocTrend]["dev_cla"] = "volatile_organic_compounds_parts";
  doc["cmps"][tvocTrend]["unit_of_meas"] = "ppb";
  doc["cmps"][tvocTrend]["val_tpl"] = "{{ value_json.tvoc_trend }}";
  doc["cmps"][tvocTrend]["ic"] = "mdi:trending-up";
  doc["cmps"][tvocTrend]["sug_dsp_prc"] = 2;
  doc["cmps"][tvocTrend]["name"] = "TVOC 趋势";
  doc["cmps"][tvocTrend]["uniq_id"] = tvocTrend;
  #endif
  static char body[2000];
  serializeJson(doc, body);
  _client.setBufferSize(2000);
  _client.publish((String("homeassistant/device/") + devId + "/config").c_str(), body, true);
  _client.setBufferSize(500);
}