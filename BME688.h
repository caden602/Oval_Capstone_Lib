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
}bme_data_t;                    // total   -> 16 bytes

/*
void bme_setup(Adafruit_BME680 *bme, uint8_t pcb);

bool bme_get_data(Adafruit_BME680 *bme, bme_data_t *bme_data);
*/