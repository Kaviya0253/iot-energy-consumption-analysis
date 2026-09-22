#include <ZMPT101B.h>

#define SENSITIVITY 600.0f       // Adjust sensitivity if voltage reading is too low
#define MOVING_AVG_SIZE 10

ZMPT101B voltageSensor(34, 50.0); // Use GPIO 34 for analog input
float voltageReadings[MOVING_AVG_SIZE];  // Array to store the last 10 voltage readings
int currentIndex = 0;  // Index for storing readings in the array

void setup() {
  Serial.begin(115200);  // Start serial communication at 115200 baud rate
  voltageSensor.setSensitivity(SENSITIVITY);  // Set the sensitivity for the sensor
}

void loop() {
  float voltage = voltageSensor.getRmsVoltage();  // Get RMS voltage from sensor

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

  // Print the measured voltage and the moving average
  Serial.print("Voltage: ");
  Serial.print(voltage);
  Serial.print(" V, Moving Average: ");
  Serial.print(averageVoltage);
  Serial.println(" V");

  delay(1000);  // Wait for 1 second before the next reading
}
