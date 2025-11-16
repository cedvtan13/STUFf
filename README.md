# Arduino Weather & Light Data Logger

This project uses an Arduino Mini to build a complete, standalone data logger. It records temperature, humidity, atmospheric pressure, and ambient light levels to a text file on a microSD card, creating a comprehensive dataset for environmental analysis.

## Project Goal
To log and "show the correlation between solar radiation (light) and atmospheric heating (temperature)," while also recording humidity and barometric pressure.

---

## Hardware Required
* **Arduino Mini (5V version)**
* **FTDI USB-to-Serial Adapter** (for programming the Mini)
* **DHT11** (Temperature & Humidity Sensor)
* **BMP180** (Barometric Pressure Sensor)
* **LDR** (Light-Dependent Resistor)
* **10kΩ Resistor** (for the LDR's voltage divider)
* **10kΩ Resistor** (optional, but recommended as a pull-up for the DHT11)
* **MicroSD Card Module**
* **MicroSD Card** (formatted as FAT32)
* **Breadboard** and Jumper Wires
* **5V DC Power Supply** (e.g., USB phone charger)

---

## Wiring Instructions
Ensure the Arduino is unpowered while wiring.



### 1. DHT11 (Temperature/Humidity)
* `VCC` → `5V`
* `GND` → `GND`
* `Data` → `Digital Pin 2`
* **Note:** For 4-pin DHT11 components, connect a **10kΩ resistor** between the `Data` pin and the `5V` pin.

### 2. BMP180 (Pressure Sensor - I2C)
* `VCC` → `5V`
* `GND` → `GND`
* `SCL` → `A5`
* `SDA` → `A4`

### 3. LDR (Light Sensor - Analog)
This creates a voltage divider.
* `LDR Pin 1` → `5V`
* `LDR Pin 2` → `A0`
* `10kΩ Resistor Pin 1` → `A0` (same row as LDR Pin 2)
* `10kΩ Resistor Pin 2` → `GND`

### 4. MicroSD Card Module (SPI)
* `VCC` → `5V`
* `GND` → `GND`
* `CS` → `Digital Pin 4`
* `SCK` → `Digital Pin 13`
* `MISO` → `Digital Pin 12`
* `MOSI` → `Digital Pin 11`

---

## Software & Setup

### 1. Arduino IDE Libraries (DONT FORGET THIS, basig dili mu run inyo code)
Before uploading the code, you must install these libraries via the Arduino IDE (`Tools > Manage Libraries...`):
* **`DHT sensor library`** by Adafruit
* **`Adafruit BMP085 Library`** by Adafruit (This is the correct library for the BMP180)
* **`Adafruit Unified Sensor`** (will be installed as a dependency)

### 2. SD Card (FINISHED)
Your microSD card **must be formatted as FAT32** (or FAT16).

---

## How to Use
1.  **Upload Code:** Copy the code below into the Arduino IDE. Connect your Arduino Mini via the FTDI adapter and upload the sketch.
2.  **Deploy:** Disconnect the Arduino from your computer. Insert the formatted SD card.
3.  **Power On:** Connect the Arduino to your standalone 5V DC power supply.
4.  **Check Operation:** The onboard LED (Pin 13) will give a short blink every 3 seconds. This is the "heartbeat" confirming that a new line of data has been successfully saved to the SD card.
5.  **Retrieve Data:** After you are done collecting data, power down the Arduino, remove the SD card, and put it in your computer. You will find a file named `datalog.txt` that you can open with any spreadsheet program (like Excel or Google Sheets).

---

## Complete Arduino Code (datalogger.ino)
This is the final, stable code. It is optimized to prevent RAM overflow on the Arduino Mini and includes the "heartbeat" LED.

```cpp
#include <SPI.h>
#include <SD.h>
#include <DHT.h>
#include <Wire.h> //I2C comms
#include <Adafruit_BMP085.h>  //barometer lib

#define BUILTIN_LED 13
#define DHTPIN 2  //dht11 --> DP2
#define SD_CS_PIN 4 //sd card module chip select --> DP4
#define LDR_PIN A0
#define DHTTYPE DHT11 //dht11 sensor
DHT dht(DHTPIN, DHTTYPE);

Adafruit_BMP085 bmp;

File dataFile;
char logFileName[] = "datalog.txt";


void setup() {
  delay(1000);
  // put your setup code here, to run once:
  Serial.begin(9600);

  //verification for data logging
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);
  // while (!Serial){
  //   ; //waiting for serial port connection
  // }

  Serial.println("Starting data logger...");
  dht.begin();

  //bmp 180 initialization
  Serial.print("Initializing BMP180...");
  if (!bmp.begin()){
    Serial.println("ERROR: Could not find a valid BMP180 sensor!!!"); //error handling
    while(1);
  }
  Serial.println("BMP180 initialized.");

  //sd-card initailization
  Serial.println("Initializating SD-Card...");
  if (!SD.begin(SD_CS_PIN)){
    Serial.println("ERROR: SD-Card initalization failed!!!");
    while(1);
  }
  Serial.println("Card Initialized.");
  dataFile = SD.open(logFileName, FILE_WRITE);

  //datawrite to txt file
  if (dataFile){
    dataFile.println("Timestamp_ms, Temperature_C, Humidity_%, Presusre_hPa, Light_Level");
    dataFile.close();
    Serial.println("Log file header written.");
  }
  else{
    Serial.print("Error opening ");
    Serial.println(logFileName);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

  //timestamp
  unsigned long timestamp = millis();

  //dht11 data
  float humidity = dht.readHumidity();
  float temp = dht.readTemperature(); //Deg_C

  //bmp180 data
  float pressure = bmp.readPressure()/100.0F; ///100F, conversion from Pa to hPa

  //ldr data
  int lightLevel = analogRead(LDR_PIN);

  //error handling for failed reads
  if (isnan(humidity)||isnan(temp)){
    Serial.println("ERROR: Filed to read from DHt sensor!!!");
    return;
  }
  if(isnan(pressure)){
    Serial.println("ERROR: Failed to read from BMP sensor!!");
    return;
  }

  //print data to serial monitor (debug)
  Serial.println("");
  Serial.print("Timestamp: ");      Serial.print(timestamp);
  Serial.print(", Temp: ");         Serial.print(temp);
  Serial.print("degC, Humidity: "); Serial.print(humidity);
  Serial.print("%, Pressure: ");    Serial.print(pressure);
  Serial.print(" hPa, Light: ");    Serial.println(lightLevel);

  //saving of data to SD Card
  dataFile = SD.open(logFileName, O_RDWR|O_CREAT|O_APPEND);

  if (dataFile){
    dataFile.print(timestamp);
    dataFile.print(",");
    dataFile.print(temp);
    dataFile.print(",");
    dataFile.print(humidity);
    dataFile.print(",");
    dataFile.print(pressure);
    dataFile.print(",");
    dataFile.println(lightLevel);
    dataFile.close();

    //rx blinks indicating it logged data
    digitalWrite(BUILTIN_LED, HIGH);
    delay(100);
    digitalWrite(BUILTIN_LED, LOW);

  }
  else{
    Serial.print("Error opening ");
    Serial.print(logFileName);
    Serial.println(" for writing.");
  }

  delay(2900);  //delay for wr

}

