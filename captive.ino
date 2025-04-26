#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecure.h>
#include <TM1637Display.h>

constexpr uint8_t CLK_PIN = D4;
constexpr uint8_t DIO_PIN = D5;
TM1637Display display(CLK_PIN, DIO_PIN);
unsigned long lastDisplayUpdate = 0;
const uint16_t displayInterval = 150;

const char* ap_ssid = "ACME-SETUP";
const char* firmware_url = "https://raw.githubusercontent.com/Badkirked/boom/main/firmware.bin";

DNSServer dns;
ESP8266WebServer server(80);
IPAddress apIP(192, 168, 4, 1);
String wifiOptions;
String stationIP;
String updateStatus = "Idle...";

const uint8_t segments[] = {
  0b00000001, // A
  0b00000010, // B
  0b00000100, // C
  0b00001000, // D
  0b00010000, // E
  0b00100000, // F
  0b01000000, // G
  0b10000000  // DP
};

void scanNetworks() {
  int n = WiFi.scanNetworks();
  wifiOptions = "";
  for (int i = 0; i < n; ++i) {
    wifiOptions += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + "</option>";
  }
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'>
<meta name='viewport' content='width=device-width, initial-scale=1.0, user-scalable=no'>
<title>Operator - Link Lost</title>
<style>
body {
  margin: 0; padding: 0;
  background: black;
  color: #00ff00;
  font-family: monospace;
  overflow: hidden;
  height: 100vh;
  display: flex;
  justify-content: center;
  align-items: center;
}
#terminal {
  text-align: left;
  width: 95%;
  max-width: 600px;
  font-size: 1.2em;
  white-space: pre-wrap;
  line-height: 1.6em;
  padding: 1em;
  box-sizing: border-box;
  position: relative;
  background: rgba(0, 0, 0, 0.85);
}
.blink { animation: blink 1s step-start infinite; }
@keyframes blink { 50% { opacity: 0; } }
#hint {
  color: #ff0033;
  font-style: italic;
  display: none;
  margin-top: 1em;
  animation: glitch 0.2s infinite alternate;
}
@keyframes glitch {
  0% { transform: skewX(2deg); }
  100% { transform: skewX(-2deg); }
}
#cta {
  display: block;
  margin-top: 2em;
  color: #0f0;
  text-decoration: none;
  text-align: center;
  font-size: 1em;
}
#tearing {
  position: absolute;
  top: 0; left: 0;
  width: 100%; height: 100%;
  background: repeating-linear-gradient(0deg, rgba(0,255,0,0.1), rgba(0,255,0,0.1) 1px, transparent 1px, transparent 2px);
  opacity: 0;
  pointer-events: none;
  transition: opacity 0.2s;
  z-index: 9999;
}
#secret {
  color: #ff0033;
  display: none;
  margin-top: 2em;
  animation: blink 0.5s step-end infinite;
}
#overlay {
  position: fixed;
  top: 0; left: 0;
  width: 100vw; height: 100vh;
  background: rgba(0, 0, 0, 0.95);
  color: #0f0;
  font-size: 1.5em;
  display: flex;
  align-items: center;
  justify-content: center;
  font-family: monospace;
  z-index: 99999;
}
</style></head><body>
<div id='overlay'>TAP TO BEGIN</div>
<div id='terminal'>
<span id='operator'>Operator<span class='blink'>:</span></span>
<div id='message'></div>
<div id='hint'></div>
<a href='#' id='cta'>[ Proceed ]</a>
<div id='secret'></div>
<div id='tearing'></div>
<audio id='buzz' src='data:audio/mp3;base64,//uQxAAAAAAAAAAAAAAAAAAAAAAAWGluZwAAAA8AAAACAAACcQCA//////////////////////8AAA==' preload='auto'></audio>
<audio id='boom' src='data:audio/mp3;base64,//uQxAAAAAAAAAAAAAAAAAAAAAAAWGluZwAAAA8AAAACAAACcQCA//////////////////////8AAA==' preload='auto'></audio>
</div>
<script>
const pages = [
  { msg: "Initiating anomaly scan...", hint: "Reality = flux. Proceed with caution." },
  { msg: "Memory echoes detected...", hint: "Voice of the past is not yours." },
  { msg: "Time fragmentation expanding", hint: "Do not trust the chronology." },
  { msg: "Observer effect: activated", hint: "You are being watched." },
  { msg: "Corruption of tapestry logic...", hint: "Every pattern hides a lie." },
  { msg: "Entropy level: critical", hint: "Dissolve what you thought was real." },
  { msg: "Phantom rendering visible...", hint: "Computation is not there. It’s you." },
  { msg: "Non-local reflection emerging", hint: "Identity: unstable." },
  { msg: "Detached from realspace", hint: "You’ve come this far... the jump is imminent." },
  { msg: "Cryptic porthole: unlocked", hint: "Connect to Wi-Fi. Jump. The abyss waits." }
];
let index = 0;
let audioPrimed = false;
let secretBuffer = "";
const msgDiv = document.getElementById("message");
const hintDiv = document.getElementById("hint");
const cta = document.getElementById("cta");
const secretDiv = document.getElementById("secret");
const tearing = document.getElementById("tearing");
const buzz = document.getElementById("buzz");
const boom = document.getElementById("boom");
const overlay = document.getElementById("overlay");

