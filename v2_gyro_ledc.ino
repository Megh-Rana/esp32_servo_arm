// Version 2 - Gyroscope Control | LEDC (no library)
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
#define US_MIN 3277
#define US_MAX 6554
#define US_MID 4915

WebServer server(80);
int pos[4] = {US_MID, US_MID, US_MID, US_MID};
unsigned long lastCmd = 0;
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

int angleToDuty(float val, float inMin, float inMax) {
  float t = constrain((val - inMin) / (inMax - inMin), 0.0f, 1.0f);
  return (int)(US_MIN + t * (US_MAX - US_MIN));
}

const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<style>
  body{font-family:sans-serif;text-align:center;background:#111;color:#eee;padding:20px;user-select:none}
  .grip{display:flex;gap:20px;justify-content:center;margin-top:30px}
  button{width:140px;height:100px;font-size:22px;border-radius:16px;border:none;color:#fff;cursor:pointer;touch-action:manipulation}
  #btnOpen{background:#27a}
  #btnClose{background:#a33}
  #status{margin-top:16px;font-size:13px;color:#888}
  #permBtn{margin-top:20px;padding:12px 24px;font-size:16px;background:#555;color:#fff;border:none;border-radius:8px;cursor:pointer}
</style></head><body>
<h2>Robotic Arm</h2>
<p id="status">Tap button to enable sensor</p>
<button id="permBtn" onclick="requestPerm()">Enable Motion Sensor</button>
<div class="grip">
  <button id="btnOpen"
    ontouchstart="grip(1)"  ontouchend="grip(0)"
    onmousedown="grip(1)"   onmouseup="grip(0)">Open</button>
  <button id="btnClose"
    ontouchstart="grip(-1)" ontouchend="grip(0)"
    onmousedown="grip(-1)"  onmouseup="grip(0)">Close</button>
</div>
<script>
let sending=false, gripState=0;
let alpha=0, beta=0, gamma=0;

function grip(v){ gripState=v; }

function requestPerm(){
  if(typeof DeviceOrientationEvent!=='undefined' &&
     typeof DeviceOrientationEvent.requestPermission==='function'){
    DeviceOrientationEvent.requestPermission().then(s=>{
      if(s==='granted') startSensor();
    });
  } else { startSensor(); }
  document.getElementById('permBtn').style.display='none';
}

function startSensor(){
  document.getElementById('status').textContent='Sensor active — tilt to control';
  window.addEventListener('deviceorientation',e=>{
    alpha=e.alpha||0; beta=e.beta||0; gamma=e.gamma||0;
  });
  setInterval(sendData,100);
}

function sendData(){
  if(sending) return;
  sending=true;
  fetch(`/orient?a=${alpha.toFixed(1)}&b=${beta.toFixed(1)}&g=${gamma.toFixed(1)}&gr=${gripState}`)
    .finally(()=>{ sending=false; });
}
</script></body></html>
)rawliteral";

void handleRoot() { server.send_P(200, "text/html", PAGE); }

void handleOrient() {
  if (!server.hasArg("a")) { server.send(400); return; }

  float a  = server.arg("a").toFloat();  // alpha  0–360  → UD
  float b  = server.arg("b").toFloat();  // beta  -90–90  → FB
  float g  = server.arg("g").toFloat();  // gamma -90–90  → LR
  int   gr = server.arg("gr").toInt();

  reattachAll();
  servosActive = true;
  lastCmd = millis();

  pos[CH_LR] = angleToDuty(g,  -90.0f,  90.0f);
  pos[CH_UD] = angleToDuty(a,    0.0f, 360.0f);
  pos[CH_FB] = angleToDuty(b,  -90.0f,  90.0f);
  if (gr ==  1) pos[CH_GRIP] = constrain(pos[CH_GRIP] + 80, US_MIN, US_MAX);
  if (gr == -1) pos[CH_GRIP] = constrain(pos[CH_GRIP] - 80, US_MIN, US_MAX);

  for (int i = 0; i < 4; i++) ledcWrite(i, pos[i]);
  server.send(200, "text/plain", "ok");
}

void setup() {
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  WiFi.softAP(ssid, password);

  reattachAll();
  delay(600);
  detachAll();

  server.on("/", handleRoot);
  server.on("/orient", handleOrient);
  server.begin();
}

void loop() {
  server.handleClient();
  if (servosActive && millis() - lastCmd > 800) {
    detachAll();
    servosActive = false;
  }
}
