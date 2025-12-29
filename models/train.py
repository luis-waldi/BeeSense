from ultralytics import YOLO

# Modell laden
model = YOLO('yolo11n.pt')

# Training starten
model.train(
    data='yolov11_bumblebee.yaml',
    imgsz=224,
    epochs=100,
    batch=16,
    cache=True
)
