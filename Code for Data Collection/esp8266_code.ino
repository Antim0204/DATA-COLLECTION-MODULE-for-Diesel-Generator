
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>

const uint8_t sda = D2;
const uint8_t scl = D1;


// Replace with your Wi-Fi network credentials
const char* ssid = "Abhinav";
const char* password = "Abhinav1234";

// Replace with your ThingSpeak channel and write API key
unsigned long channelID = 2510242;
const char* writeAPIKey = "L2QK02Z2RFYW82YP";

// Initialize the MPU6050 sensor
Adafruit_MPU6050 mpu;

// Create a WiFiClient instance to connect to ThingSpeak
WiFiClient client;

void setup() {
  Serial.begin(115200);
  delay(10);
  Wire.begin(sda, scl);
  // Connect to Wi-Fi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Initialize the MPU6050 sensor
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  // Set up ThingSpeak
  ThingSpeak.begin(client);
}

void loop() {
  // Get acceleration data from the MPU6050 sensor
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Upload acceleration data to ThingSpeak
  ThingSpeak.setField(6, a.acceleration.x);
  ThingSpeak.setField(7, a.acceleration.y);
  ThingSpeak.setField(8, a.acceleration.z);
  Serial.println("Data Sent");
  ThingSpeak.writeFields(channelID, writeAPIKey);
  Serial.println(a.acceleration.x);
  Serial.println(a.acceleration.y);
  Serial.println(a.acceleration.z);

  // Wait for 15 seconds before uploading the next data point
  delay(1000);
}