// Version 1 - Button Control | LEDC (no library)
#include <WiFi.h>
#include <WebServer.h>

const char* ssid     = "RoboArm";
const char* password = "12345678";

#define PIN_LR   13
#define PIN_UD   12
#define PIN_FB   14
#define PIN_GRIP 27

#define CH_LR   0
#define CH_UD   1
#define CH_FB   2
#define CH_GRIP 3

#define FREQ   50
#define RES    16
#define US_MIN 3277   // ~1ms  @ 16-bit / 50Hz
#define US_MAX 6554   // ~2ms
#define US_MID 4915   // ~1.5ms
#define STEP   100

WebServer server(80);
int pos[4] = {US_MID, US_MID, US_MID, US_MID};
unsigned long lastMove = 0;
bool servosActive = false;

void attachServo(int ch, int pin) {
  ledcSetup(ch, FREQ, RES);
  ledcAttachPin(pin, ch);
}

void reattachAll() {
  attachServo(CH_LR,   PIN_LR);
  attachServo(CH_UD,   PIN_UD);
  attachServo(CH_FB,   PIN_FB);
  attachServo(CH_GRIP, PIN_GRIP);
  for (int i = 0; i < 4; i++) ledcWrite(i, pos[i]);
}

void detachAll() {
  ledcDetachPin(PIN_LR);
  ledcDetachPin(PIN_UD);
  ledcDetachPin(PIN_FB);
  ledcDetachPin(PIN_GRIP);
}

void setPos(int ch, int val) {
  pos[ch] = constrain(val, US_MIN, US_MAX);
  ledcWrite(ch, pos[ch]);
}

const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<style>
  body{font-family:sans-serif;text-align:center;background:#111;color:#eee;padding:20px}
  .row{display:flex;justify-content:center;gap:10px;margin:8px 0}
  button{width:90px;height:60px;font-size:16px;border-radius:10px;border:none;
         background:#2a7;color:#fff;cursor:pointer;touch-action:manipulation}
  button:active{background:#185}
  h3{margin:16px 0 4px}
</style></head><body>
<h2>Robotic Arm</h2>
<h3>Left / Right</h3>
<div class="row">
  <button onclick="cmd('lr-')">◀ Left</button>
  <button onclick="cmd('lr+')">Right ▶</button>
</div>
<h3>Up / Down</h3>
<div class="row">
  <button onclick="cmd('ud+')">▲ Up</button>
  <button onclick="cmd('ud-')">▼ Down</button>
</div>
<h3>Forward / Back</h3>
<div class="row">
  <button onclick="cmd('fb+')">Fwd ▶</button>
  <button onclick="cmd('fb-')">◀ Back</button>
</div>
<h3>Grip</h3>
<div class="row">
  <button onclick="cmd('gr+')">Open</button>
  <button onclick="cmd('gr-')">Close</button>
</div>
<script>
function cmd(c){fetch('/cmd?a='+c)}
</script></body></html>
)rawliteral";

void handleRoot() { server.send_P(200, "text/html", PAGE); }

void handleCmd() {
  if (!server.hasArg("a")) { server.send(400); return; }
  String a = server.arg("a");
  reattachAll();
  servosActive = true;
  lastMove = millis();

  if      (a == "lr-") setPos(CH_LR,   pos[0] - STEP);
  else if (a == "lr+") setPos(CH_LR,   pos[0] + STEP);
  else if (a == "ud-") setPos(CH_UD,   pos[1] - STEP);
  else if (a == "ud+") setPos(CH_UD,   pos[1] + STEP);
  else if (a == "fb-") setPos(CH_FB,   pos[2] - STEP);
  else if (a == "fb+") setPos(CH_FB,   pos[2] + STEP);
  else if (a == "gr-") setPos(CH_GRIP, pos[3] - STEP);
  else if (a == "gr+") setPos(CH_GRIP, pos[3] + STEP);

  server.send(200, "text/plain", "ok");
}

void setup() {
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  WiFi.softAP(ssid, password);

  reattachAll();
  delay(600);
  detachAll();

  server.on("/", handleRoot);
  server.on("/cmd", handleCmd);
  server.begin();
}

void loop() {
  server.handleClient();
  if (servosActive && millis() - lastMove > 800) {
    detachAll();
    servosActive = false;
  }
}
