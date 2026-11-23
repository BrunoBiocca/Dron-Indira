#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "webpage.h"

const char* SSID = "ESP32-DRON";
const char* PASS = "test_bruno";

int vertical = 0; // movement variables
int joyX = 0;
int joyY = 0;

WebServer server(80); //80 is the port
WebSocketsServer ws(81); // creates the WBS in port 81, need a separated port

void startpage() {
  //200 it the code from server stating that everything is fine to your browser
  server.send(200, "text/html", PAGE);
}


//handles the commands received in the socket
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  if (type == WStype_CONNECTED) {
    Serial.println("Client connected");
  }

  if (type == WStype_DISCONNECTED) {
    vertical = 0;
    Serial.println("Client disconnected, vertical = 0");
  }

  if (type == WStype_TEXT) {
    String cmd = String((char*)payload);

    if (cmd == "UP") {
      vertical = 1;
    } else if (cmd == "DOWN") {
      vertical = -1;
    } else if (cmd == "STOP") {
      vertical = 0;
    } else if (cmd.startsWith("J,")) {
      int firstComma = cmd.indexOf(',');
      int secondComma = cmd.indexOf(',', firstComma + 1);
      joyX = cmd.substring(firstComma + 1, secondComma).toInt();
      joyY = cmd.substring(secondComma + 1).toInt();
      Serial.print("Joystick X: ");
      Serial.print(joyX);
      Serial.print("  Y: ");
      Serial.println(joyY);
      return;
    }

    Serial.print("Vertical: ");
    Serial.println(vertical);
  }
}


void setup() {
  Serial.begin(115200);

  // WIfi config
  WiFi.mode(WIFI_AP);
  WiFi.softAP(SSID, PASS);

  Serial.print("IP from ESP32: ");
  Serial.println(WiFi.softAPIP());

  //connects the route with the function, "/" indicates that it's the main page
  server.on("/", startpage); 
  server.begin(); // begins server

  //whenever smth happens on the WebSocket, run eventWS function
  ws.begin();
  ws.onEvent(onWebSocketEvent);
}

void loop() {
  server.handleClient(); //checks for petitions recerived to the server
  ws.loop(); //same but for WSS
}
