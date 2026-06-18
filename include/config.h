#ifndef CONFIG_H
#define CONFIG_H

// #define USE_AQICN
// #define USE_TVOC

#define LAT 0
#define LNG 0

#define STASSID ""
#define STAPSK ""

#define MQTT_ADDR ""
#define MQTT_PORT 1883
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""

#ifdef USE_AQICN
#define AQICN_ID ""
#define AQICN_NAME ""
#define AQICN_TOKEN ""
#define AQICN_INTERVAL 10
#endif

#endif