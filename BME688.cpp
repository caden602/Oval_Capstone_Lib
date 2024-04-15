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

unsigned long last_bme_sample;

void bme_setup(Adafruit_BME680 *bme, uint8_t pcb) {
  //*
  if(pcb){
    if (!bme->begin(0x76)) {
      Serial.println("Failed to find BME688 on pcb");
      while (1);
    }
  }
  else{
    if (!bme->begin(0x77)) {
      Serial.println("Failed to find BME688");
      while (1);
    }
  }
  //*/

  Serial.println("BME688 Found!");

  // Set up oversampling and filter initialization
  bme->setTemperatureOversampling(BME680_OS_8X);
  bme->setHumidityOversampling(BME680_OS_2X);
  bme->setPressureOversampling(BME680_OS_4X);
  bme->setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme->setGasHeater(320, 150); // 320*C for 150 ms

  delay(500);

  last_bme_sample = 0;
}

void bme_get_data(Adafruit_BME680 *bme) {
  // Perform Reading and check for error

  bme_data_t bme_data;

  if(millis() - last_bme_sample >= 1000){
    if (bme->performReading()) {
      
      // set bme data
      bme_data.temperature = bme->temperature;
      bme_data.pressure = bme->pressure / 100.0;
      bme_data.humidity = bme->humidity;
      bme_data.gas_resistance = bme->gas_resistance / 1000.0;

      // Get time for last sample
      last_bme_sample = millis();

      // store in eeprom
      
    }
  }
}