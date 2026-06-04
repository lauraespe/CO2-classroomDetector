// -----------------------------------------------------
// LIBRARIES
// -----------------------------------------------------
// Libraries are pre-written code that allow us to use specific components more easily.

#include <Wire.h>               // Communication library for I2C devices
#include <Adafruit_GFX.h>       // Graphics library for displays
#include <Adafruit_SSD1306.h>   // OLED display library
#include <SoftwareSerial.h>     // Creates additional serial communication pins
#include <DHT.h>                // DHT11 temperature and humidity sensor library

// -----------------------------------------------------
// OLED SCREEN SETTINGS
// -----------------------------------------------------

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Create the OLED display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// -----------------------------------------------------
// MH-Z19B CO₂ SENSOR SETTINGS
// -----------------------------------------------------

// Create a software serial connection.
// First number = RX pin
// Second number = TX pin
SoftwareSerial co2Serial(2, 3); // RX, TX

// Command used to request a CO₂ measurement from the sensor
byte cmd[] = {0xFF, 0x01, 0x86, 0, 0, 0, 0, 0, 0x79};

// -----------------------------------------------------
// DHT11 TEMPERATURE & HUMIDITY SENSOR SETTINGS
// -----------------------------------------------------

#define DHTPIN 4 // Data pin connected to the DHT11
#define DHTTYPE DHT11 // Sensor type

// Create the DHT sensor object
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// -----------------------------------------------------
// BUZZER SETTINGS
// -----------------------------------------------------
int buzzer = 10; // Pin connected to the buzzer

// CO₂ threshold that will trigger the alarm
int co2Limit = 1200;

void setup() {
  Serial.begin(9600);      // Start communication with the computer
  co2Serial.begin(9600);   // Start communication with the CO₂ sensor
  dht.begin();             // Start the DHT11 sensor

  pinMode(buzzer, OUTPUT);  // Set the buzzer pin as an output

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);   // Start the OLED display
  display.clearDisplay();                      // Clear anything previously shown on the display
  display.setTextColor(WHITE);                 // Set text colour
  display.setTextSize(1);                      // Set text size
  display.setCursor(0, 20);                    // Position the cursor
  display.println("CO2 Detector");             // Show a welcome message
  display.display();                           // Update the display

  delay(2000);     // Wait 2 seconds before starting                 
}

void loop() {
  int co2 = readCO2();                   // Read the current CO₂ concentration
  float temp = dht.readTemperature();    // Read temperature from the DHT11
  float hum = dht.readHumidity();        // Read humidity from the DHT11

  // ---------------------------------------------------
  // PRINT DATA TO THE SERIAL MONITOR
  // ---------------------------------------------------
  Serial.print("CO2: ");
  Serial.print(co2);
  Serial.print(" ppm | Temp: ");
  Serial.print(temp);
  Serial.print(" C | Hum: ");
  Serial.print(hum);
  Serial.println(" %");

  // If CO₂ is higher than the limit, activate the buzzer.
  if (co2 > co2Limit) {
    digitalWrite(buzzer, HIGH);
  } else {
    digitalWrite(buzzer, LOW);
  }

  // ---------------------------------------------------
  // OLED DISPLAY
  // ---------------------------------------------------
  
  // Clear the screen before writing new values
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);

  // ---------------- CO₂ ----------------
  display.print("CO2: ");
  if (co2 == -1) {
    display.println("Error");
  } else {
    display.print(co2);
    display.println(" ppm");
  }

  // ---------------- TEMPERATURE ----------------
  display.setCursor(0, 22);
  display.print("Temp: ");
  if (isnan(temp)) {
    display.println("Error");
  } else {
    display.print(temp);
    display.println(" C");
  }

  // ---------------- HUMIDITY ----------------
  display.setCursor(0, 44);
  display.print("Hum: ");
  if (isnan(hum)) {
    display.println("Error");
  } else {
    display.print(hum);
    display.println(" %");
  }

  // Send everything to the OLED screen
  display.display();

  delay(3000); // Wait 3 seconds before taking a new measurement
}

// =====================================================
// FUNCTION: READ CO₂
// =====================================================

// Requests a measurement from the MH-Z19B sensor and returns the value in ppm.
// If the reading fails, returns -1.
int readCO2() {
  byte response[9];        // Array that will store the sensor response

  // Remove any old data waiting in the serial buffer
  while (co2Serial.available()) {
    co2Serial.read();
  }

  // Send the measurement request command
  co2Serial.write(cmd, 9);
  delay(300);

  // Check if we received the expected 9 bytes
  if (co2Serial.available() >= 9) {
    for (int i = 0; i < 9; i++) {
      response[i] = co2Serial.read();
    }

    // Verify that the response is valid
    if (response[0] == 0xFF && response[1] == 0x86) {
      int ppm = response[2] * 256 + response[3];   // Calculate the CO₂ concentration
      return ppm;
    }
  }

  return -1; // Return -1 if something went wrong

}