function typeLine(text, callback) {
  msgDiv.textContent = "";
  let i = 0;
  function typeChar() {
    if (i < text.length) {
      msgDiv.textContent += text.charAt(i);
      i++;
      setTimeout(typeChar, 50 + Math.random() * 30);
    } else {
      callback();
    }
  }
  typeChar();
}

function showPage(i) {
  msgDiv.textContent = "";
  hintDiv.style.display = "none";
  typeLine(pages[i].msg, () => {
    setTimeout(() => {
      hintDiv.textContent = pages[i].hint;
      hintDiv.style.display = "block";
    }, 800);
  });
}

function initAudio() {
  if (!audioPrimed) {
    buzz.play().catch(() => {});
    audioPrimed = true;
  }
  overlay.style.display = "none";
  showPage(index);
}

cta.onclick = e => {
  e.preventDefault();
  index++;
  if (index < pages.length) {
    showPage(index);
    if (index === pages.length - 1) {
      boom.play();
      if (navigator.vibrate) navigator.vibrate(200);
    }
  } else {
    tearing.style.opacity = 1;
    setTimeout(() => {
      tearing.style.opacity = 0;
      location.href = "/connect";
    }, 1200);
  }
};

document.getElementById("overlay").addEventListener("click", initAudio);
document.getElementById("overlay").addEventListener("touchstart", initAudio);

document.body.addEventListener("keydown", e => {
  if (e.key === "~") secretBuffer = "";
  else if (/^[a-zA-Z0-9]$/.test(e.key)) secretBuffer += e.key;
  if (secretBuffer.toLowerCase() === "unlock") {
    secretDiv.style.display = "block";
    typeSecret("> hidden channel: LISTENING.");
  }
});
function typeSecret(text) {
  let i = 0;
  secretDiv.textContent = "";
  function typeChar() {
    if (i < text.length) {
      secretDiv.textContent += text.charAt(i);
      i++;
      setTimeout(typeChar, 60);
    }
  }
  typeChar();
}
</script></body></html>
)rawliteral";

void handleConnectPage() {
  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<title>ACME Protocol</title></head><body style='font-family:monospace;background:#000;color:#0f0;text-align:center;padding-top:40px'>";
  html += "<h2>Protocol Step V: Network Alignment</h2>";
  html += "<form action='/update'>";
  html += "<label>Network:</label><br><select name='ssid'>" + wifiOptions + "</select><br><br>";
  html += "<label>Password:</label><br><input type='password' name='pass'><br><br>";
  html += "<input type='submit' value='Execute OTA'>";
  html += "</form></body></html>";
  server.send(200, "text/html", html);
}

void handleOTAUpdate() {
  if (!server.hasArg("ssid") || !server.hasArg("pass")) {
    server.send(400, "text/plain", "Missing credentials.");
    return;
  }
  String ssid = server.arg("ssid");
  String pass = server.arg("pass");
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());
  unsigned long tStart = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - tStart < 20000) delay(500);
  if (WiFi.status() == WL_CONNECTED) {
    stationIP = WiFi.localIP().toString();
    WiFiClientSecure client;
    client.setInsecure();
    server.send(200, "text/plain", "Connected. Fetching firmware from GitHub…\nIP: " + stationIP + "\n\nDevice will reboot if successful.");
    delay(2000);
    ESPhttpUpdate.update(client, firmware_url);
  } else {
    server.send(200, "text/plain", "Wi-Fi failed.");
    delay(3000);
    ESP.restart();
  }
}

void setup() {
  Serial.begin(115200);
  display.setBrightness(7);
  display.clear();
  randomSeed(analogRead(A0));

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255,255,255,0));
  WiFi.softAP(ap_ssid);
  delay(500);
  dns.start(53, "*", apIP);
  scanNetworks();
  server.on("/", []() {
    server.send_P(200, "text/html", index_html);
  });
  server.on("/connect", handleConnectPage);
  server.on("/update", handleOTAUpdate);
  server.onNotFound([]() {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });
  server.begin();
  Serial.println("Mystery Portal Live at 192.168.4.1");
}

void loop() {
  dns.processNextRequest();
  server.handleClient();

  if (millis() - lastDisplayUpdate > displayInterval) {
    lastDisplayUpdate = millis();
    uint8_t segs[] = {0, 0, 0, 0};
    segs[random(0, 4)] = segments[random(0, 8)];
    display.setSegments(segs);
  }
}
