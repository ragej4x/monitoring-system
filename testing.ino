// Pin definitions
#define WATER_PUMP_PIN 13
#define SOL_A_PUMP_PIN 12
#define SOL_B_PUMP_PIN 8
#define PH_UP_PUMP_PIN 7
#define PH_DOWN_PUMP_PIN 4
#define FLOAT_SWITCH_PIN A5
#define TDS_SENSOR_PIN A1
#define PH_SENSOR_PIN A0
#define ONE_WIRE_BUS 2

#include <OneWire.h>
#include <DallasTemperature.h>

// TDS sensor variables
#define VREF 5.0
#define SCOUNT  30
int analogBuffer[SCOUNT];
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
float averageVoltage = 0;
float tdsValue = 0;
float temperature = 25; // default, will update from temp sensor

// pH sensor variables
float calibration_value = 21.34;
int buffer_arr[10], temp;

// Temperature sensor
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(9600);
  pinMode(WATER_PUMP_PIN, OUTPUT);
  pinMode(SOL_A_PUMP_PIN, OUTPUT);
  pinMode(SOL_B_PUMP_PIN, OUTPUT);
  pinMode(PH_UP_PUMP_PIN, OUTPUT);
  pinMode(PH_DOWN_PUMP_PIN, OUTPUT);
  pinMode(FLOAT_SWITCH_PIN, INPUT_PULLUP);
  pinMode(TDS_SENSOR_PIN, INPUT);
  sensors.begin();
  // Turn off all pumps initially
  digitalWrite(WATER_PUMP_PIN, LOW);
  digitalWrite(SOL_A_PUMP_PIN, LOW);
  digitalWrite(SOL_B_PUMP_PIN, LOW);
  digitalWrite(PH_UP_PUMP_PIN, LOW);
  digitalWrite(PH_DOWN_PUMP_PIN, LOW);
}

// Helper: Median filter for TDS
int getMedianNum(int bArray[], int iFilterLen){
  int bTab[iFilterLen];
  for (byte i = 0; i<iFilterLen; i++) bTab[i] = bArray[i];
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

void loop() {
  // 1. Read Water Level
  bool waterLow = (digitalRead(FLOAT_SWITCH_PIN) == LOW);
  Serial.print("Water Level: "); Serial.println(waterLow ? "LOW" : "OK");

  // 2. Read Temperature
  sensors.requestTemperatures();
  temperature = sensors.getTempCByIndex(0);
  Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" C");

  // 3. Read TDS
  for (int i = 0; i < SCOUNT; i++) {
    analogBuffer[i] = analogRead(TDS_SENSOR_PIN);
    delay(5);
  }
  averageVoltage = getMedianNum(analogBuffer, SCOUNT) * (float)VREF / 1024.0;
  float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
  float compensationVoltage = averageVoltage / compensationCoefficient;
  tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage
              - 255.86 * compensationVoltage * compensationVoltage
              + 857.39 * compensationVoltage) * 0.5;
  Serial.print("TDS Value: "); Serial.print(tdsValue, 0); Serial.println(" ppm");

  // 4. Read pH
  for (int i = 0; i < 10; i++) {
    buffer_arr[i] = analogRead(PH_SENSOR_PIN);
    delay(10);
  }
  // Sort
  for (int i = 0; i < 9; i++) {
    for (int j = i + 1; j < 10; j++) {
      if (buffer_arr[i] > buffer_arr[j]) {
        temp = buffer_arr[i];
        buffer_arr[i] = buffer_arr[j];
        buffer_arr[j] = temp;
      }
    }
  }
  unsigned long avgval = 0;
  for (int i = 2; i < 8; i++) avgval += buffer_arr[i];
  float volt = (float)avgval * 5.0 / 1024 / 6;
  float ph_act = -5.70 * volt + calibration_value;
  Serial.print("pH Value: "); Serial.println(ph_act);

  // --- Pump Logic ---
  // Only one pump can be ON at a time (priority: Water > Sol A > Sol B > pH UP > pH DOWN)
  // Turn all pumps OFF first
  digitalWrite(WATER_PUMP_PIN, LOW);
  digitalWrite(SOL_A_PUMP_PIN, LOW);
  digitalWrite(SOL_B_PUMP_PIN, LOW);
  digitalWrite(PH_UP_PUMP_PIN, LOW);
  digitalWrite(PH_DOWN_PUMP_PIN, LOW);

  float tdsSetpoint = 800; // example setpoint
  
  // Main pump control logic - properly structured if/else chain
  if (waterLow) {
    digitalWrite(WATER_PUMP_PIN, HIGH);
    Serial.println("Water Pump: ON");
    Serial.println("Solution A Pump: OFF");
    Serial.println("Solution B Pump: OFF");
    Serial.println("pH UP Pump: OFF");
    Serial.println("pH DOWN Pump: OFF");
  } else if (tdsValue < tdsSetpoint) {
    digitalWrite(SOL_A_PUMP_PIN, HIGH);
    Serial.println("Water Pump: OFF");
    Serial.println("Solution A Pump: ON");
    Serial.println("Solution B Pump: OFF");
    Serial.println("pH UP Pump: OFF");
    Serial.println("pH DOWN Pump: OFF");
  } else if (ph_act < 5.75) {
    digitalWrite(PH_UP_PUMP_PIN, HIGH);
    Serial.println("Water Pump: OFF");
    Serial.println("Solution A Pump: OFF");
    Serial.println("Solution B Pump: OFF");
    Serial.println("pH UP Pump: ON");
    Serial.println("pH DOWN Pump: OFF");
  } else if (ph_act > 6.75) {
    digitalWrite(PH_DOWN_PUMP_PIN, HIGH);
    Serial.println("Water Pump: OFF");
    Serial.println("Solution A Pump: OFF");
    Serial.println("Solution B Pump: OFF");
    Serial.println("pH UP Pump: OFF");
    Serial.println("pH DOWN Pump: ON");
  } else {
    Serial.println("Water Pump: OFF");
    Serial.println("Solution A Pump: OFF");
    Serial.println("Solution B Pump: OFF");
    Serial.println("pH UP Pump: OFF");
    Serial.println("pH DOWN Pump: OFF");
  }

  Serial.println("--------------------------");
  delay(2000); // Wait 2 seconds before next cycle
}
