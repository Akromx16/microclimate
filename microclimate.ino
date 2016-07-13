#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <Adafruit_BMP085_U.h>
#include <DHT.h>
#include <DHT_U.h>
#include <CurieBLE.h>

BLEPeripheral blePeripheral;
BLEService climateService("181A");
BLEFloatCharacteristic Altitude("2AB3", BLERead | BLENotify);
BLEFloatCharacteristic Humidity("2A6F", BLERead | BLENotify);
BLEFloatCharacteristic Light("2A76", BLERead | BLENotify);
BLEFloatCharacteristic Pressure("2A6D", BLERead | BLENotify);
BLEFloatCharacteristic Temperature("2A6E", BLERead | BLENotify);

Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
DHT_Unified dht(2, DHT22);

float altitude = 0;
float humidity = 0;
float light = 0;
float pressure = 0;
float temperature = 0;

String Output = "";
uint32_t delayMS;
int n = 0;

void setup() {
  Serial.begin(57600);
  Serial1.begin(57600);
  Output.reserve(200);
  
  if (!tsl.begin())
  {
    Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
    while (1);
  } else {
    tsl.enableAutoRange(true);
    tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);
  }

  if (!bmp.begin())
  {
    Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");    while (1);
  }

  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);
  delayMS = sensor.min_delay / 1000;

  if (delayMS < 2000)
    delayMS = 2000;
  
  blePeripheral.setLocalName("ClimateMonitor");
  blePeripheral.setAdvertisedServiceUuid(climateService.uuid());  // add the service UUID
  blePeripheral.addAttribute(Altitude);
  blePeripheral.addAttribute(Humidity);
  blePeripheral.addAttribute(Light);
  blePeripheral.addAttribute(Pressure);
  blePeripheral.addAttribute(Temperature);
  
  Altitude.setValue(0);
  Humidity.setValue(0);
  Light.setValue(0);
  Pressure.setValue(0);
  Temperature.setValue(0);
  blePeripheral.begin();
}

void loop() {
  blePeripheral.poll();

  n++;
  sensors_event_t event;

  tsl.getEvent(&event);
  if (event.light) {
    light += event.light;
  } else {
    Serial.println("Error reading light!");
  }

  bmp.getEvent(&event);
  if (event.pressure)
  {
    pressure += event.pressure;
    altitude += bmp.pressureToAltitude(SENSORS_PRESSURE_SEALEVELHPA,
                                       event.pressure);
  } else {
    Serial.println("Error reading pressure!");
    Serial.println("Error reading altitude!");
  }

  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println("Error reading temperature!");
  } else {
    temperature += event.temperature;
  }

  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println("Error reading humidity!");
  } else {
    humidity += event.relative_humidity;
  }
  
  Output = "";
  Output += "{";
  Output += String('"') + "Altitude" + String('"') + ": " + String('"') + String(altitude / n) + String('"') + ", ";
  Output += String('"') + "Humidity" + String('"') + ": " + String('"') + String(humidity / n) + String('"') + ", ";
  Output += String('"') + "Light" + String('"') + ": " + String('"') + String(light / n) + String('"') + ", ";
  Output += String('"') + "Pressure" + String('"') + ": " + String('"') + String(pressure / n) + String('"') + ", ";
  Output += String('"') + "Temperature" + String('"') + ": " + String('"') + String(temperature / n) + String('"');
  Output += "}\n";

  Serial.print(Output);

  if (n >= 5)
  {
    Serial.println("Broadcasting");
    Altitude.setValue(altitude / n);
    Humidity.setValue(humidity / n);
    Light.setValue(light / n);
    Pressure.setValue(pressure / n);
    Temperature.setValue(temperature / n);
    Serial1.print(Output);
    altitude = 0;
    humidity = 0;
    light = 0;
    pressure = 0;
    temperature = 0;
    n = 0;
  }

  delay(delayMS);
}
