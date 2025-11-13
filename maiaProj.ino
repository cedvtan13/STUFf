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
String logFileName = "datalog.txt";


void setup() {
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
    Serial.println("Error opening " + logFileName);
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
  dataFile = SD.open(logFileName, FILE_WRITE);

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
    Serial.println("Error opening " + logFileName + " for writing.");
  }

  delay(5000);  //delay for wr

}
