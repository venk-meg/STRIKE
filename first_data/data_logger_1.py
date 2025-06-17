#!/usr/bin/env python3
import serial
import csv
import time

# ── USER CONFIG ───────────────────────────────────────────────────────────────
PORT       = 'COM11'         # ← your COM port
BAUDRATE   = 115200
CSV_FILE   = 'previous_harmony.csv'
DURATION   = 2.0            # seconds to record per run
# ────────────────────────────────────────────────────────────────────────────────

def main():
    ser = serial.Serial(PORT, BAUDRATE, timeout=0.1)
    print(f"Recording from {PORT} @ {BAUDRATE} baud for {DURATION}s...")

    # open CSV in utf-8 so we can write anything
    with open(CSV_FILE, 'a', newline='', encoding='utf-8') as csvf:
        writer = csv.writer(csvf)

        # first row divider
        print("START")
        writer.writerow(["START"])
        csvf.flush()

        start = time.monotonic()
        while time.monotonic() - start < DURATION:
            raw = ser.readline()
            if not raw:
                continue

            # drop any bytes that aren't valid UTF-8
            text = raw.decode('utf-8', errors='ignore').strip()
            if not text:
                continue

            print(text)                   # echo to console
            writer.writerow(text.split(','))  # split on commas
            csvf.flush()

    ser.close()
    print("Done.")

if __name__ == '__main__':
    main()
