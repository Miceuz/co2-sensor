/*************************************************** 
  This is an example for the SHT31-D Humidity & Temp Sensor

  Designed specifically to work with the SHT31-D sensor from Adafruit
  ----> https://www.adafruit.com/products/2857

  These sensors use I2C to communicate, 2 pins are required to  
  interface
 ****************************************************/
 
#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SHT31.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <InfluxDb.h>
#include "MHZ19.h"                                        
#include <SoftwareSerial.h>                                // Remove if using HardwareSerial or Arduino package without SoftwareSerial support

Adafruit_SHT31 sht31 = Adafruit_SHT31();

const char* ssid     = "TP-LINK_AD8D1A";
const char* password = "superi8k6t";

#define INFLUXDB_HOST "192.168.0.101"
Influxdb influx(INFLUXDB_HOST);

#define RX_PIN D7                                          // Rx pin which the MHZ19 Tx pin is attached to
#define TX_PIN D6                                          // Tx pin which the MHZ19 Rx pin is attached to
#define BAUDRATE 9600                                      // Device to MH-Z19 Serial baudrate (should not be changed)

SoftwareSerial mySerial(RX_PIN, TX_PIN);                   // (Uno example) create device to MH-Z19 serial
MHZ19 myMHZ19;                                             // Constructor for library

void setup() {
  Serial.begin(9600);

  while (!Serial)
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("SHT31 test");
  if (! sht31.begin(0x45)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  influx.setDb("environment");
  mySerial.begin(BAUDRATE);                               // (Uno example) device to MH-Z19 serial start   
  myMHZ19.begin(mySerial);                                // *Serial(Stream) refence must be passed to library begin(). 
  myMHZ19.autoCalibration();                              // Turn auto calibration ON (OFF autoCalibration(false))

}


void loop() {

  int CO2; 
  CO2 = myMHZ19.getCO2();                             // Request CO2 (as ppm)
  
  Serial.print("CO2 (ppm): ");                      
  Serial.println(CO2);                                

  int8_t Temp;
  Temp = myMHZ19.getTemperature();                     // Request Temperature (as Celsius)
  Serial.print("Temperature (C): ");                  
  Serial.println(Temp);                               

  float t = sht31.readTemperature();
  float h = sht31.readHumidity();

  if (! isnan(t)) {  // check if 'is not a number'
    Serial.print("Temp *C = "); Serial.println(t);
  } else { 
    Serial.println("Failed to read temperature");
    return;
  }
  
  if (! isnan(h)) {  // check if 'is not a number'
    Serial.print("Hum. % = "); Serial.println(h);
  } else { 
    Serial.println("Failed to read humidity");
    return;
  }

  

  Serial.println();

  InfluxData row("hum_temp");
  row.addTag("sensor", "sht30");
  row.addValue("temperature", t);
  row.addValue("humidity", h);
  influx.write(row);

  InfluxData row2("co2_temp");
  row2.addTag("sensor", "MHZ19");
  row2.addValue("temperature", Temp);
  row2.addValue("co2", CO2);
  influx.write(row2);

  delay(5000);
}
