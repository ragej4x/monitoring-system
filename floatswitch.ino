const int floatSwitchPin = A3;

void setup() {
  pinMode(floatSwitchPin, INPUT_PULLUP);  // Enable internal pull-up
  Serial.begin(9600);
}

void loop() {
  if (digitalRead(floatSwitchPin) == LOW) {
    Serial.println("Float switch is ON (triggered)");
  } else {
    Serial.println("Float switch is OFF (not triggered)");
  }

  delay(500);
}
