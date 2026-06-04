#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <DHT.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// MH-Z19B
SoftwareSerial co2Serial(2, 3); // RX, TX
byte cmd[] = {0xFF, 0x01, 0x86, 0, 0, 0, 0, 0, 0x79};

// DHT11
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Buzzer
int buzzer = 10;
int co2Limit = 1500;

void setup() {
  Serial.begin(9600);
  co2Serial.begin(9600);
  dht.begin();

  pinMode(buzzer, OUTPUT);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println("CO2 Detector");
  display.display();

  delay(2000);
}

void loop() {
  int co2 = readCO2();
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  Serial.print("CO2: ");
  Serial.print(co2);
  Serial.print(" ppm | Temp: ");
  Serial.print(temp);
  Serial.print(" C | Hum: ");
  Serial.print(hum);
  Serial.println(" %");

  if (co2 > co2Limit) {
    digitalWrite(buzzer, HIGH);
  } else {
    digitalWrite(buzzer, LOW);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);

  display.print("CO2: ");
  if (co2 == -1) {
    display.println("Error");
  } else {
    display.print(co2);
    display.println(" ppm");
  }

  display.setCursor(0, 22);
  display.print("Temp: ");
  if (isnan(temp)) {
    display.println("Error");
  } else {
    display.print(temp);
    display.println(" C");
  }

  display.setCursor(0, 44);
  display.print("Hum: ");
  if (isnan(hum)) {
    display.println("Error");
  } else {
    display.print(hum);
    display.println(" %");
  }

  display.display();

  delay(3000);
}

int readCO2() {
  byte response[9];

  while (co2Serial.available()) {
    co2Serial.read();
  }

  co2Serial.write(cmd, 9);
  delay(300);

  if (co2Serial.available() >= 9) {
    for (int i = 0; i < 9; i++) {
      response[i] = co2Serial.read();
    }

    if (response[0] == 0xFF && response[1] == 0x86) {
      int ppm = response[2] * 256 + response[3];
      return ppm;
    }
  }

  return -1;
}