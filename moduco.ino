#include "Adafruit_PM25AQI.h"
#include <SoftwareSerial.h>
#include <Arduino.h>
#include <Adafruit_AHTX0.h>
#include <WiFi.h>
#include "ThingSpeak.h"

#define SECRET_SSID "Redmi Note 10"
#define SECRET_PASS "panpinpun"
#define SECRET_CH_ID 2344504
#define SECRET_WRITE_APIKEY "L3HWW8PR30ME0AG2"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int keyIndex = 0;
WiFiClient client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char* myWriteAPIKey = SECRET_WRITE_APIKEY;
String myStatus = "";

#define PM25PIN 34
#define MQ135PIN 32

SoftwareSerial pmSerial(16, 17); // Software Serial for PM2.5 sensor (change pins as needed)
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
Adafruit_AHTX0 aht;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  Serial.println("Air Quality Monitoring");

  delay(1000);

  pmSerial.begin(9600);

  if (!aqi.begin_UART(&pmSerial)) {
    Serial.println("Could not find PM2.5 sensor!");
    while (1) delay(10);
  }

  Serial.println("PM2.5 sensor found!");

  if (!aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }
  Serial.println("AHT10 or AHT20 found");

  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);

  Serial.begin(115200);
  while (!Serial)
    delay(10);
  

  delay(5000);
  
}

void loop() {
  PM25_AQI_Data pmData;
  sensors_event_t humidity, temp;

  // Read PM2.5 data
  if (!aqi.read(&pmData)) {
    Serial.println("Could not read from PM2.5 sensor");
  } else {
    Serial.println("PM2.5 sensor reading success");
    Serial.println("PM2.5 Data:");
    Serial.print("PM 1.0: "); Serial.print(pmData.pm10_standard);
    Serial.print("\tPM 2.5: "); Serial.print(pmData.pm25_standard);
    Serial.print("\tPM 10: "); Serial.println(pmData.pm100_standard);
  }

  // Read my mq 135
  
  // Read CO sensor data
  int coSensorValue = analogRead(MQ135PIN);
  float coVoltage = coSensorValue * (5.0 / 4096.0);
  // You need to adjust the conversion equation based on your CO sensor's datasheet
  // The following is just an example
  float coPPM = 10 * ((coVoltage - 0.22) / 0.8);
  Serial.print("CO Concentration: ");
  Serial.print(coPPM);
  Serial.println(" ppm");

  int CO2Value = analogRead(MQ135PIN);
  Serial.print("CO2 Value: ");
  Serial.print(CO2Value);
  Serial.println(" ppm");  // Parts per million

  delay(5000);

  // Read AHT10/AHT20 data
  aht.getEvent(&humidity, &temp);
  Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
  Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");


  // WiFi and ThingSpeak
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected.");
  }

  // Set ThingSpeak fields
  ThingSpeak.setField(1, pmData.pm25_standard);
  ThingSpeak.setField(2, coPPM);
  ThingSpeak.setField(3, CO2Value);
  ThingSpeak.setField(4, temp.temperature);
  ThingSpeak.setField(5, humidity.relative_humidity);


  // Set the status
  ThingSpeak.setStatus(myStatus);

  // Write to ThingSpeak
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (x == 200) {
    Serial.println("Channel update successful.");
  } else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }

  delay(15000);
}
