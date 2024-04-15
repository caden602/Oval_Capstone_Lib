// Basic demo for magnetometer readings from Adafruit LIS3MDL

#include <LIS3MDL.h>

void lis3mdl_setup(Adafruit_LIS3MDL *lis3mdl) {
  
  // Try to initialize!
  if (! lis3mdl->begin_I2C()) {          // hardware I2C mode, can pass in address & alt Wire
  //if (! lis3mdl.begin_SPI(LIS3MDL_CS)) {  // hardware SPI mode
  //if (! lis3mdl.begin_SPI(LIS3MDL_CS, LIS3MDL_CLK, LIS3MDL_MISO, LIS3MDL_MOSI)) { // soft SPI
    Serial.println("Failed to find LIS3MDL");
    while (1) { delay(10); }
  }
  Serial.println("LIS3MDL Found!");

  lis3mdl->setPerformanceMode(LIS3MDL_MEDIUMMODE);


  lis3mdl->setOperationMode(LIS3MDL_CONTINUOUSMODE);


  lis3mdl->setDataRate(LIS3MDL_DATARATE_155_HZ);

  
  lis3mdl->setRange(LIS3MDL_RANGE_4_GAUSS);
  

  lis3mdl->setIntThreshold(500);
  lis3mdl->configInterrupt(false, false, true, // enable z axis
                          true, // polarity
                          false, // don't latch
                          true); // enabled!

  delay(500);
}

void lis3mdl_set_data_rate(Adafruit_LIS3MDL *lis3mdl, double rate){
  lis3mdl_dataRate_t _rate;
  
  if(rate < 1.25){
    _rate = LIS3MDL_DATARATE_0_625_HZ;
  }
  else if(rate < 2.5){
    _rate = LIS3MDL_DATARATE_1_25_HZ;
  }
  else if(rate < 5){
    _rate = LIS3MDL_DATARATE_2_5_HZ;
  }
  else if(rate < 10){
    _rate = LIS3MDL_DATARATE_5_HZ;
  }
  else if(rate < 20){
    _rate = LIS3MDL_DATARATE_10_HZ;
  }
  else if(rate < 40){
    _rate = LIS3MDL_DATARATE_20_HZ;
  }
  else if(rate < 80){
    _rate = LIS3MDL_DATARATE_40_HZ;
  }
  else if(rate < 155){
    _rate = LIS3MDL_DATARATE_80_HZ;
  }
  else if(rate < 300){
    _rate = LIS3MDL_DATARATE_155_HZ;
  }
  else if(rate < 560){
    _rate = LIS3MDL_DATARATE_300_HZ;
  }
  else if(rate < 1000){
    _rate = LIS3MDL_DATARATE_560_HZ;
  }
  else{
    _rate = LIS3MDL_DATARATE_1000_HZ;
  }

  Serial.print("Sample Rate: ");
  Serial.println(_rate, HEX);


  lis3mdl->setDataRate(_rate);
}

bool lis3mdl_get_data(Adafruit_LIS3MDL *lis3mdl, lis3mdl_data_t* lis3mdl_data) {
  lis3mdl->read();      // get X Y and Z data at once

  // /*
  // // Then print out the raw data
  // Serial.print("\nX:  "); Serial.print(lis3mdl->x); 
  // Serial.print("  \tY:  "); Serial.print(lis3mdl->y); 
  // Serial.print("  \tZ:  "); Serial.println(lis3mdl->z); 
  // */

  // /* Or....get a new sensor event, normalized to uTesla */
  // sensors_event_t event; 
  // lis3mdl->getEvent(&event);
  // /* Display the results (magnetic field is measured in uTesla) */
  // Serial.print("\tX: "); Serial.print(event.magnetic.x);
  // Serial.print(" \tY: "); Serial.print(event.magnetic.y); 
  // Serial.print(" \tZ: "); Serial.print(event.magnetic.z); 
  // Serial.println(" uTesla ");

  lis3mdl_data->x = lis3mdl->x;
  lis3mdl_data->y = lis3mdl->y;
  lis3mdl_data->z = lis3mdl->z;
  return false;
}