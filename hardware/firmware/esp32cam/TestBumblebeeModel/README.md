# Test Bumble

Dieses Projekt testet ausschließlich das Bumblebee-Detektionsmodell auf einem einzelnen Bild.

**Ablauf:**
- Es wird nur das Bild `/test/test.jpg` von der SD-Karte geladen und für die Inferenz verwendet.
- Die Kamera wird nicht genutzt.
- Das Modell muss auf der SD-Karte im Verzeichnis `/models/yolov8_bumblebee_quantized.espdl` liegen.
- Die Erkennungsergebnisse (Hummeln) werden auf dem Serial Monitor ausgegeben.