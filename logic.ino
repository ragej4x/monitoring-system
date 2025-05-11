#include <OneWire.h>
#include <DallasTemperature.h>

// Pin definitions
const int waterPumpPin = 13;
const int solutionAPin = 12;
const int solutionBPin = 9;  // Changed from 8 to avoid conflict with DS18B20
const int solutionCPin = 7;
const int solutionDPin = 4;

// Sensor pins
const int floaterSwitchPin = A3;
const int phSensorPin = A0;
const int ppmSensorPin = A1;
const int temperatureSensorPin = 2;  // Changed from 8 to match the DS18B20 setup

// OneWire setup for DS18B20 temperature sensor
OneWire oneWire(temperatureSensorPin);
DallasTemperature tempSensors(&oneWire);

// Desired values
float desiredPh = 6.5;   // Set your desired pH level
int desiredPpm = 1000;   // Set your desired PPM value

// Timing variables
unsigned long lastCheckTime = 0;
const unsigned long checkInterval = 20000; // 20 seconds in milliseconds
unsigned long lastLogTime = 0;
const unsigned long logInterval = 5000;  // Log sensor data every 5 seconds

// Sensor data storage
float currentPh = 0.0;
int currentPpm = 0;
float currentTemp = 0.0;

// TDS Sensor Variables
#define VREF 5.0              // analog reference voltage(Volt) of the ADC
#define SCOUNT 30            // sum of sample point
int analogBuffer[SCOUNT];     // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;
float temperature = 25;       // current temperature for compensation

// pH Sensor Variables
float calibration_value = 21.34;
int phval = 0; 
unsigned long int avgval; 
int buffer_arr[10], temp;

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  
  // Set up pump pins as outputs
  pinMode(waterPumpPin, OUTPUT);
  pinMode(solutionAPin, OUTPUT);
  pinMode(solutionBPin, OUTPUT);
  pinMode(solutionCPin, OUTPUT);
  pinMode(solutionDPin, OUTPUT);
  
  // Set all pumps to OFF initially
  turnOffAllPumps();
  
  // Set up sensor pins as inputs
  pinMode(floaterSwitchPin, INPUT_PULLUP);  // Using PULLUP as in floatswitch.ino
  pinMode(phSensorPin, INPUT);
  pinMode(ppmSensorPin, INPUT);
  
  // Initialize DS18B20 temperature sensor
  tempSensors.begin();
  
  Serial.println("-------------------------------------");
  Serial.println("Hydroponic Monitoring System Starting");
  Serial.println("-------------------------------------");
  Serial.println("Target pH: " + String(desiredPh));
  Serial.println("Target PPM: " + String(desiredPpm));
  Serial.println("-------------------------------------");
}

void loop() {
  // Always read sensor values for continuous monitoring
  updateSensorReadings();
  
  // Log sensor data at regular intervals regardless of pump operations
  if (millis() - lastLogTime >= logInterval) {
    logSensorData();
    lastLogTime = millis();
  }
  
  // Check if water level is sufficient using floater switch
  int floaterState = digitalRead(floaterSwitchPin);
  
  // If floater switch is LOW (triggered) - as per floatswitch.ino
  if (floaterState == LOW) {
    // Turn on water pump
    digitalWrite(waterPumpPin, HIGH);
    
    // Check if it's time to perform sensor readings and adjustments
    if (millis() - lastCheckTime >= checkInterval) {
      // Turn off water pump before checking sensors
      digitalWrite(waterPumpPin, LOW);
      Serial.println("\n[SYSTEM] Water pump OFF - Performing checks and adjustments");
      
      // Update sensor readings again to ensure fresh data
      updateSensorReadings();
      
      // Log detailed sensor data
      logDetailedSensorData();
      
      // Check PPM and adjust accordingly
      adjustPpm(currentPpm);
      
      // Check pH and adjust accordingly
      adjustPh(currentPh);
      
      // Update last check time
      lastCheckTime = millis();
      Serial.println("[SYSTEM] Checks and adjustments complete");
      Serial.println("-------------------------------------");
    }
  } else {
    // If floater switch is HIGH (not triggered) - as per floatswitch.ino
    turnOffAllPumps();
    Serial.println("[ALERT] Water level too low - All pumps OFF");
  }
  
  // Small delay to avoid excessive looping
  delay(500);
}

// Function to update all sensor readings
void updateSensorReadings() {
  currentPh = readPh();
  currentPpm = readPpm();
  currentTemp = readTemperature();
  
  // Update TDS temperature compensation with current temperature reading
  temperature = currentTemp;
}

// Function to log basic sensor data periodically
void logSensorData() {
  Serial.print("[SENSORS] pH: ");
  Serial.print(currentPh, 2);
  Serial.print(" | PPM: ");
  Serial.print(currentPpm);
  Serial.print(" | Temp: ");
  Serial.print(currentTemp, 1);
  Serial.println("°C");
}

