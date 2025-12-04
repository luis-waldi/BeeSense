#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);

// WLAN-Zugangsdaten anpassen
const char* ssid     = "DEIN_WLAN_NAME";
const char* password = "DEIN_WLAN_PASSWORT";

// Beispielwerte – später durch echte Messwerte ersetzen
int   flights_today = 23;
float temperature   = 21.8;
float humidity      = 56.2;

// Einfache HTML-Seite fürs Dashboard
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="UTF-8" />
<meta name="viewport" content="width=device-width, initial-scale=1.0" />
<title>Hummelstock Dashboard</title>
<style>
    body { font-family: Arial, sans-serif; background: #f5f7fa; padding: 20px; }
    .card {
        background: white; padding: 20px; margin-bottom: 15px;
        border-radius: 12px; box-shadow: 0 2px 5px rgba(0,0,0,0.1);
    }
    h2 { margin-top: 0; }
    .value { font-size: 2rem; font-weight: bold; color: #0077cc; }
</style>
</head>
<body>
    <h1>Hummelstock Dashboard</h1>

    <div class="card">
        <h2>Flüge heute</h2>
        <div id="flights" class="value">--</div>
    </div>

    <div class="card">
        <h2>Temperatur (°C)</h2>
        <div id="temperature" class="value">--</div>
    </div>

    <div class="card">
        <h2>Luftfeuchtigkeit (%)</h2>
        <div id="humidity" class="value">--</div>
    </div>

<script>
function updateData() {
    fetch("/data")
        .then(res => res.json())
        .then(data => {
            document.getElementById("flights").innerText     = data.flights_today;
            document.getElementById("temperature").innerText = data.temperature.toFixed(1);
            document.getElementById("humidity").innerText    = data.humidity.toFixed(1);
        })
        .catch(err => console.error(err));
}
setInterval(updateData, 5000);
updateData();
</script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
}

void handleData() {
  String json = "{";
  json += "\"flights_today\":" + String(flights_today) + ",";
  json += "\"temperature\":"   + String(temperature, 1) + ",";
  json += "\"humidity\":"      + String(humidity, 1);
  json += "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Starte WLAN...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nVerbunden!");
  Serial.print("IP-Adresse: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);

  server.begin();
  Serial.println("Webserver gestartet");
}

void loop() {
  server.handleClient();
  // später: hier flights_today, temperature, humidity mit echten Sensorwerten aktualisieren
}