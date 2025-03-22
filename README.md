# ESP32 Home Automation Project

This project implements a home automation system using an ESP32 Dev Module. Follow the instructions below to set up and run the project.

## Hardware Requirements

- ESP32 Dev Module
- USB Cable for programming
- Your home automation peripherals (relays,four male to female connecting wires)

## ESP32 Dev Module Pin Layout

![ESP32 DevKit V1 Pinout](board_circuit_pins/esp32%20devkit%20v1%20pinout.jpeg)

## Software Setup Instructions (Windows)

1. **Install Visual Studio Code**
   - Download and install VSCode from [https://code.visualstudio.com/](https://code.visualstudio.com/)

2. **Install PlatformIO Extension**
   - Open VSCode
   - Go to Extensions (Ctrl+Shift+X)
   - Search for "PlatformIO IDE"
   - Click Install

3. **Clone/Setup the Project**
   - Clone this repository or download the code
   - Open VSCode
   - Click "Open Folder" and select the project directory

4. **Configure the Project**
   - Open `platformio.ini` to verify the configuration
   - Update the WiFi credentials in the code (change your wifi ssid and password)
   - The default board configuration should be:
     ```ini
     [env:esp32dev]
     platform = espressif32
     board = esp32dev
     framework = arduino
     ```

5. **Install Dependencies**
   - PlatformIO will automatically install required dependencies
   - Wait for the process to complete (check the PlatformIO status bar)

## Building and Uploading

1. **Connect ESP32**
   - Connect your ESP32 to your computer via USB
   - Windows should automatically install drivers
   - If not, install CP210x USB driver manually

2. **Select COM Port**
   - Device Manager will show the COM port
   - Note this port number for troubleshooting

3. **Upload Code**
   - Click the PlatformIO Upload button (→) or
   - Use Ctrl+Alt+U to upload(or choose upload framework image from the platformio menu)

## Accessing the Web Interface

1. After uploading, open Serial Monitor in VSCode
2. The ESP32 will print its IP address (e.g., `192.168.178.81`)
3. Open your web browser and enter this IP address
4. The web interface will be accessible on your local network

## Troubleshooting

- If upload fails:
  - Check USB connection
  - Verify COM port
  - Hold BOOT button while uploading
  - Press EN (reset) button after upload

- If web interface is not accessible:
  - Verify WiFi credentials
  - Check ESP32's IP address in Serial Monitor
  - Ensure you're on the same network

## Support

For issues and questions, please open an issue in the repository.