// Function to log detailed sensor data during adjustments
void logDetailedSensorData() {
  Serial.println("\n-------------------------------------");
  Serial.println("[DETAILED READINGS]");
  Serial.println("pH Value: " + String(currentPh, 2) + " (Target: " + String(desiredPh, 2) + ")");
  Serial.println("PPM Value: " + String(currentPpm) + " (Target: " + String(desiredPpm) + ")");
  Serial.println("Temperature: " + String(currentTemp, 1) + "°C");
  
  // Calculate and display deviations from targets
  float phDeviation = currentPh - desiredPh;
  int ppmDeviation = currentPpm - desiredPpm;
  
  Serial.print("pH Deviation: ");
  Serial.print(phDeviation > 0 ? "+" : "");
  Serial.println(String(phDeviation, 2) + " from target");
  
  Serial.print("PPM Deviation: ");
  Serial.print(ppmDeviation > 0 ? "+" : "");
  Serial.println(String(ppmDeviation) + " from target");
  
  // Float switch status
  String floaterStatus = (digitalRead(floaterSwitchPin) == LOW) ? "Triggered (OK)" : "Not Triggered (Low)";
  Serial.println("Float Switch: " + floaterStatus);
  
  Serial.println("-------------------------------------");
}

// Function to read pH value - Using code from ph_sensor.ino
float readPh() {
  for (int i = 0; i < 10; i++) { 
    buffer_arr[i] = analogRead(phSensorPin);
    delay(10);  // Reduced from 30ms to avoid slowing down the main loop
  }

  // Sort the analog readings
  for (int i = 0; i < 9; i++) {
    for (int j = i + 1; j < 10; j++) {
      if (buffer_arr[i] > buffer_arr[j]) {
        temp = buffer_arr[i];
        buffer_arr[i] = buffer_arr[j];
        buffer_arr[j] = temp;
      }
    }
  }

  avgval = 0;
  for (int i = 2; i < 8; i++) {
    avgval += buffer_arr[i];
  }

  float volt = (float)avgval * 5.0 / 1024 / 6;
  float ph_act = -5.70 * volt + calibration_value;
  
  return ph_act;
}

// Function to read PPM value - Using code from tdsmeter.ino
int readPpm() {
  static unsigned long analogSampleTimepoint = millis();
  
  // Read samples
  if(millis() - analogSampleTimepoint > 40) {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(ppmSensorPin);
    analogBufferIndex++;
    if(analogBufferIndex == SCOUNT) {
      analogBufferIndex = 0;
    }
  }
  
  // Process TDS reading
  for(copyIndex = 0; copyIndex < SCOUNT; copyIndex++) {
    analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
  }
  
  // Get median and convert to voltage
  float averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0;
  
  // Temperature compensation
  float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
  float compensationVoltage = averageVoltage / compensationCoefficient;
  
  // Convert voltage to TDS value
  int tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage - 
                 255.86 * compensationVoltage * compensationVoltage + 
                 857.39 * compensationVoltage) * 0.5;
                 
  return tdsValue;
}

// Function to read temperature - Using code from tempsen.ino
float readTemperature() {
  tempSensors.requestTemperatures();
  float celsius = tempSensors.getTempCByIndex(0);
  
  // Error handling in case sensor isn't connected properly
  if (celsius == -127.00) {
    Serial.println("[ERROR] Temperature sensor not found or not responding!");
    return 25.0; // Return default value if sensor error
  }
  
  return celsius;
}

// Function to adjust PPM by controlling solution A and B pumps
void adjustPpm(int currentPpm) {
  turnOffAllPumps();
  
  if (currentPpm > desiredPpm) {
    // If PPM is too high, add solution A
    Serial.println("[ACTION] PPM too high - Activating Solution A pump");
    digitalWrite(solutionAPin, HIGH);
    delay(5000); // Run pump for 5 seconds
    digitalWrite(solutionAPin, LOW);
    Serial.println("[ACTION] Solution A pump deactivated");
  } else if (currentPpm < desiredPpm) {
    // If PPM is too low, add solution B
    Serial.println("[ACTION] PPM too low - Activating Solution B pump");
    digitalWrite(solutionBPin, HIGH);
    delay(5000); // Run pump for 5 seconds
    digitalWrite(solutionBPin, LOW);
    Serial.println("[ACTION] Solution B pump deactivated");
  } else {
    Serial.println("[STATUS] PPM is at desired level - No adjustment needed");
  }
}

// Function to adjust pH by controlling solution C and D pumps
void adjustPh(float currentPh) {
  turnOffAllPumps();
  
  if (currentPh > desiredPh) {
    // If pH is too high, add solution C
    Serial.println("[ACTION] pH too high - Activating Solution C pump");
    digitalWrite(solutionCPin, HIGH);
    delay(5000); // Run pump for 5 seconds
    digitalWrite(solutionCPin, LOW);
    Serial.println("[ACTION] Solution C pump deactivated");
  } else if (currentPh < desiredPh) {
    // If pH is too low, add solution D
    Serial.println("[ACTION] pH too low - Activating Solution D pump");
    digitalWrite(solutionDPin, HIGH);
    delay(5000); // Run pump for 5 seconds
    digitalWrite(solutionDPin, LOW);
    Serial.println("[ACTION] Solution D pump deactivated");
  } else {
    Serial.println("[STATUS] pH is at desired level - No adjustment needed");
  }
}

// Function to turn off all pumps
void turnOffAllPumps() {
  digitalWrite(waterPumpPin, LOW);
  digitalWrite(solutionAPin, LOW);
  digitalWrite(solutionBPin, LOW);
  digitalWrite(solutionCPin, LOW);
  digitalWrite(solutionDPin, LOW);
}

// Median filtering algorithm from tdsmeter.ino
int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
    
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  
  if ((iFilterLen & 1) > 0) {
    bTemp = bTab[(iFilterLen - 1) / 2];
  } else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  
  return bTemp;
}
