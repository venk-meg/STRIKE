import serial
import csv
import time
import os

# ── USER CONFIG ────────────────────────────────────────────────────────
PORT       = "COM15"   # adjust to your serial port
BAUD       = 500000
hz         = 25        # just for filename
iterations = 10        # how many repeats
record_dur = 3.0       # seconds recording
rest_dur   = 3.0       # seconds resting

# ask for a gesture name
gesture = input("Enter gesture name: ").strip() or "none"

# build filename
filename = f"trial_{gesture}_{iterations}x_{hz}hz.csv"

# ── OPEN SERIAL & CSV ─────────────────────────────────────────────────
ser = serial.Serial(PORT, BAUD, timeout=1)
# make sure folder exists
os.makedirs(os.path.dirname(filename) or ".", exist_ok=True)

with open(filename, "w", newline="", encoding="utf-8") as csvfile:
    writer = csv.writer(csvfile)
    # write header
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

    print(f"\nWill record '{gesture}' {iterations} times → {filename}\n")

    for i in range(1, iterations+1):
        # prep countdown
        print(f"Get ready to record: {gesture} (trial {i} of {iterations})")
        for cnt in (3,2,1):
            print(cnt)
            time.sleep(1)
        print("Recording!")
        
        # Record for record_dur seconds
        end_time = time.time() + record_dur
        while time.time() < end_time:
            raw = ser.readline().decode("ascii", errors="ignore").strip()
            if not raw:
                continue
            parts = [p.strip() for p in raw.split(",")]
            if len(parts) == 20:
                writer.writerow(parts)
        csvfile.flush()

        print(f"Recorded '{gesture}' #{i}")
        if i < iterations:
            print(f"Resting for {rest_dur:.1f}s…\n")
            time.sleep(rest_dur)

    print(f"\n✅ Complete! Saved in \"{filename}\"")
    ser.close()
