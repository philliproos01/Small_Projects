#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "ESP8266-Access-Point";
const char* password = "123456789";

ESP8266WebServer server(80);
IPAddress local_ip(192,168,4,1);
IPAddress gateway(192,168,4,1);
IPAddress subnet(255,255,255,0);

bool ledState = false;
bool inputStates[14] = {false};

String getHTML() {
  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<script>function updateStatus() {fetch('/status').then(response => response.json()).then(data => {";
  html += "document.getElementById('led').innerHTML = data.led ? 'ON' : 'OFF';";
  for (int i = 0; i < 14; i++) {
    html += "document.getElementById('input" + String(i) + "').innerHTML = data.inputs[" + String(i) + "] ? 'HIGH' : 'LOW';";
  }
  html += "});} setInterval(updateStatus, 1000);</script></head><body>";
  html += "<h1>ESP8266 Web Server</h1>";
  
  for (int i = 0; i < 15; i++) {
    if (i == 0) {
      html += "<button onclick='fetch(\"/toggle\")'>Toggle LED</button> LED Status: <span id='led'></span><br>";
    } else {
      html += "<button>Button " + String(i) + "</button> Input " + String(i) + " Status: <span id='input" + String(i-1) + "'></span><br>";
    }
  }
  
  html += "</body></html>";
  return html;
}

void handleRoot() {
  server.send(200, "text/html", getHTML());
}

void handleToggle() {
  ledState = !ledState;
  Serial.println(ledState ? "LED ON" : "LED OFF");
  server.send(200, "text/plain", "LED toggled");
}

void handleStatus() {
  String json = "{\"led\":" + String(ledState);
  json += ",\"inputs\":[";
  for (int i = 0; i < 14; i++) {
    json += String(inputStates[i]);
    if (i < 13) json += ",";
  }
  json += "]}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESP8266 Web Server Simulation");
  
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid, password);
  
  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);
  server.on("/status", handleStatus);
  
  server.begin();
  Serial.println("HTTP server started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  server.handleClient();
  
  // Simulate random changes in input states
  if (random(100) < 5) {  // 5% chance of change for any input
    int inputToChange = random(14);
    inputStates[inputToChange] = !inputStates[inputToChange];
    Serial.print("Input ");
    Serial.print(inputToChange);
    Serial.println(inputStates[inputToChange] ? " HIGH" : " LOW");
  }
  
  delay(100);  // Small delay to prevent flooding the serial monitor
}
