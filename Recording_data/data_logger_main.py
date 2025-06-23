import serial
import csv
import time

logtrial = 1
gesturename = "none"
hz = 25
iterations = 20 #2 seconds active, 3 seconds rest

filename = "trial" + str(logtrial) + "_" + gesturename + "_" + str(iterations) + "iterations_" + str(hz) + "hz" + ".csv"

# 1) adjust this to your port:
PORT = "COM15"          # Windows e.g. "COM3"
# PORT = "/dev/ttyACM0" # Linux / macOS
BAUD  = 500000

# 2) open serial
ser = serial.Serial(PORT, BAUD, timeout=1)

# 3) open CSV and write header
with open(filename, "w", newline="") as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow([
        "timestamp",
        "accel_x","accel_y","accel_z",
        "gyro_x","gyro_y","gyro_z",
        "mag_x","mag_y","mag_z",
        "linacc_x","linacc_y","linacc_z",
        "grav_x","grav_y","grav_z",
        "gamerot_w","gamerot_x","gamerot_y","gamerot_z"
    ])
    csvfile.flush()

    print(f"Logging incoming serial on {PORT} → sensor_data.csv\nPress Ctrl-C to stop.")
    try:
        while True:
            raw = ser.readline().decode("ascii", errors="ignore").strip()
            if not raw:
                continue

            # split on commas
            parts = [p.strip() for p in raw.split(",")]

            # only log complete 20-field rows
            if len(parts) == 20:
                writer.writerow(parts)
                csvfile.flush()
                print(raw)
            else:
                # you’ll see these if a line got cut off
                print(f"⚠️ skipped (wrong field count): {raw}")

    except KeyboardInterrupt:
        print("\nDone logging.")
        ser.close()
