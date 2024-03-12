/***************************************************************************
  This is a library for the BME680 gas, humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME680 Breakout
  ----> http://www.adafruit.com/products/3660

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)


typedef struct{
    float temperature;          // 32 bits -> 4 bytes
    uint32_t pressure;          // 32 bits -> 4 bytes
    float humidity;             // 32 bits -> 4 bytes
    uint32_t gas_resistance;    // 32 bits -> 4 bytes
    uint8_t error;              // 8 bits  -> 1 byte
}bme_data_t;                    // total   -> 17 bytes


void bme_setup(Adafruit_BME680 *bme) {
  if (!bme->begin()) {
    Serial.println("Failed to find BME688");
    while (1);
  }
  Serial.println("BME68 Found!");

  // Set up oversampling and filter initialization
  bme->setTemperatureOversampling(BME680_OS_8X);
  bme->setHumidityOversampling(BME680_OS_2X);
  bme->setPressureOversampling(BME680_OS_4X);
  bme->setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme->setGasHeater(320, 150); // 320*C for 150 ms
}

void bme_get_data(Adafruit_BME680 *bme, bme_data_t *bme_data) {
    bme_data->error = 0;

    // Perform Reading and check for error
    if (! bme->performReading()) {
        Serial.println("Failed to perform reading on BME688");

        // set the error bit to 1
        bme_data->error = 1;
    }
    bme_data->temperature = bme->temperature;
    bme_data->pressure = bme->pressure / 100.0;
    bme_data->humidity = bme->humidity;
    bme_data->gas_resistance = bme->gas_resistance / 1000.0;

}