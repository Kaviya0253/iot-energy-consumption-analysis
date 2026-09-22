#include <ZMPT101B.h>

#define SENSITIVITY 600.0f       // Adjust sensitivity if voltage reading is too low
#define MOVING_AVG_SIZE 10

ZMPT101B voltageSensor(34, 50.0); // Use GPIO 34 for analog input
float voltageReadings[MOVING_AVG_SIZE];  // Array to store the last 10 voltage readings
int currentIndex = 0;  // Index for storing readings in the array

// ACS712 current sensor
const int sensorPin = 35;     // GPIO pin where ACS712 OUT pin is connected
int mVperAmp = 185;           // Sensitivity for ACS712 (5A version)
double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;
double Power = 0;

void setup() {
  Serial.begin(115200);  // Start serial communication at 115200 baud rate
  voltageSensor.setSensitivity(SENSITIVITY);  // Set the sensitivity for the voltage sensor

  Serial.println("ACS712 Current Sensor");
}

void loop() {
  // Measure voltage
  float voltage = voltageSensor.getRmsVoltage();  // Get RMS voltage from voltage sensor

  // Store the voltage in the readings array
  voltageReadings[currentIndex] = voltage;

  // Move to the next index (circular buffer)
  currentIndex = (currentIndex + 1) % MOVING_AVG_SIZE;

  // Calculate the moving average of the voltage readings
  float averageVoltage = 0;
  for (int i = 0; i < MOVING_AVG_SIZE; i++) {
    averageVoltage += voltageReadings[i];
  }
  averageVoltage /= MOVING_AVG_SIZE;

  // Measure the peak-to-peak voltage from the current sensor
  Voltage = getVPP();

  // Calculate the RMS voltage from peak-to-peak
  VRMS = (Voltage / 2.0) * 0.707;

  // Calculate the RMS current
  AmpsRMS = (VRMS * 1000) / mVperAmp;

  // Calculate the power in Watts (P = V * I)
  Power = averageVoltage * AmpsRMS;

  // Print the results
  Serial.print("Voltage: ");
  Serial.print(voltage);
  Serial.print(" V, Moving Average: ");
  Serial.print(averageVoltage);
  Serial.print(" V, ");

  Serial.print("Current: ");
  Serial.print(AmpsRMS);
  Serial.print(" Amps RMS  ---  ");

  Serial.print("Power: ");
  Serial.print(Power);
  Serial.println(" Watts");

  delay(1000); // 1 second delay between readings
}

// Function to calculate peak-to-peak voltage
float getVPP() {
  float result;
  int readValue;                // Value read from the sensor
  int maxValue = 0;             // Store max value
  int minValue = 4096;          // Store min value (ESP32 ADC resolution)

  uint32_t start_time = millis();
  while ((millis() - start_time) < 1000) { // Sample for 1 second
    readValue = analogRead(sensorPin);
    if (readValue > maxValue) maxValue = readValue;
    if (readValue < minValue) minValue = readValue;
  }
   
  // Calculate peak-to-peak voltage
  result = ((maxValue - minValue) * 3.3) / 4096.0; // Normalize to 3.3V ADC reference
  
  return result;
}
