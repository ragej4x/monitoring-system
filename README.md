# Hydroponic Monitoring System

A web-based monitoring system for hydroponic setups that connects an Arduino-based sensor system with a web interface through Firebase Realtime Database.

## Architecture

- **Arduino**: Collects sensor data (pH, PPM, temperature, water level) and controls pumps
- **Firebase**: Acts as the real-time database for storing sensor data, logs, and system state
- **Flask**: Provides the web server and API endpoints
- **Frontend**: HTML/CSS/JS dashboard for monitoring and control

## Setup Instructions

### Prerequisites

- Python 3.8 or higher
- Arduino IDE
- Firebase account
- Internet-connected Arduino board (with WiFi capability)

### Firebase Setup

1. Create a Firebase project at [Firebase Console](https://console.firebase.google.com/)
2. Set up a Realtime Database
3. Note your Firebase configuration details:
   - API Key
   - Auth Domain
   - Database URL
   - Project ID
   - Storage Bucket
   - Messaging Sender ID
   - App ID

### Arduino Setup

1. Upload the `firebase_hydro.ino` sketch to your Arduino board
2. Update the WiFi credentials and Firebase configuration in the sketch
3. Connect sensors and pumps according to the pin definitions in the code

### Flask Server Setup

1. Clone this repository
2. Install required packages:
   ```
   pip install -r requirements.txt
   ```
3. Update the Firebase configuration in `app.py` with your Firebase project details
4. Run the Flask server:
   ```
   python app.py
   ```
5. Access the dashboard at `http://localhost:5000`

## API Endpoints

- `/api/sensor_data` - Get current sensor readings, pump status, and target values
- `/api/history` - Get historical sensor data
- `/api/logs` - Get system logs

## Features

- Real-time monitoring of pH, PPM, temperature, and water level
- Manual control of water and nutrient pumps
- Automatic adjustment of pH and nutrient levels based on target values
- Historical data visualization
- System logs

## License

MIT License

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## System Components

### Hardware Components
- Arduino with WiFi capabilities (Arduino Nano RP2040 Connect, Arduino UNO WiFi Rev2, etc.)
- pH sensor - connected to pin A0
- PPM/TDS sensor - connected to pin A1
- DS18B20 temperature sensor - connected to pin 2
- Float switch - connected to pin A3
- 5 pumps:
  - Water pump - pin 13
  - Solution A pump - pin 12
  - Solution B pump - pin 9
  - Solution C pump - pin 7
  - Solution D pump - pin 4

### Software Components
- Arduino firmware (`firebase_hydro.ino`)
- Flask web application for monitoring and control
- Firebase Realtime Database for data storage and communication

## Setup Instructions

### 1. Firebase Setup
1. Use the existing Firebase project at `iot-monitoringsys-default-rtdb.asia-southeast1.firebasedatabase.app`
2. The database structure will be automatically created by the Arduino code

### 2. Arduino Setup
1. Install required libraries:
   - WiFiS3 (or WiFiNINA depending on your board)
   - ArduinoJson
   - OneWire
   - DallasTemperature
2. Open the `firebase_hydro.ino` file in Arduino IDE
3. Update WiFi credentials with your network details:
   ```cpp
   #define WIFI_SSID "YOUR_WIFI_SSID"
   #define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
   ```
4. Upload the code to your Arduino

### 3. Flask Web Application Setup
1. Install Python requirements:
   ```
   pip install -r requirements.txt
   ```
2. Start the Flask application:
   ```
   python app.py
   ```
3. Access the web interface at http://localhost:5000

## System Operation

### Automatic Mode
The system automatically:
- Monitors water level using the float switch
- Measures pH, PPM, and temperature every 5 seconds
- Records data to Firebase every 10 seconds
- Adjusts nutrient levels based on sensor readings every 20 seconds
- Turns on appropriate pumps to maintain target pH and PPM levels

### Web Interface
The web interface allows you to:
- View real-time sensor readings
- Set target values for pH and PPM
- Manually control pumps
- View historical data in graphs

## Pump Functions
- **Water Pump (pin 13)**: Fills the system when water level is low
- **Solution A (pin 12)**: Used when PPM is too high
- **Solution B (pin 9)**: Used when PPM is too low
- **Solution C (pin 7)**: Used when pH is too high
- **Solution D (pin 4)**: Used when pH is too low

## Troubleshooting

### Arduino Connectivity Issues
- Check that WiFi credentials are correct
- Ensure the Arduino has a stable internet connection
- Verify Firebase host and auth key are correct

### Sensor Reading Issues
- Calibrate the pH sensor by adjusting the `calibration_value` variable
- Ensure all sensors are properly connected to the correct pins
- Check that the float switch is properly positioned in the water tank

### Web Interface Issues
- Verify that the Flask application is running
- Check that your computer can access the Firebase database
- Ensure proper network connectivity for real-time updates

## Maintenance
- Regularly clean and calibrate pH and PPM sensors
- Check and clean pump tubing to prevent blockages
- Monitor solution levels and refill as needed

Connection documentation

DTS ES :
   pin A1
   5.0vcc
   GRD


TEMP SEN:
   pin 8
   5.0vcc
   GRDs

   5.1k or 10k  ohm resistor between pin a5 and 5v vcc 
   dont overpass the 10k limit = abnormal output



PH TDS:
   pin A0 - Po
   5.0vcc
   GRD


  +3.3V
        |
    [Pump +]
        |
[Collector of 2n2222]
        |
    [Pump −]
        |
[Emitter of 2n2222]
        |
    Arduino Pin 2 (OUTPUT)
        |
    GND



        _______
       |       |
       | 2N2222|
       |_______|
        | | |
        E B C

Alternatives - BC547 if ever wlang 2n2222 


Pump + is powered from 3.3V pin of the Arduino.

Pump − is controlled via the 2n2222 transistor, which is connected to Arduino pin 2.

Arduino Pin 2 controls the transistor's base to switch the pump on/off.

Arduino GND is connected to the emitter of the 2n2222 and pump's −.




Pump Power (+) Connection:

Connect the Pump's + pin to the 5V pin (or 3.3V pin) on the Arduino, depending on your pump's rating.

Pump Ground (−) Connection:

Connect the Pump's − pin to the Collector of the 2n2222 transistor.

The Emitter of the 2n2222 will be connected to Arduino GND.

Base of 2n2222:

The Base of the 2n2222 is connected to Arduino Pin 2 — this controls when the pump is switched on or off.