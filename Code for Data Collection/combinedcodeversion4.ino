#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "max6675.h"
#include <WiFi.h>
#include <ThingSpeak.h>
#include <LiquidCrystal_I2C.h>

const char *ssid = "Abhinav";
const char *password = "Abhinav1234";

// Your ThingSpeak channel ID and API key
unsigned long channelId = 2510242;
const char *apiKey = "L2QK02Z2RFYW82YP";

WiFiClient client; // Initialize WiFi client

// Define LCD pins and dimensions
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

const int analogInterruptPin = 32;
volatile long pulse = 0;
volatile long pulse_for_volume = 0;
float volume = 0;
unsigned long previousMillis = 0;
float flowRate = 0;

// Define the pins for the MAX6675 thermocouple sensors
#define MAX6675_CS_1 25
#define MAX6675_SO_1 33
#define MAX6675_SCK_1 26

#define MAX6675_CS_2 27
#define MAX6675_SO_2 14
#define MAX6675_SCK_2 12

#define MAX6675_CS_3 5
#define MAX6675_SO_3 23
#define MAX6675_SCK_3 19

MAX6675 thermocouple_1(MAX6675_SCK_1, MAX6675_CS_1, MAX6675_SO_1);
MAX6675 thermocouple_2(MAX6675_SCK_2, MAX6675_CS_2, MAX6675_SO_2);
MAX6675 thermocouple_3(MAX6675_SCK_3, MAX6675_CS_3, MAX6675_SO_3);

void analogInterruptHandler()
{
  // Increment the pulse count
  pulse++;
  pulse_for_volume++;
}

void setup()
{
  Serial.begin(115200);
  Wire.begin();

  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");

  // Initialize ThingSpeak client
  ThingSpeak.begin(client);

  // Initialize LCD display
  lcd.init();
  lcd.backlight();

  pinMode(analogInterruptPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(analogInterruptPin), analogInterruptHandler, RISING);
}

void loop()
{
  unsigned long currentMillis = millis();
  unsigned long elapsedTime = currentMillis - previousMillis;
  volume = pulse_for_volume * 2.690 / 1000;
  // Update flow rate every second
  if (elapsedTime >= 1000)
  {
    
    // Calculate flow rate in ml/second
    flowRate = pulse * 6.90 / elapsedTime;
    // Reset pulse and timer
    pulse = 0;
    previousMillis = currentMillis;

    // Output flow rate
    Serial.print("Flow rate: ");
    Serial.print(flowRate);
    Serial.println(" ml/second");

    // Read temperatures from each MAX6675 and print them
    float temp1 = thermocouple_1.readCelsius();
    float temp2 = thermocouple_2.readCelsius();
    float temp3 = thermocouple_3.readCelsius();

    // Print temperatures
    Serial.print("Thermocouple 1 temperature: ");
    Serial.print(temp1);
    Serial.println(" °C");

    Serial.print("Thermocouple 2 temperature: ");
    Serial.print(temp2);
    Serial.println(" °C");

    Serial.print("Thermocouple 3 temperature: ");
    Serial.print(temp3);
    Serial.println(" °C");

    // Display data on LCD
    lcd.clear();
    lcd.setCursor(0, 0); // Set cursor to first row
    lcd.print("Fuel Rate: ");
    lcd.print(flowRate);
    lcd.print(" ml/sec");

    lcd.setCursor(0, 1); // Set cursor to second row

    // Check conditions for warnings
    if (volume > 400)
    {
      lcd.print("Warning! Fuel LOW");
      lcd.setCursor(0, 1); // Move cursor to next row
      lcd.print("(Refill Tank)");
    }
    else if (temp1 > 40)
    {
      lcd.print("Warning! Rad Temp High");
    }
    else
    {
      lcd.print("Volume: ");
      lcd.print(volume);
      lcd.print(" L");
    }

    // Upload data to ThingSpeak
    ThingSpeak.setField(1, temp1);      // Field 1: Radiator Temperature
    ThingSpeak.setField(2, temp2);      // Field 2: Engine Temperature
    ThingSpeak.setField(3, temp3);      // Field 3: Average Body Temperature
    ThingSpeak.setField(4, flowRate);   // Field 4: Fuel Rate (ml/sec)
    ThingSpeak.setField(5, volume);     // Field 5: Diesel Consumed (L)

    int status = ThingSpeak.writeFields(channelId, apiKey);

    if (status == 200)
    {
      Serial.println("Data sent to ThingSpeak successfully");
    }
    else
    {
      Serial.println("Problem sending data to ThingSpeak");
    }
  }
}
