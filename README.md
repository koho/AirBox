# Air Quality Monitor

Air Box is an air quality monitor built on ESP8266. It can be easily integrated
into [Home Assistant](https://www.home-assistant.io/) via MQTT protocol.
If you install it outdoors, you can also share your real-time air quality data on the [AQICN](https://waqi.info/) world
map.

## Sensors

It currently supports PM and TVOC monitoring.

### PMS5003 (Required)

The PMS5003 PM2.5 Air Quality Sensor is a high-performance sensor designed for accurate detection and measurement of
particulate matter (PM) in the air, specifically focusing on PM2.5 particles.

- TX pin on sensor -> D6
- RX pin on sensor -> D7

### GY-SGP30 (Optional)

The SGP30 is a digital multi-pixel VOC sensor.

- SDA pin on sensor -> D1
- SCL pin on sensor -> D2

Enable it by defining `USE_TVOC`.

## Configuration

The config file is located in `include/config.h`. You must check your config file before uploading your program.

| Name           | Type   | Required  | Description                                 |
|----------------|--------|-----------|---------------------------------------------|
| USE_TVOC       |        | No        | Enable the VOC sensor.                      |
| LAT            | float  | Yes       | Latitude.                                   |
| LNG            | float  | Yes       | Longitude.                                  |
| STASSID        | string | Yes       | WiFi SSID.                                  |
| STAPSK         | string | Yes       | WiFi password.                              |
| MQTT_ADDR      | string | Yes       | MQTT server address.                        |
| MQTT_PORT      | int    | Yes       | MQTT server port.                           |
| MQTT_USERNAME  | string | No        | MQTT username.                              |
| MQTT_PASSWORD  | string | No        | MQTT password.                              |
| USE_AQICN      |        | No        | Publish data to AQICN.                      |
| AQICN_ID       | string | USE_AQICN | Station ID.                                 |
| AQICN_NAME     | string | USE_AQICN | Station name.                               |
| AQICN_TOKEN    | string | USE_AQICN | Token from AQICN.                           |
| AQICN_INTERVAL | int    | USE_AQICN | Data push interval (minutes).               |

## Screenshots

![homeassistant](/docs/ha.png)
