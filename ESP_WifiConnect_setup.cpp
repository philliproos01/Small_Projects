



#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";

WebServer server(80);
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

/* SSID for local connection*/
char* ssid_router; //= "Point Lighting Guest";  // Enter SSID here
char* password_router;//= "PLguestPL";  //Enter Password here
String header;
bool local_connection = 0;

WiFiServer server_connect(80);

// Auxiliar variables to store the current output state
String output26State = "off";
String output27State = "off";
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;


// Assign output variables to GPIO pins
const int output26 = 26;
const int output27 = 27;


void setup_local_connection(String user, String pass) {
  Serial.println("Disabling AP... connecting to router");
  server.close();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.print("Connecting to ");
  Serial.println(ssid_router);

  delay(500);

  local_connection = 1;
  if (pass != "") {
    WiFi.begin(ssid_router, password_router);
  } else {
    WiFi.begin(ssid_router);
  }
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

}

void handleRoot() {
  String html = "<html><body>";
  html += "<h1>ESP32 Web Server</h1>";
  html += "<form action='/submit' method='POST'>";
  html += "Enter text 1: <input type='text' name='input_text1'><br>";
  html += "Enter text 2: <input type='text' name='input_text2'><br>";
  html += "<input type='submit' value='Submit'>";
  html += "</form></body></html>";
  server.send(200, "text/html", html);
}

void handleSubmit() {
  String inputText1 = server.arg("input_text1");
  String inputText2 = server.arg("input_text2");
  
  
  Serial.println("Text 1: " + inputText1);
  Serial.println("Text 2: " + inputText2);
  
  server.send(200, "text/plain", "Texts received: " + inputText1 + ", " + inputText2);
  setup_local_connection(inputText1, inputText2);
}

void setup() {
  Serial.begin(115200);
  
  
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  
  

  server.on("/", handleRoot);
  server.on("/submit", HTTP_POST, handleSubmit);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  if (!local_connection) {
    server.handleClient();
  }

  if (local_connection) {
    WiFiClient client = server_connect.available();   // Listen for incoming clients
    if (client) {                             // If a new client connects,
      currentTime = millis();
      previousTime = currentTime;
      Serial.println("New Client.");          // print a message out in the serial port
      String currentLine = "";                // make a String to hold incoming data from the client
      while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
        currentTime = millis();
        if (client.available()) {             // if there's bytes to read from the client,
          char c = client.read();             // read a byte, then
          Serial.write(c);                    // print it out the serial monitor
          header += c;
          if (c == '\n') {                    // if the byte is a newline character
            // if the current line is blank, you got two newline characters in a row.
            // that's the end of the client HTTP request, so send a response:
            if (currentLine.length() == 0) {
              // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
              // and a content-type so the client knows what's coming, then a blank line:
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();
              
              // turns the GPIOs on and off
              if (header.indexOf("GET /26/on") >= 0) {
                Serial.println("GPIO 26 on");
                output26State = "on";
                digitalWrite(output26, HIGH);
              } else if (header.indexOf("GET /26/off") >= 0) {
                Serial.println("GPIO 26 off");
                output26State = "off";
                digitalWrite(output26, LOW);
              } else if (header.indexOf("GET /27/on") >= 0) {
                Serial.println("GPIO 27 on");
                output27State = "on";
                digitalWrite(output27, HIGH);
              } else if (header.indexOf("GET /27/off") >= 0) {
                Serial.println("GPIO 27 off");
                output27State = "off";
                digitalWrite(output27, LOW);
              }
              
              // Display the HTML web page
              client.println("<!DOCTYPE html><html>");
              client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
              client.println("<link rel=\"icon\" href=\"data:,\">");
              // CSS to style the on/off buttons 
              // Feel free to change the background-color and font-size attributes to fit your preferences
              client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
              client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
              client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
              client.println(".button2 {background-color: #555555;}</style></head>");
              
              // Web Page Heading
              client.println("<body><h1>ESP32 Web Server</h1>");
              
              // Display current state, and ON/OFF buttons for GPIO 26  
              client.println("<p>GPIO 26 - State " + output26State + "</p>");
              // If the output26State is off, it displays the ON button       
              if (output26State=="off") {
                client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
              } else {
                client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
              } 
                
              // Display current state, and ON/OFF buttons for GPIO 27  
              client.println("<p>GPIO 27 - State " + output27State + "</p>");
              // If the output27State is off, it displays the ON button       
              if (output27State=="off") {
                client.println("<p><a href=\"/27/on\"><button class=\"button\">ON</button></a></p>");
              } else {
                client.println("<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a></p>");
              }
              client.println("</body></html>");
              
              // The HTTP response ends with another blank line
              client.println();
              // Break out of the while loop
              break;
            } else { // if you got a newline, then clear currentLine
              currentLine = "";
            }
          } else if (c != '\r') {  // if you got anything else but a carriage return character,
            currentLine += c;      // add it to the end of the currentLine
          }
        }
      }
      // Clear the header variable
      header = "";
      // Close the connection
      client.stop();
      Serial.println("Client disconnected.");
      Serial.println("");
    }
  
  }
}
