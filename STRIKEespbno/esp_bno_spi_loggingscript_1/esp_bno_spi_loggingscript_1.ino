#include <Wire.h>
#include <Adafruit_BNO08x.h>
#include "SPI.h"

#define BNO08X_CS   10
#define BNO08X_INT   8
#define BNO08X_RST  18    // use -1 if you don’t have a reset pin wired

Adafruit_BNO08x bno08x(BNO08X_RST);
sh2_SensorValue_t sensorValue;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  // For SPI mode:
  SPI.begin(/*sck=*/13, /*miso=*/12, /*mosi=*/11, /*ss=*/BNO08X_CS);

  if (!bno08x.begin_SPI(BNO08X_CS, BNO08X_INT, &SPI)) {
    Serial.println("Failed to find BNO08x chip");
    while (1) delay(10);
  }
  Serial.println("BNO08x Found!");

  setReports();
  delay(100);
}

void setReports() {
  Serial.println("Setting desired reports...");
  bno08x.enableReport(SH2_ACCELEROMETER);               // calibrated accel
  bno08x.enableReport(SH2_GYROSCOPE_CALIBRATED);        // calibrated gyro
  bno08x.enableReport(SH2_MAGNETIC_FIELD_CALIBRATED);   // calibrated mag
  bno08x.enableReport(SH2_LINEAR_ACCELERATION);         // remove gravity
  bno08x.enableReport(SH2_GRAVITY);                     // gravity vector
  bno08x.enableReport(SH2_ROTATION_VECTOR);             // 9-axis fusion
  bno08x.enableReport(SH2_GEOMAGNETIC_ROTATION_VECTOR); // mag-+accel fusion
  bno08x.enableReport(SH2_GAME_ROTATION_VECTOR);        // accel+gyro fusion
  bno08x.enableReport(SH2_STEP_COUNTER);                // optional
  bno08x.enableReport(SH2_STABILITY_CLASSIFIER);         // still vs moving
  bno08x.enableReport(SH2_RAW_ACCELEROMETER);           // raw accel (debug)
  bno08x.enableReport(SH2_RAW_GYROSCOPE);               // raw gyro (debug)
  bno08x.enableReport(SH2_RAW_MAGNETOMETER);            // raw mag (debug)
  bno08x.enableReport(SH2_SHAKE_DETECTOR);              // shake event
  bno08x.enableReport(SH2_TAP_DETECTOR);                // tap event
  bno08x.enableReport(SH2_PERSONAL_ACTIVITY_CLASSIFIER); // optional full-body
}

void printActivity(uint8_t activity_id) {
  switch (activity_id) {
    case PAC_UNKNOWN:       Serial.print("Unknown");     break;
    case PAC_IN_VEHICLE:    Serial.print("In Vehicle");  break;
    case PAC_ON_BICYCLE:    Serial.print("On Bicycle");  break;
    case PAC_ON_FOOT:       Serial.print("On Foot");     break;
    case PAC_STILL:         Serial.print("Still");       break;
    case PAC_TILTING:       Serial.print("Tilting");     break;
    case PAC_WALKING:       Serial.print("Walking");     break;
    case PAC_RUNNING:       Serial.print("Running");     break;
    case PAC_ON_STAIRS:     Serial.print("On Stairs");   break;
    default:                Serial.print("NOT LISTED");  break;
  }
  Serial.print(" ("); Serial.print(activity_id); Serial.print(")");
}

