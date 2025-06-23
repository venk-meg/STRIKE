import asyncio
import os
from bleak import BleakScanner, BleakClient
from datetime import datetime
import re

timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
filename = f"sensor_data_{timestamp}.csv"

# Nordic UART Service UUIDs
UART_SERVICE_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
UART_TX_CHAR_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

buffer = ""

# Ensure file is created with header
with open(filename, "w", encoding="utf-8") as f:
    f.write("timestamp,accel_x,accel_y,accel_z,gyro_x,gyro_y,gyro_z,"
            "mag_x,mag_y,mag_z,linacc_x,linacc_y,linacc_z,"
            "grav_x,grav_y,grav_z,gamerot_w,gamerot_x,gamerot_y,gamerot_z\n")


def notification_handler(sender: str, data: bytes):
    text = data.decode('utf-8', errors='ignore')
    print("RAW incoming BLE:", text.strip())  # Terminal output

    with open(filename, "a", encoding="utf-8") as f:
        f.write(text)


async def run():
    print("Scanning for BLE device...")
    device = await BleakScanner.find_device_by_address("D8:BE:94:5B:55:B3")

    if not device:
        print("Device NOT found. Check if advertising and try again.")
        return

    async with BleakClient(device) as client:
        print("Connected to", device.address)
        await client.start_notify(UART_TX_CHAR_UUID, notification_handler)
        print("Streaming... (Ctrl+C to stop)")

        try:
            while True:
                await asyncio.sleep(1)
        except KeyboardInterrupt:
            print("Disconnecting...")
            await client.stop_notify(UART_TX_CHAR_UUID)

asyncio.run(run())
