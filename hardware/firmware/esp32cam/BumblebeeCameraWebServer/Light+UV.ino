// UV Light Sensor - Vishay VEML6070

#include <Wire.h>
#include <Adafruit_VEML6070.h>

#define PIN_QWIIC_SDA 2
#define PIN_QWIIC_SCL 1

Adafruit_VEML6070 uv = Adafruit_VEML6070();

void scanI2C() {
  Serial.println("Scanning I2C bus...");
  byte count = 0;
  
  for (byte i = 1; i < 127; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found device at 0x");
      if (i < 16) Serial.print("0");
      Serial.println(i, HEX);
      count++;
    }
  }
  
  if (count == 0) {
    Serial.println("No I2C devices found!");
  } else {
    Serial.print("Found ");
    Serial.print(count);
    Serial.println(" device(s)");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\nVEML6070 UV Sensor Test");
  
  Wire.begin(PIN_QWIIC_SDA, PIN_QWIIC_SCL);
  Wire.setClock(100000); // 100kHz for better stability
  
  delay(100);
  
  // Scan for I2C devices
  scanI2C();
  
  // Initialize VEML6070 sensor
  // Integration time options: VEML6070_HALF_T, VEML6070_1_T, VEML6070_2_T, VEML6070_4_T
  Serial.println("\nInitializing VEML6070...");
  uv.begin(VEML6070_4_T);
  Serial.println("VEML6070 initialized!");
  
  delay(500);
}

void loop() {
  uint16_t uvLevel = uv.readUV();
  
  Serial.print("UV Light Level (raw): ");
  Serial.print(uvLevel);
  Serial.print(" | ");
  
  // Calculate UV Index (approximate formula)
  float uvIndex = uvLevel / 100.0;
  Serial.print("UV Index: ");
  Serial.print(uvIndex, 2);
  Serial.print(" | ");
  
  // UV Index classification
  if (uvIndex < 3) {
    Serial.println("Low");
  } else if (uvIndex < 6) {
    Serial.println("Moderate");
  } else if (uvIndex < 8) {
    Serial.println("High");
  } else if (uvIndex < 11) {
    Serial.println("Very High");
  } else {
    Serial.println("Extreme");
  }
  
  delay(2000);
}
