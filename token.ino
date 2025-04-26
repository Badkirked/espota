#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "TelegramUI.h"

const char* ssid     = "The LAN before time";
const char* password = "d43f8d6354";

ESP8266WebServer server(80);
TelegramUI telegramUI(server);

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected! IP: " + WiFi.localIP().toString());

  telegramUI.begin();
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  server.handleClient();
}
