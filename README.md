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

### 1. Arduino IDE Libraries
Before uploading the code, you must install these libraries via the Arduino IDE (`Tools > Manage Libraries...`):
* **`DHT sensor library`** by Adafruit
* **`Adafruit BMP085 Library`** by Adafruit (This is the correct library for the BMP180)
* **`Adafruit Unified Sensor`** (will be installed as a dependency)

### 2. SD Card
Your microSD card **must be formatted as FAT32** (or FAT16).

---

## How to Use
1.  **Upload Code:** Copy the code from `datalogger.ino` (below) into the Arduino IDE. Connect your Arduino Mini via the FTDI adapter and upload the sketch.
2.  **Deploy:** Disconnect the Arduino from your computer. Insert the formatted SD card.
3.  **Power On:** Connect the Arduino to your standalone 5V DC power supply.
4.  **Check Operation:** The onboard LED (Pin 13) will give a short blink every 5 seconds. This is the "heartbeat" confirming that a new line of data has been successfully saved to the SD card.
5.  **Retrieve Data:** After you are done collecting data, power down the Arduino, remove the SD card, and put it in your computer. You will find a file named `datalog.txt` that you can open with any spreadsheet program (like Excel or Google Sheets).

---

## Complete Arduino Code (datalogger.ino)
This is the final, stable code. It is optimized to prevent RAM overflow on the Arduino Mini and includes the "heartbeat" LED.

```cpp
/*
 * Arduino Multi-Sensor Data Logger
 * * Logs: Temperature, Humidity, Pressure, and Light Level
 * To:   microSD Card
 * * This code is optimized for stability on an Arduino Mini (ATmega328P)
 * by avoiding String concatenation in the main loop to prevent SRAM fragmentation.
 */

// Include all necessary libraries
#include <SPI.h>
#include <SD.h>
#include <DHT.h>
#include <Wire.h>            // For I2C communication (BMP180)
#include <Adafruit_BMP085.h> // For the BMP180 barometer

// --- Pin Definitions ---
#define DHTPIN 2       // DHT11 Data pin
#define SD_CS_PIN 4    // SD Card Module Chip Select
#define LDR_PIN A0     // LDR is connected to Analog Pin 0
#define BUILTIN_LED 13 // Onboard LED for heartbeat
#define DHTTYPE DHT11  // We are using the DHT11 sensor

// --- Sensor Objects ---
DHT dht(DHTPIN, DHTTYPE); // Initialize the DHT sensor
Adafruit_BMP085 bmp;      // Initialize the BMP180 sensor

// --- Global Variables ---
File dataFile;
// Use a char array instead of a String object to save RAM
char logFileName[] = "datalog.txt";

// =====================================================================
//   SETUP
// =====================================================================
void setup() {
  // Start Serial for debugging (but don't wait for it)
  Serial.begin(9600);

  // Set up the heartbeat LED
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW); // Start with the LED off

  /*
   * NOTE: The line 'while(!Serial)' has been REMOVED.
   * This is critical. It allows the Arduino to boot and run
   * when powered by a DC adapter (not connected to a PC).
   */

  Serial.println("Starting data logger...");

  // Start the sensors
  dht.begin();

  Serial.print("Initializing BMP180...");
  if (!bmp.begin()) {
    Serial.println("ERROR: Could not find a valid BMP180 sensor!!!");
    while (1); // Stop here if the sensor is not found
  }
  Serial.println("BMP180 initialized.");

  // Start the SD card
  Serial.println("Initializating SD-Card...");
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("ERROR: SD-Card initalization failed!!!");
    while (1); // Stop here if the card failed
  }
  Serial.println("Card Initialized.");

  // --- Open the Data File and Write the Header ---
  dataFile = SD.open(logFileName, FILE_WRITE);

  if (dataFile) {
    // Write the CSV header row
    dataFile.println("Timestamp_ms,Temperature_C,Humidity_%,Pressure_hPa,Light_Level");
    dataFile.close(); // Close the file to save
    Serial.println("Log file header written.");
  } else {
    Serial.println("Error opening datalog.txt");
  }
}

// =====================================================================
//   LOOP
// =====================================================================
void loop() {
  // --- 1. Read Data from ALL Sensors ---
  unsigned long timestamp = millis();
  float humidity = dht.readHumidity();
  float temp = dht.readTemperature(); // Read as Celsius
  float pressure = bmp.readPressure() / 100.0F; // Convert Pa to hPa
  int lightLevel = analogRead(LDR_PIN); // Reads 0-1023

  // --- 2. Check for Sensor Read Errors ---
  if (isnan(humidity) || isnan(temp)) {
    Serial.println("ERROR: Failed to read from DHT sensor!!!");
    return; // Skip this loop and try again
  }
  if (isnan(pressure)) {
    Serial.println("ERROR: Failed to read from BMP sensor!!");
    return; // Skip this loop
  }

  // --- 3. Print Data to Serial Monitor (for debugging) ---
  Serial.println("");
  Serial.print("Timestamp: ");       Serial.print(timestamp);
  Serial.print(", Temp: ");          Serial.print(temp);
  Serial.print("degC, Humidity: "); Serial.print(humidity);
  Serial.print("%, Pressure: ");     Serial.print(pressure);
  Serial.print(" hPa, Light: ");     Serial.println(lightLevel);

  // --- 4. Save Data to SD Card (RAM-Safe Method) ---
  // Re-open the file for each write. This is safer.
  dataFile = SD.open(logFileName, FILE_WRITE);

  if (dataFile) {
    // Print each value directly to the file, separated by commas
    dataFile.print(timestamp);
    dataFile.print(",");
    dataFile.print(temp);
    dataFile.print(",");
    dataFile.print(humidity);
    dataFile.print(",");
    dataFile.print(pressure);
    dataFile.print(",");
    dataFile.println(lightLevel); // Use println for the last item

    dataFile.close(); // Close the file to save data

    // --- Blink the heartbeat LED to show a successful write ---
    digitalWrite(BUILTIN_LED, HIGH);
    delay(100); // Keep LED on for a 1/10th of a second
    digitalWrite(BUILTIN_LED, LOW);

  } else {
    Serial.println("Error opening datalog.txt for writing.");
  }

  // Delay for 5 seconds (minus the 100ms blink)
  delay(4900);
}
