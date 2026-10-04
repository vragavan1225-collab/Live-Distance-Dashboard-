#include <WiFi.h>
#include <WebServer.h>

// Pin Definitions for HC-SR04
const int TRIG_PIN = 5;
const int ECHO_PIN = 18;

// Wokwi Virtual WiFi credentials
const char* ssid = "Wokwi-GUEST";
const char* password = "";

WebServer server(80);

// Function to measure distance in cm
float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout
  if (duration == 0) return -1.0; // Return -1 if out of range/timeout

  float distanceCm = duration * 0.0343 / 2.0;
  return distanceCm;
}

// Serve main HTML page with AJAX auto-refresh
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  html += "<title>ESP32 Distance Dashboard</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; background-color: #f4f4f9; }";
  html += ".card { background: white; padding: 30px; border-radius: 10px; display: inline-block; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }";
  html += ".value { font-size: 48px; color: #007bff; margin: 20px 0; font-weight: bold; }";
  html += "</style>";
  html += "<script>";
  html += "setInterval(function() {";
  html += "  fetch('/distance').then(response => response.text()).then(data => {";
  html += "    document.getElementById('distValue').innerText = data;";
  html += "  });";
  html += "}, 1000);"; // Fetch every 1000ms
  html += "</script></head><body>";
  html += "<div class=\"card\">";
  html += "<h2>Live Distance Sensor</h2>";
  html += "<div class=\"value\"><span id=\"distValue\">--</span> cm</div>";
  html += "<p>Updating live via AJAX...</p>";
  html += "</div></body></html>";

  server.send(200, "text/html", html);
}

// API endpoint returning raw distance value
void handleDistance() {
  float dist = getDistance();
  if (dist < 0) {
    server.send(200, "text/plain", "Out of Range");
  } else {
    server.send(200, "text/plain", String(dist, 1));
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("Access Dashboard at: http://");
  Serial.println(WiFi.localIP());

  // Define HTTP routes
  server.on("/", handleRoot);
  server.on("/distance", handleDistance);

  server.begin();
}

void loop() {
  server.handleClient();
}