void loop() {
  delay(10);

  if (bno08x.wasReset()) {
    Serial.println("Sensor was reset—re‐enabling reports.");
    setReports();
  }
  if (!bno08x.getSensorEvent(&sensorValue)) {
    return;
  }

  // Print a millisecond timestamp, then the sensor name, then its values:
  Serial.print(millis());
  Serial.print(",");

  switch (sensorValue.sensorId) {

    case SH2_ACCELEROMETER:
      Serial.print("ACCEL,");
      Serial.print(sensorValue.un.accelerometer.x); Serial.print(",");
      Serial.print(sensorValue.un.accelerometer.y); Serial.print(",");
      Serial.println(sensorValue.un.accelerometer.z);
      break;

    case SH2_GYROSCOPE_CALIBRATED:
      Serial.print("GYRO,");
      Serial.print(sensorValue.un.gyroscope.x); Serial.print(",");
      Serial.print(sensorValue.un.gyroscope.y); Serial.print(",");
      Serial.println(sensorValue.un.gyroscope.z);
      break;

    case SH2_MAGNETIC_FIELD_CALIBRATED:
      Serial.print("MAGCAL,");
      Serial.print(sensorValue.un.magneticField.x); Serial.print(",");
      Serial.print(sensorValue.un.magneticField.y); Serial.print(",");
      Serial.println(sensorValue.un.magneticField.z);
      break;

    case SH2_LINEAR_ACCELERATION:
      Serial.print("LINACC,");
      Serial.print(sensorValue.un.linearAcceleration.x); Serial.print(",");
      Serial.print(sensorValue.un.linearAcceleration.y); Serial.print(",");
      Serial.println(sensorValue.un.linearAcceleration.z);
      break;

    case SH2_GRAVITY:
      Serial.print("GRAV,");
      Serial.print(sensorValue.un.gravity.x); Serial.print(",");
      Serial.print(sensorValue.un.gravity.y); Serial.print(",");
      Serial.println(sensorValue.un.gravity.z);
      break;

    case SH2_ROTATION_VECTOR:
      Serial.print("ROT,");
      Serial.print(sensorValue.un.rotationVector.real); Serial.print(",");
      Serial.print(sensorValue.un.rotationVector.i);    Serial.print(",");
      Serial.print(sensorValue.un.rotationVector.j);    Serial.print(",");
      Serial.println(sensorValue.un.rotationVector.k);
      break;

    case SH2_GEOMAGNETIC_ROTATION_VECTOR:
      Serial.print("GEORM,");
      Serial.print(sensorValue.un.geoMagRotationVector.real); Serial.print(",");
      Serial.print(sensorValue.un.geoMagRotationVector.i);    Serial.print(",");
      Serial.print(sensorValue.un.geoMagRotationVector.j);    Serial.print(",");
      Serial.println(sensorValue.un.geoMagRotationVector.k);
      break;

    case SH2_GAME_ROTATION_VECTOR:
      Serial.print("GAMEROT,");
      Serial.print(sensorValue.un.gameRotationVector.real); Serial.print(",");
      Serial.print(sensorValue.un.gameRotationVector.i);    Serial.print(",");
      Serial.print(sensorValue.un.gameRotationVector.j);    Serial.print(",");
      Serial.println(sensorValue.un.gameRotationVector.k);
      break;

    case SH2_STEP_COUNTER:
      Serial.print("STEP,");
      Serial.print(sensorValue.un.stepCounter.steps);   Serial.print(",");
      Serial.println(sensorValue.un.stepCounter.latency);
      break;

    case SH2_STABILITY_CLASSIFIER: {
      Serial.print("STAB,");
      sh2_StabilityClassifier_t stability = sensorValue.un.stabilityClassifier;
      switch (stability.classification) {
        case STABILITY_CLASSIFIER_UNKNOWN:     Serial.println("Unknown");  break;
        case STABILITY_CLASSIFIER_ON_TABLE:    Serial.println("On Table"); break;
        case STABILITY_CLASSIFIER_STATIONARY:  Serial.println("Stationary"); break;
        case STABILITY_CLASSIFIER_STABLE:      Serial.println("Stable");  break;
        case STABILITY_CLASSIFIER_MOTION:      Serial.println("In Motion"); break;
        default:                               Serial.println("N/A");    break;
      }
      break;
    }

    case SH2_RAW_ACCELEROMETER:
      Serial.print("RAWACC,");
      Serial.print(sensorValue.un.rawAccelerometer.x); Serial.print(",");
      Serial.print(sensorValue.un.rawAccelerometer.y); Serial.print(",");
      Serial.println(sensorValue.un.rawAccelerometer.z);
      break;

    case SH2_RAW_GYROSCOPE:
      Serial.print("RAWGYRO,");
      Serial.print(sensorValue.un.rawGyroscope.x); Serial.print(",");
      Serial.print(sensorValue.un.rawGyroscope.y); Serial.print(",");
      Serial.println(sensorValue.un.rawGyroscope.z);
      break;

    case SH2_RAW_MAGNETOMETER:
      Serial.print("RAWMAG,");
      Serial.print(sensorValue.un.rawMagnetometer.x); Serial.print(",");
      Serial.print(sensorValue.un.rawMagnetometer.y); Serial.print(",");
      Serial.println(sensorValue.un.rawMagnetometer.z);
      break;

    case SH2_SHAKE_DETECTOR: {
      Serial.print("SHAKE,");
      sh2_ShakeDetector_t sd = sensorValue.un.shakeDetector;
      switch (sd.shake) {
        case SHAKE_X: Serial.println("X");  break;
        case SHAKE_Y: Serial.println("Y");  break;
        case SHAKE_Z: Serial.println("Z");  break;
        default:      Serial.println("None"); break;
      }
      break;
    }

    case SH2_TAP_DETECTOR:
      // The Adafruit SH-2 struct for tap detector does NOT expose .direction or .type
      // We just report “TAP” whenever a tap is detected:
      Serial.println("TAP");
      break;

    case SH2_PERSONAL_ACTIVITY_CLASSIFIER: {
      Serial.print("ACT,");
      sh2_PersonalActivityClassifier_t act = sensorValue.un.personalActivityClassifier;
      Serial.print("MostLikely=");
      printActivity(act.mostLikelyState);
      Serial.println("");
      Serial.println("Confidences:");
      for (uint8_t i = 0; i < PAC_OPTION_COUNT; i++) {
        Serial.print("\t");
        printActivity(i);
        Serial.print(": ");
        Serial.println(act.confidence[i]);
      }
      break;
    }

    default:
      // Uncomment if you want to see un‐handled sensor IDs:
      // Serial.print("ID "); Serial.print(sensorValue.sensorId); Serial.println(" skipped.");
      break;
  }
}
