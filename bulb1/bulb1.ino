#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFi.h>
#include <ZMPT101B.h>
#include <WiFiClient.h>
#include <HTTPClient.h>

#define SENSITIVITY 606.0f
#define MOVING_AVG_SIZE 10

ZMPT101B voltageSensor(34, 50.0);
float voltageReadings[MOVING_AVG_SIZE];
int voltageIndex = 0;

const int sensorPin = 35;
int mVperAmp = 140;
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
unsigned long totalUsageTime = 0;

double costPerUnit = 4.80;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000); // 19800 for IST (GMT+5:30)

const char* ssid = "Infinix NOTE 30 5G";
const char* password = "6pecubnv6e4gi2i";
String serverName = "http://192.168.210.111:8080/Energy_Monitor/insert_data.php";
 // Replace with your local IP address

void setup() {
  Serial.begin(115200);
  voltageSensor.setSensitivity(SENSITIVITY);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  
  timeClient.begin();
  Serial.println("ACS712 Current Sensor and ZMPT101B Voltage Sensor");
}

void loop() {
  timeClient.update();
  
  // Convert epoch time to date format
  unsigned long epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime((time_t *)&epochTime);
  
  int year = ptm->tm_year + 1900;
  int month = ptm->tm_mon + 1;
  int day = ptm->tm_mday;
  int hour = ptm->tm_hour;
  int minute = ptm->tm_min;
  int second = ptm->tm_sec;
  
  // Rest of your sensor measurement and calculations
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
  
  totalUsageTime = currentMillis;
  double totalUsageHours = totalUsageTime / 3600000.0;
  
  Serial.print("Voltage: ");
  Serial.print(averageVoltage);
  Serial.print(" V, ");
  
  Serial.print("Current (Average): ");
  Serial.print(averageCurrent);
  Serial.print(" Amps RMS, ");
  
  Serial.print("Power: ");
  Serial.print(Power);
  Serial.print(" Watts, ");
  
  Serial.print("Energy: ");
  Serial.print(Energy);
  Serial.print(" Wh, ");
  
  Serial.print("Cost: Rs ");
  Serial.print(Cost);
  Serial.print(", ");
  
  Serial.print("Total Time Used: ");
  Serial.print(totalUsageHours);
  Serial.println(" hours");

  // Send data to the PHP script using HTTP POST
  sendDataToServer(averageVoltage, averageCurrent, Power, Energy, Cost);
  
  delay(1000);
}

void sendDataToServer(double voltage, double current, double power, double energy, double cost) {
  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);  // Connect to the server

    http.addHeader("Content-Type", "application/x-www-form-urlencoded"); // Specify content type

    // Create the payload to send
    String payload = "voltage=" + String(voltage) + "&current=" + String(current) + "&power=" + String(power) +
                     "&energy=" + String(energy) + "&cost=" + String(cost);
    
    int httpCode = http.POST(payload);  // Use POST method
    
    if (httpCode > 0) {
      Serial.println("Data sent successfully");
    } else {
      Serial.println("Error in sending data: " + String(httpCode));
    }
    
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

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
