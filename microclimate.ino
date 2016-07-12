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
uint32_t delayMS;

void setup() {
  Serial.begin(9600);
  if(!tsl.begin()) // light sensor setup
  {
    Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
    while(1);
  } else 
  {
    tsl.enableAutoRange(true);
    tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */
  }

  if(!bmp.begin())
  {
    Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }

  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);
  delayMS = sensor.min_delay / 1000;

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
  sensors_event_t event;
  
  tsl.getEvent(&event);
  if (event.light)
  {
    Light.setValue(event.light);
    Serial.print("Light:       "); Serial.print(event.light); Serial.println(" lux");
  } else
  {
    Serial.println("Sensor overload");
  }

  bmp.getEvent(&event);
  
  if (event.pressure)
  {
    Pressure.setValue(event.pressure);
    Serial.print("Pressure:    ");
    Serial.print(event.pressure);
    Serial.println(" hPa");

    float alt = bmp.pressureToAltitude(SENSORS_PRESSURE_SEALEVELHPA,
                                        event.pressure);
    Altitude.setValue(alt);
    Serial.print("Altitude:    "); 
    Serial.print(alt); 
    Serial.println(" m");
    
  }
  else
  {
    Serial.println("Sensor error");
  }

  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println("Error reading temperature!");
  }
  else {
    Temperature.setValue(event.temperature);
    Serial.print("Temperature: ");
    Serial.print(event.temperature);
    Serial.println(" *C");
  }
  
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println("Error reading humidity!");
  }
  else {
    Humidity.setValue(event.relative_humidity);
    Serial.print("Humidity: ");
    Serial.print(event.relative_humidity);
    Serial.println("%");
  }

  Serial.println("");
  
  if (delayMS > 500)
    delay(delayMS);
  else
    delay(500);
}
