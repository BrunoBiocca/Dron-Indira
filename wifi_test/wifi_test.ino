#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

const char* SSID = "ESP32-DRON";
const char* PASS = "test_bruno";

WebServer server(80); //80 is the port

void paginaInicio() {
  //200 it the code from server stating that everything is fine to your browser
  server.send(200, "text/html", "<h1>Hola desde el ESP32</h1>");
}

void setup() {
  Serial.begin(115200);

  // WIfi config
  WiFi.mode(WIFI_AP);
  WiFi.softAP(SSID, PASS);

  Serial.print("IP del ESP32: ");
  Serial.println(WiFi.softAPIP());

  //connects the route with the function, "/" indicates that it's the main page
  server.on("/", paginaInicio); 
  server.begin(); // begins server
}

void loop() {
  server.handleClient(); //checks for petitions recerived to the server

}
