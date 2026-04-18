// Version 1 - Button Control | ESP32Servo library
// Requires: ESP32Servo by Kevin Harrington (Library Manager)
#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const char* ssid     = "RoboArm";
const char* password = "12345678";

#define PIN_LR   13
#define PIN_UD   12
#define PIN_FB   14
#define PIN_GRIP 27

#define STEP 2  // degrees per button press

WebServer server(80);
Servo sLR, sUD, sFB, sGrip;
int pos[4] = {90, 90, 90, 90};  // degrees
unsigned long lastMove = 0;
bool servosActive = false;

void attachAll() {
  sLR.attach(PIN_LR,   500, 2400);
  sUD.attach(PIN_UD,   500, 2400);
  sFB.attach(PIN_FB,   500, 2400);
  sGrip.attach(PIN_GRIP, 500, 2400);
  sLR.write(pos[0]);
  sUD.write(pos[1]);
  sFB.write(pos[2]);
  sGrip.write(pos[3]);
}

void detachAll() {
  sLR.detach();
  sUD.detach();
  sFB.detach();
  sGrip.detach();
}

void setPos(int idx, Servo& s, int val) {
  pos[idx] = constrain(val, 0, 180);
  s.write(pos[idx]);
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
  attachAll();
  servosActive = true;
  lastMove = millis();

  if      (a == "lr-") setPos(0, sLR,   pos[0] - STEP);
  else if (a == "lr+") setPos(0, sLR,   pos[0] + STEP);
  else if (a == "ud-") setPos(1, sUD,   pos[1] - STEP);
  else if (a == "ud+") setPos(1, sUD,   pos[1] + STEP);
  else if (a == "fb-") setPos(2, sFB,   pos[2] - STEP);
  else if (a == "fb+") setPos(2, sFB,   pos[2] + STEP);
  else if (a == "gr-") setPos(3, sGrip, pos[3] - STEP);
  else if (a == "gr+") setPos(3, sGrip, pos[3] + STEP);

  server.send(200, "text/plain", "ok");
}

void setup() {
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  WiFi.softAP(ssid, password);

  attachAll();
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
