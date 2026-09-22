const int sensorPin = 35;     // GPIO pin where ACS712 OUT pin is connected
int mVperAmp = 155;           // Sensitivity for ACS712 (5A version)
double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;

const int numReadings = 20;   // Increase the number of readings for moving average
double readings[numReadings]; // Array to hold readings
int readIndex = 0;            // Index for reading array
double total = 0;             // Running total for moving average
double averageCurrent = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("ACS712 Current Sensor");
}

void loop() {
  // Measure the peak-to-peak voltage from the sensor
  Voltage = getVPP();

  // Calculate the RMS voltage
  VRMS = (Voltage / 2.0) * 0.707;

  // Calculate the RMS current
  AmpsRMS = (VRMS * 1000) / mVperAmp;

  // Subtract the last reading from the total
  total = total - readings[readIndex];

  // Add the new reading to the total
  readings[readIndex] = AmpsRMS;
  total = total + readings[readIndex];

  // Advance to the next index in the array
  readIndex = readIndex + 1;

  // If we reach the end of the array, start over
  if (readIndex >= numReadings) {
    readIndex = 0;
  }

  // Calculate the average
  averageCurrent = total / numReadings;

  // Print the average current
  Serial.print("Current (Average): ");
  Serial.print(averageCurrent);
  Serial.println(" Amps RMS");

  delay(500); // Shorten the delay for faster updates
}

// Function to calculate peak-to-peak voltage
float getVPP() {
  float result;
  int readValue;                // Value read from the sensor
  int maxValue = 0;             // Store max value
  int minValue = 4096;          // Store min value (ESP32 ADC resolution)

  uint32_t start_time = millis();
  while ((millis() - start_time) < 2000) { // Increase sample duration to 2 seconds for smoother results
    readValue = analogRead(sensorPin);
    if (readValue > maxValue) maxValue = readValue;
    if (readValue < minValue) minValue = readValue;
  }

  // Calculate peak-to-peak voltage
  result = ((maxValue - minValue) * 3.3) / 4096.0; // Normalize to 3.3V ADC reference
  
  return result;
}
