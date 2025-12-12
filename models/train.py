from ultralytics import YOLO

# Modell laden
model = YOLO('yolov11n.pt')

# Training starten
model.train(
    data='yolov11_bumblebee.yaml',
    imgsz=320,
    epochs=50,
    batch=16,
    cache=True
)
