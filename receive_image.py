import serial
import struct
from datetime import datetime

image_counter = 1

# com_input = str(input("Please enter a COM port: "))

# Open Serial Port
ser = serial.Serial('COM3', 115200, timeout=10)

while True:

    if image_counter == 7:
        exit()

    # Receive image size
    line = ser.readline().strip()
    if line == b"Image Start...":
        fb_size_raw = ser.read(4)
        if len(fb_size_raw) != 4:
            print("Failed to receive size")
        else:
            fb_size = struct.unpack('<I', fb_size_raw)[0]

            # Receive image data
            image_data = ser.read(fb_size)

            end_line = ser.readline().strip()
            if end_line == b"Image End...":
                print(f"Image Data Received: {fb_size} bytes")
                with open(f"./ImageDump/testImage_{image_counter}.jpg", "wb") as f:
                    f.write(image_data)
                image_counter += 1

            else:
                print("Image reception failed T_T")
