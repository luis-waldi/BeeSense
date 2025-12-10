// Temperature and Humidity Display

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_HDC1000.h>

#define PIN_QWIIC_SDA 2
#define PIN_QWIIC_SCL 1

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_HDC1000 hdc = Adafruit_HDC1000();

void displaySensorData() {
  display.clearDisplay();
  
  // Read sensor data
  float temp = hdc.readTemperature();
  float humi = hdc.readHumidity();
  
  // Display title
  display.setTextSize(1);
  display.setTextColor(WHITE, BLACK);
  display.setCursor(0, 0);
  display.println("BeeSense Sensor");
  
  // Draw separator line
  display.drawLine(0, 10, SCREEN_WIDTH, 10, WHITE);
  
  // Display temperature
  display.setTextSize(2);
  display.setCursor(0, 18);
  display.print("T: ");
  display.print(temp, 1);
  display.println(" C");
  
  // Display humidity
  display.setCursor(0, 40);
  display.print("H: ");
  display.print(humi, 1);
  display.println(" %");
  
  display.display();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(PIN_QWIIC_SDA,PIN_QWIIC_SCL);
  
  // Initialize display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  display.setRotation(2);
  display.display();
  delay(100);
  display.clearDisplay();
  
  // Initialize HDC1000 sensor
  hdc.begin();
  
  // Show startup message
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("BeeSense");
  display.println("Initializing...");
  display.display();
  delay(2000);
}

void loop() {
  // Display sensor data
  displaySensorData();
  
  // Also print to Serial for debugging
  Serial.print("Temperature: ");
  Serial.print(hdc.readTemperature());
  Serial.print(" °C, Humidity: ");
  Serial.print(hdc.readHumidity());
  Serial.println(" %");
  
  // Update every 2 seconds
  delay(2000);
}