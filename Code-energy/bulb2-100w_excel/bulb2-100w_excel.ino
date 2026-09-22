#include <ZMPT101B.h>

#define SENSITIVITY 460.0f       // Adjust sensitivity if voltage reading is too low
#define MOVING_AVG_SIZE 10

ZMPT101B voltageSensor(32, 50.0);
float voltageReadings[MOVING_AVG_SIZE];
int voltageIndex = 0;

const int sensorPin = 33;
int mVperAmp = 229;
double currentReadings[MOVING_AVG_SIZE];
int currentIndex = 0;
double totalCurrent = 0;
double averageCurrent = 0;
double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;
double Power = 0;
double Energy = 0;
double Cost = 0;
unsigned long previousMillis = 0;
unsigned long interval = 60000;
unsigned long programStartTime = millis();

double costPerUnit = 4.80;

// Starting date and time (year, month, day, hour, minute, second)
int year = 2024;
int month = 11;
int day = 16;
int hour = 12;
int minute = 0;
int second = 0;

void setup() {
  Serial.begin(115200);
  voltageSensor.setSensitivity(SENSITIVITY);

  Serial.println("Date,Time,Voltage (V),Current (Amps),Power (Watts),Energy (Wh),Cost (Rs)");
}

void loop() {
  // Update date and time based on elapsed time
  updateDateTime();

  // Measure voltage
  float voltage = voltageSensor.getRmsVoltage();
  voltageReadings[voltageIndex] = voltage;
  voltageIndex = (voltageIndex + 1) % MOVING_AVG_SIZE;

  float averageVoltage = 0;
  for (int i = 0; i < MOVING_AVG_SIZE; i++) {
    averageVoltage += voltageReadings[i];
  }
  averageVoltage /= MOVING_AVG_SIZE;

  Voltage = getVPP();
  VRMS = (Voltage / 2.0) * 0.707;
  AmpsRMS = (VRMS * 1000) / mVperAmp;

  totalCurrent -= currentReadings[currentIndex];
  currentReadings[currentIndex] = AmpsRMS;
  totalCurrent += currentReadings[currentIndex];
  currentIndex = (currentIndex + 1) % MOVING_AVG_SIZE;

  averageCurrent = totalCurrent / MOVING_AVG_SIZE;
  Power = averageVoltage * averageCurrent;

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    Energy += Power * (interval / 3600000.0);
    double energyInKWh = Energy / 1000.0;
    Cost = energyInKWh * costPerUnit;
  }

  // Print results with separated columns
  Serial.print(year);
  Serial.print("-");
  Serial.print(month);
  Serial.print("-");
  Serial.print(day);
  Serial.print(",");

  Serial.print(hour);
  Serial.print(":");
  Serial.print(minute);
  Serial.print(":");
  Serial.print(second);
  Serial.print(",");

  Serial.print(averageVoltage);
  Serial.print(",");

  Serial.print(averageCurrent);
  Serial.print(",");

  Serial.print(Power);
  Serial.print(",");

  Serial.print(Energy);
  Serial.print(",");

  Serial.println(Cost);

  delay(1000);
}

// Function to update date and time based on millis()
void updateDateTime() {
  unsigned long elapsedMillis = millis() - programStartTime;
  second = elapsedMillis / 1000;

  if (second >= 60) {
    minute += second / 60;
    second %= 60;
  }

  if (minute >= 60) {
    hour += minute / 60;
    minute %= 60;
  }

  if (hour >= 24) {
    day += hour / 24;
    hour %= 24;
  }

  // Adjust days for the month
  int daysInMonth = getDaysInMonth(year, month);
  if (day > daysInMonth) {
    day -= daysInMonth;
    month++;

    if (month > 12) {
      month = 1;
      year++;
    }
  }
}

// Function to return the number of days in a month
int getDaysInMonth(int year, int month) {
  if (month == 2) {
    // Check for leap year
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
      return 29;
    } else {
      return 28;
    }
  } else if (month == 4 || month == 6 || month == 9 || month == 11) {
    return 30;
  } else {
    return 31;
  }
}

// Function to calculate peak-to-peak voltage
float getVPP() {
  float result;
  int readValue;
  int maxValue = 0;
  int minValue = 4096;

  uint32_t start_time = millis();
  while ((millis() - start_time) < 2000) {
    readValue = analogRead(sensorPin);
    if (readValue > maxValue) maxValue = readValue;
    if (readValue < minValue) minValue = readValue;
  }

  result = ((maxValue - minValue) * 3.3) / 4096.0;
  return result;
}
