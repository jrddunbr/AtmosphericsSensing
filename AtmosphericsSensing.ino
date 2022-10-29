#include <Wire.h>
#include <Adafruit_LPS2X.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_SHT4x.h"
#include <WiFi.h>

//#define SERIAL_MONITOR

Adafruit_LPS25 lps;
Adafruit_SHT4x sht4 = Adafruit_SHT4x();

char ssid[] = "JARVARS";
char pass[] = "password";

WiFiServer server(8080);

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  WiFi.reset();
  WiFi.init(AP_STA_MODE);
  if (!lps.begin_I2C()) {
    while (1) { delay(10); }
  }
  if (! sht4.begin()) {
    while (1) delay(10);
  }
  sht4.setPrecision(SHT4X_HIGH_PRECISION);
  sht4.setHeater(SHT4X_NO_HEATER);
  WiFi.begin(ssid, pass);
  server.begin();
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

void loop() {
  sensors_event_t lps25_temp, lps25_pressure, sht40_temp, sht40_rh;
  
  lps.getEvent(&lps25_pressure, &lps25_temp);
  sht4.getEvent(&sht40_rh, &sht40_temp);
  long rssi = WiFi.RSSI();
  
  #ifndef SERIAL_MONITOR
  Serial.print("LPS25 Temperature: ");Serial.print(lps25_temp.temperature);Serial.println(" degrees C");
  Serial.print("SHT40 Temperature: "); Serial.print(sht40_temp.temperature); Serial.println(" degrees C");
  Serial.print("LPS25 Pressure: ");Serial.print(lps25_pressure.pressure);Serial.println(" hPa");
  Serial.print("SHT40 Humidity: "); Serial.print(sht40_rh.relative_humidity); Serial.println("% rH");
  #endif

  #ifdef SERIAL_MONITOR
  Serial.print("SHT40 Temperature:");
  Serial.print(sht40_temp.temperature);
  Serial.print(",");
  Serial.print("SHT40 Humidity:");
  Serial.print(sht40_rh.relative_humidity);
  Serial.print(",");
  Serial.print("LPS25 Pressure:");
  Serial.print(lps25_pressure.pressure);
  Serial.println("");
  #endif

  WiFiClient client = server.available();
  if (client) {

    String http_header = "HTTP/1.1 200 OK\nContent-Type: application/json\nConnection: close\n";
    String json_data = "{";
    json_data += "\"lps25_temp_c\": ";
    json_data += lps25_temp.temperature;
    json_data += ",";
    json_data += "\"lps25_pressure_hPa\": ";
    json_data += lps25_pressure.pressure;
    json_data += ",";
    json_data += "\"sht40_temp_c\": ";
    json_data += sht40_temp.temperature;
    json_data += ",";
    json_data += "\"sht40_humidity_rh\": ";
    json_data += sht40_rh.relative_humidity;
    json_data += ",";
    json_data += "\"wifi_rssi_dBm\": ";
    json_data += rssi;
    json_data += "}\n";

    http_header += "Length: ";
    http_header += json_data.length();
    http_header += "\n";

    client.println(http_header);
    client.print(json_data);
    client.flush();
    client.stop();
  }
  delay(1000);
}
