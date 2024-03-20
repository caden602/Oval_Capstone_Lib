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

#include <BME688.h>

void bme_setup(Adafruit_BME680 *bme) {
  if (!bme->begin()) {
    Serial.println("Failed to find BME688");
    while (1);
  }
  Serial.println("BME688 Found!");

  // Set up oversampling and filter initialization
  bme->setTemperatureOversampling(BME680_OS_8X);
  bme->setHumidityOversampling(BME680_OS_2X);
  bme->setPressureOversampling(BME680_OS_4X);
  bme->setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme->setGasHeater(320, 150); // 320*C for 150 ms
}

bool bme_get_data(Adafruit_BME680 *bme, bme_data_t *bme_data) {
    // Perform Reading and check for error
    if (! bme->performReading()) {
        return false;
    }
    bme_data->temperature = bme->temperature;
    bme_data->pressure = bme->pressure / 100.0;
    bme_data->humidity = bme->humidity;
    bme_data->gas_resistance = bme->gas_resistance / 1000.0;
    
    return true;
}