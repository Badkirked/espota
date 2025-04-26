#ifndef TELEGRAM_UI_H
#define TELEGRAM_UI_H

#include <ESP8266WebServer.h>

class TelegramUI {
public:
    TelegramUI(ESP8266WebServer& server);
    void begin();
    void handleRoot();
    void handlePost();

private:
    ESP8266WebServer& _server;
    String botToken;
    String botUsername;
    String chatId;
    String tokenStatus;
    bool tokenValid;

    void renderPage();
};

#endif
