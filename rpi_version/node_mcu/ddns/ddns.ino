#include <EasyDDNS.h>
#include <ESP8266WiFi.h>

const char* kWiFiSSID = "VPS";
const char* kWiFiPassword = "milorad__senic";
const int   kUdpServerPort = 12345;
IPAddress ip(192, 168, 2, 200);
IPAddress gateway(192, 168, 2, 1); 
IPAddress subnet(255, 255, 255, 0); 

// TODO: Check if this is really needed. It was in an example.
WiFiServer server(80);

void setup() {
  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet); 
  WiFi.begin(kWiFiSSID, kWiFiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  // TODO: Check if this is really needed. It was in an example.
  server.begin();
  EasyDDNS.service("noip");
  EasyDDNS.client("vs-bgnet.ddns.net","viktorslavkovic","qSKI1uZLFP");
}

void loop() {
  EasyDDNS.update(10000); // Check for New Ip Every 10 Seconds.
}

