# studyproject-bee-counting

# BeeSense Dashboard (PC Simulation)

Dieses Dashboard ist die **UI-Simulation** des BeeSense/Hummelstock-Projekts.  
Es dient dazu, die geplante Weboberfläche *ohne ESP32-Hardware* lokal im Browser zu entwickeln und zu testen.

Der Branch **`dashboard`** enthält ausschließlich den Web-Teil (HTML, CSS, JS) und ist **getrennt vom Firmware-Code** im `main`-Branch.

---

## Ziel des Dashboards

Das Dashboard soll später:

- die Live-Werte aus der ESP32-Firmware anzeigen  
- historische Daten (Flüge, Temperatur, Feuchtigkeit) visualisieren  
- Standortinformationen (GPS/Koordinaten) anzeigen  
- Warnungen und Statusinfos für den Imker geben  
- als Grundlage für die mobile/remote Weboberfläche dienen  

Aktuell läuft das Dashboard **mit simulierten Testdaten**, um UI-Design und Logik unabhängig von der Hardware entwickeln zu können.

---

## Aktuelle Funktionen

### ✔ Übersicht
- Live-Simulation von:
  - Flügen
  - Temperatur
  - Luftfeuchtigkeit
- Ampelstatus für Temperatur/Feuchtigkeit

### ✔ Flüge
- Tagesflüge
- Balkendiagramm der letzten 7 Tage (ohne heutigen Tag)

### ✔ Temperatur
- aktuelle Temperatur
- Liniendiagramm: Ø Temperatur der letzten 7 Tage (ohne heute)

### ✔ Standort
- Interaktive Karte (Leaflet)
- Eingabe von Koordinaten durch den Imker
- Speicherung im Browser (localStorage)
- Rechtskarte höhenverstellbar

### ✔ Navigation
- Reiterstruktur (Tabs) für saubere Übersicht

---

## 🛠 Projektstruktur
dashboard/
│── index.html → Hauptdatei (UI, Charts, Karte, Simulation)
│── style.css → Farbschema, Layout, Kartenfarben
│── assets/ (optional)
│── README.md → Diese Datei


Das Dashboard wird **nicht** in das PlatformIO-Projekt eingebettet, sondern **parallel getrennt entwickelt**.

---

## Zukunft: Anbindung an PlatformIO / ESP32 Firmware

Das Dashboard wird später **nicht lokal simuliert**, sondern:

1. **über einen Webserver auf dem ESP32 bereitgestellt**  
2. Daten über eine **REST-API oder WebSocket** vom ESP32 beziehen  
3. historische Daten aus dem Firmware-Speicher (Flash / SD-Karte) oder dem Backend laden  

### Geplant:

- `/api/flights/today`
- `/api/flights/history`  
- `/api/temperature/current`
- `/api/temperature/history`
- `/api/humidity/current`
- `/api/humidity/history`
- `/api/location`

Diese API-Struktur wird später im `main` oder einem eigenen Backend-Branch definiert.

---

## Dashboard lokal starten

Es werden **keine zusätzlichen Installationen** benötigt.

### Variante A – direkt per Browser öffnen

Einfach doppelklick auf:

index.html

Dann Browser öffnen:

http://localhost:5500


## Quellen
https://data.mendeley.com/datasets/8gb9r2yhfc/6
