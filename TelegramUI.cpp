#include "TelegramUI.h"
#include <WiFiClientSecure.h>

TelegramUI::TelegramUI(ESP8266WebServer& server) : _server(server) {}

void TelegramUI::begin() {
    _server.on("/", HTTP_GET, [this]() { handleRoot(); });
    _server.on("/", HTTP_POST, [this]() { handlePost(); });
}

void TelegramUI::handleRoot() {
    renderPage();
}

void TelegramUI::handlePost() {
    if (_server.hasArg("bot_token")) {
        botToken = _server.arg("bot_token");
        WiFiClientSecure client;
        client.setInsecure();

        // === getMe ===
        if (client.connect("api.telegram.org", 443)) {
            String url = "/bot" + botToken + "/getMe";
            client.println("GET " + url + " HTTP/1.1");
            client.println("Host: api.telegram.org");
            client.println("Connection: close");
            client.println();

            delay(1000);
            String resp = client.readString();
            int start = resp.indexOf("\"username\":\"") + 12;
            int end = resp.indexOf("\"", start);
            if (start > 11 && end > start) {
                botUsername = resp.substring(start, end);
                tokenValid = true;
                tokenStatus = "✅ Token is valid! Bot: @" + botUsername;
            } else {
                botUsername = "";
                tokenValid = false;
                tokenStatus = "❌ Invalid token or Telegram API unreachable.";
            }
            client.stop();
        }

        // === getUpdates ===
        if (tokenValid) {
            if (client.connect("api.telegram.org", 443)) {
                String url = "/bot" + botToken + "/getUpdates";
                client.println("GET " + url + " HTTP/1.1");
                client.println("Host: api.telegram.org");
                client.println("Connection: close");
                client.println();

                delay(1000);
                String resp = client.readString();
                int idStart = resp.lastIndexOf("\"chat\":{\"id\":") + 13;
                int idEnd = resp.indexOf(",", idStart);
                if (idStart > 12 && idEnd > idStart) {
                    chatId = resp.substring(idStart, idEnd);
                } else {
                    chatId = "";
                }
                client.stop();
            }
        }
    }
    renderPage();
}

void TelegramUI::renderPage() {
    String page = R"rawliteral(
<!DOCTYPE html><html lang="en"><head>
<meta charset="UTF-8">
<title>Telegram Bot Maker</title>
<style>
  body { font-family: sans-serif; background: #f4f4f4; padding: 2em; }
  .card { max-width: 600px; margin: auto; background: #fff; padding: 2em; border-radius: 10px; box-shadow: 0 4px 10px rgba(0,0,0,0.1); }
  h2 { text-align: center; }
  ol, ul { margin-left: 1.2em; }
  label { font-weight: bold; }
  input, button { width: 100%; margin: 0.5em 0 1em; padding: 0.8em; font-size: 1em; }
  .success { color: green; }
  .error   { color: red; }
  .mono { background: #eee; font-family: monospace; padding: 0.5em; border-radius: 4px; }
  ul { list-style: none; padding-left: 0; }
  ul li { margin-bottom: 0.5em; }
  a { text-decoration: none; color: #007bff; }
</style>
</head><body><div class="card">
<h2>Create Your Telegram Bot</h2>

<p>👋 Welcome! Follow these steps:</p>
<ol>
  <li>Open <a href="https://t.me/BotFather" target="_blank"><strong>BotFather</strong></a> in Telegram</li>
  <li>Send <span class="mono">/start</span> and then <span class="mono">/newbot</span></li>
  <li>Give your bot a name (e.g., My Weather Bot)</li>
  <li>Give it a username (must end in <code>bot</code>)</li>
  <li>Copy the API token and paste it below 👇</li>
</ol>

<form method='POST' action='/'>
  <label for='bot_token'>Bot Token:</label>
  <input type='text' id='bot_token' name='bot_token' placeholder='123456:ABC-DEF...' required>
  <button type='submit'>Validate Token & Get Chat ID</button>
</form>

<hr>

<h3>🧠 New to Telegram? Follow these steps:</h3>
<ul>
  <li>📱 Install Telegram:
    <ul>
      <li><a href="https://apps.apple.com/app/telegram-messenger/id686449807" target="_blank">Telegram for iOS</a> / 
          <a href="https://play.google.com/store/apps/details?id=org.telegram.messenger" target="_blank">Android</a> / 
          <a href="https://desktop.telegram.org/" target="_blank">Windows / macOS</a></li>
      <li><a href="https://web.telegram.org/" target="_blank">Telegram Web (browser version)</a></li>
    </ul>
  </li>
  <li>👤 Create an account using your phone number</li>
</ul>
)rawliteral";

    if (tokenStatus.length()) {
        page += "<div class='" + String(tokenValid ? "success" : "error") + "'>" + tokenStatus + "</div>";
    }

    if (tokenValid) {
        page += "<p>✅ Your bot is live! Try sending it a message now.</p>";
        page += "<p><a href='https://t.me/" + botUsername + "' target='_blank'>🔗 Open Your Bot: @" + botUsername + "</a></p>";

        if (chatId.length()) {
            page += "<p><strong>🆔 Your Chat ID:</strong> <span class='mono'>" + chatId + "</span></p>";
        } else {
            page += "<p class='error'>⚠️ No recent messages found. Message your bot first, then click the button again.</p>";
        }

        page += "<h3>🔧 Python code to send messages:</h3><pre class='mono'>";
        page += "import requests\n\n";
        page += "TOKEN = \"" + botToken + "\"\n";
        page += "CHAT_ID = \"" + (chatId.length() ? chatId : "your-chat-id") + "\"\n";
        page += "MESSAGE = \"Hello from my bot!\"\n\n";
        page += "url = f\"https://api.telegram.org/bot{TOKEN}/sendMessage\"\n";
        page += "payload = {\"chat_id\": CHAT_ID, \"text\": MESSAGE}\n";
        page += "r = requests.post(url, data=payload)\n";
        page += "print(r.text)</pre>";
    }

    page += "</div></body></html>";
    _server.send(200, "text/html", page);
}
