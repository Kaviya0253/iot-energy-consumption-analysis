#include <ZMPT101B.h>

#define SENSITIVITY_60W 495.0f       // Adjust sensitivity for 60W bulb
#define SENSITIVITY_100W 315.0f      // Adjust sensitivity for 100W bulb
#define MOVING_AVG_SIZE 10

// 60W Bulb Setup
ZMPT101B voltageSensor60W(34, 50.0);
float voltageReadings60W[MOVING_AVG_SIZE];
int voltageIndex60W = 0;
const int sensorPin60W = 35;
int mVperAmp60W = 315;
double currentReadings60W[MOVING_AVG_SIZE];
int currentIndex60W = 0;
double totalCurrent60W = 0;
double averageCurrent60W = 0;
double Voltage60W = 0;
double VRMS60W = 0;
double AmpsRMS60W = 0;
double Power60W = 0;
double Energy60W = 0;
double Cost60W = 0;

// 100W Bulb Setup
ZMPT101B voltageSensor100W(32, 50.0);
float voltageReadings100W[MOVING_AVG_SIZE];
int voltageIndex100W = 0;
const int sensorPin100W = 33;
int mVperAmp100W = 245;
double currentReadings100W[MOVING_AVG_SIZE];
int currentIndex100W = 0;
double totalCurrent100W = 0;
double averageCurrent100W = 0;
double Voltage100W = 0;
double VRMS100W = 0;
double AmpsRMS100W = 0;
double Power100W = 0;
double Energy100W = 0;
double Cost100W = 0;

unsigned long previousMillis = 0;
unsigned long interval = 20000;
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
  voltageSensor60W.setSensitivity(SENSITIVITY_60W);
  voltageSensor100W.setSensitivity(SENSITIVITY_100W);

  Serial.println("Date,Time,Voltage (V),Current (Amps),Power (Watts),Energy (Wh),Cost (Rs),Voltage (100W),Current (100W),Power (100W),Energy (100W),Cost (100W)");
}

void loop() {
  // Update date and time based on elapsed time
  updateDateTime();

  // Measure voltage and current for 60W bulb
  float voltage60W = voltageSensor60W.getRmsVoltage();
  voltageReadings60W[voltageIndex60W] = voltage60W;
  voltageIndex60W = (voltageIndex60W + 1) % MOVING_AVG_SIZE;

  float averageVoltage60W = 0;
  for (int i = 0; i < MOVING_AVG_SIZE; i++) {
    averageVoltage60W += voltageReadings60W[i];
  }
  averageVoltage60W /= MOVING_AVG_SIZE;

  Voltage60W = getVPP(sensorPin60W);
  VRMS60W = (Voltage60W / 2.0) * 0.707;
  AmpsRMS60W = (VRMS60W * 1000) / mVperAmp60W;

  totalCurrent60W -= currentReadings60W[currentIndex60W];
  currentReadings60W[currentIndex60W] = AmpsRMS60W;
  totalCurrent60W += currentReadings60W[currentIndex60W];
  currentIndex60W = (currentIndex60W + 1) % MOVING_AVG_SIZE;

  averageCurrent60W = totalCurrent60W / MOVING_AVG_SIZE;
  Power60W = averageVoltage60W * averageCurrent60W;

  // Measure voltage and current for 100W bulb
  float voltage100W = voltageSensor100W.getRmsVoltage();
  voltageReadings100W[voltageIndex100W] = voltage100W;
  voltageIndex100W = (voltageIndex100W + 1) % MOVING_AVG_SIZE;

  float averageVoltage100W = 0;
  for (int i = 0; i < MOVING_AVG_SIZE; i++) {
    averageVoltage100W += voltageReadings100W[i];
  }
  averageVoltage100W /= MOVING_AVG_SIZE;

  Voltage100W = getVPP(sensorPin100W);
  VRMS100W = (Voltage100W / 2.0) * 0.707;
  AmpsRMS100W = (VRMS100W * 1000) / mVperAmp100W;

  totalCurrent100W -= currentReadings100W[currentIndex100W];
  currentReadings100W[currentIndex100W] = AmpsRMS100W;
  totalCurrent100W += currentReadings100W[currentIndex100W];
  currentIndex100W = (currentIndex100W + 1) % MOVING_AVG_SIZE;

  averageCurrent100W = totalCurrent100W / MOVING_AVG_SIZE;
  Power100W = averageVoltage100W * averageCurrent100W;

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    Energy60W += Power60W * (interval / 3600000.0);
    Energy100W += Power100W * (interval / 3600000.0);

    double energyInKWh60W = Energy60W / 1000.0;
    double energyInKWh100W = Energy100W / 1000.0;

    Cost60W = energyInKWh60W * costPerUnit;
    Cost100W = energyInKWh100W * costPerUnit;
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

  Serial.print(averageVoltage60W);
  Serial.print(",");
  Serial.print(averageCurrent60W);
  Serial.print(",");
  Serial.print(Power60W);
  Serial.print(",");
  Serial.print(Energy60W);
  Serial.print(",");
  Serial.print(Cost60W);

  Serial.print(",");

  Serial.print(averageVoltage100W);
  Serial.print(",");
  Serial.print(averageCurrent100W);
  Serial.print(",");
  Serial.print(Power100W);
  Serial.print(",");
  Serial.print(Energy100W);
  Serial.print(",");
  Serial.println(Cost100W);

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
float getVPP(int sensorPin) {                               
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
