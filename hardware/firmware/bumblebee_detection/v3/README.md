# Bumblebee Detect Example

Dieses Beispiel nimmt automatisch pro Sekunde zwei Bilder mit der Kamera auf, wendet das Bumblebee-Detektionsmodell darauf an, zählt die Ein- und Ausflüge, speichert das Bild mit den erkannten Ergebnissen (Bounding Boxen) auf der SD-Karte und schickt die Zahlen der Ein- und Ausflüge an das Dashboard. Die Erkennung läuft dabei kontinuierlich, das Bild wird jeweils als JPEG abgelegt.

## Deployment

1. ESP-IDF installieren: [https://dl.espressif.com/dl/esp-idf/](https://dl.espressif.com/dl/esp-idf/)
2. In das Projektverzeichnis wechseln (Pfad anpassen):
	```
	cd C:\Users\nilsg\repos\BeeSense\hardware\firmware\bumblebee_detection\v3
	```
3. Target setzen (nur beim ersten build):
	```
	idf.py set-target esp32s3
	```
4. Firmware bauen:
	```
	idf.py build
	```
5. Flashen und Monitor starten (Port ggf. anpassen):
	```
	idf.py -p COM3 flash monitor
	```