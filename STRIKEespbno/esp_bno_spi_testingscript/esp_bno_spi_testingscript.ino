// read different sensor data
#include <Wire.h>
#include <Adafruit_BNO08x.h>
#include "SPI.h"

#define BNO08X_CS 10
#define SPI_MOSI 11
#define SPI_MISO 12
#define SPI_SCK 13
#define BNO08X_INT 8
#define BNO08X_RST 18 //18 if you want to enable it

Adafruit_BNO08x  bno08x(BNO08X_RST);
SPIClass spi = SPIClass(HSPI);
sh2_SensorValue_t sensorValue;

void setup(void) {
  Serial.begin(115200);
  while (!Serial) delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit BNO08x test!");
  spi.begin(SPI_SCK, SPI_MISO, SPI_MOSI, BNO08X_CS);

  // Try to initialize!
  if (!bno08x.begin_SPI(BNO08X_CS, BNO08X_INT, &spi)) {
    Serial.println("BNO08x not detected. Check your jumpers and the hookup guide. Freezing...");
    while (1)
      ;
  }
  Serial.println("BNO08x Found!");

  for (int n = 0; n < bno08x.prodIds.numEntries; n++) {
    Serial.print("Part ");
    Serial.print(bno08x.prodIds.entry[n].swPartNumber);
    Serial.print(": Version :");
    Serial.print(bno08x.prodIds.entry[n].swVersionMajor);
    Serial.print(".");
    Serial.print(bno08x.prodIds.entry[n].swVersionMinor);
    Serial.print(".");
    Serial.print(bno08x.prodIds.entry[n].swVersionPatch);
    Serial.print(" Build ");
    Serial.println(bno08x.prodIds.entry[n].swBuildNumber);
  }

  setReports();

  Serial.println("Reading events");
  delay(100);
}

// Here is where you define the sensor outputs you want to receive
void setReports(void) {
  Serial.println("Setting desired reports");
  if (! bno08x.enableReport(SH2_GAME_ROTATION_VECTOR)) {
    Serial.println("Could not enable game vector");
  }
  if (!bno08x.enableReport(SH2_MAGNETIC_FIELD_CALIBRATED)) {
    Serial.println("Could not enable magnetic field calibrated");
  }
}


void loop() {
  delay(100);

  if (bno08x.wasReset()) {
    Serial.print("sensor was reset ");
    setReports();
  }
  
  if (! bno08x.getSensorEvent(&sensorValue)) {
    return;
  }
  
  switch (sensorValue.sensorId) {
    
    case SH2_GAME_ROTATION_VECTOR:
      Serial.print("Game Rotation Vector - r: ");
      Serial.print(sensorValue.un.gameRotationVector.real);
      Serial.print(" i: ");
      Serial.print(sensorValue.un.gameRotationVector.i);
      Serial.print(" j: ");
      Serial.print(sensorValue.un.gameRotationVector.j);
      Serial.print(" k: ");
      Serial.println(sensorValue.un.gameRotationVector.k);
      break;

    case SH2_MAGNETIC_FIELD_CALIBRATED:
    Serial.print("Magnetic Field - x: ");
    Serial.print(sensorValue.un.magneticField.x);
    Serial.print(" y: ");
    Serial.print(sensorValue.un.magneticField.y);
    Serial.print(" z: ");
    Serial.println(sensorValue.un.magneticField.z);
    break;

  }

}