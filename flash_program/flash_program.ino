#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "webpage.h"
#include "imu.h"
#include "mixer.h"
#include "control.h"

const char* SSID = "ESP32-DRON";
const char* PASS = "test_bruno";
const unsigned long FAILSAFE_MS = 500;

WebServer server(80);
WebSocketsServer ws(81);

int joyX = 0;
int joyY = 0;
bool upHeld = false;
bool downHeld = false;

unsigned long lastMsgMs = 0;
unsigned long lastControlUs = 0;
unsigned long lastStatusMs = 0;
unsigned long lastPrint = 0;
bool calibrateRequested = false;

MotorOutputs motors = {0, 0, 0, 0};

ControlInput currentInput() {
  ControlInput in = {joyX, joyY, upHeld, downHeld};
  return in;
}

void handleRoot() {
  server.send_P(200, "text/html", PAGE);
}

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  if (type == WStype_CONNECTED) {
    Serial.println("Client connected");
    lastMsgMs = millis();
  }

  if (type == WStype_DISCONNECTED) {
    upHeld = false;
    downHeld = false;
    joyX = 0;
    joyY = 0;
    controlStartLanding("client disconnected");
  }

  if (type == WStype_TEXT) {
    lastMsgMs = millis();
    String cmd = String((char*)payload);

    if (cmd == "H") {
      return;
    } else if (cmd == "ARM") {
      const char* err = controlTryArm(imuGet(), currentInput());
      if (err) {
        Serial.print("Arm refused: ");
        Serial.println(err);
      }
    } else if (cmd == "DISARM") {
      controlDisarm("user request");
    } else if (cmd == "CALIB") {
      if (controlIsArmed()) {
        Serial.println("Calibration refused: disarm first");
      } else {
        calibrateRequested = true;
      }
    } else if (cmd == "UP_ON") {
      upHeld = true;
    } else if (cmd == "UP_OFF") {
      upHeld = false;
    } else if (cmd == "DOWN_ON") {
      downHeld = true;
    } else if (cmd == "DOWN_OFF") {
      downHeld = false;
    } else if (cmd.startsWith("J,")) {
      int firstComma = cmd.indexOf(',');
      int secondComma = cmd.indexOf(',', firstComma + 1);
      joyX = cmd.substring(firstComma + 1, secondComma).toInt();
      joyY = cmd.substring(secondComma + 1).toInt();
    }
  }
}

void setup() {
  Serial.begin(115200);
  imuBegin();

  WiFi.mode(WIFI_AP);
  WiFi.softAP(SSID, PASS);
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.begin();

  ws.begin();
  ws.onEvent(onWebSocketEvent);

  lastControlUs = micros();
}

void loop() {
  server.handleClient();
  ws.loop();

  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'c') calibrateRequested = true;
  }

  if (calibrateRequested) {
    calibrateRequested = false;
    imuCalibrateLevel();
    lastControlUs = micros();
    controlResetPid();
  }

  if (controlIsArmed() && !controlIsLanding() && millis() - lastMsgMs > FAILSAFE_MS) {
    controlStartLanding("link lost (failsafe)");
  }

  if (imuUpdate()) {
    ImuData d = imuGet();

    unsigned long nowUs = micros();
    float dt = (nowUs - lastControlUs) / 1e6f;
    lastControlUs = nowUs;

    motors = controlUpdate(d, currentInput(), dt);

    if (millis() - lastPrint >= 200) {
      lastPrint = millis();
      Serial.printf("State %d  thrust %.0f\n", controlState(), controlGetThrust());
      Serial.printf("FL %3.0f   FR %3.0f\n", motors.fl, motors.fr);
      Serial.printf("RL %3.0f   RR %3.0f\n\n", motors.rl, motors.rr);
    }
  }

  if (millis() - lastStatusMs >= 100) {
    lastStatusMs = millis();

    if (ws.connectedClients() > 0) {
      ImuData d = imuGet();
      char buf[48];
      snprintf(buf, sizeof(buf), "S,%d,%.0f,%.1f,%.1f",
               controlState(), controlGetThrust(), d.roll, d.pitch);
      ws.broadcastTXT(buf);
    }
  }
}