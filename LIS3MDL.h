// Basic demo for magnetometer readings from Adafruit LIS3MDL

#include <Wire.h>
#include <Adafruit_LIS3MDL.h>
#include <Adafruit_Sensor.h>


#define LIS3MDL_CLK 13
#define LIS3MDL_MISO 12
#define LIS3MDL_MOSI 11
#define LIS3MDL_CS 10

typedef struct{
    int16_t x;              // 16 bits -> 2 bytes
    int16_t y;              // 16 bits -> 2 bytes
    int16_t z;              // 16 bits -> 2 bytes
} lis3mdl_data_t;           // total   -> 6 bytes

void lis3mdl_setup(Adafruit_LIS3MDL *lis3mdl);

void lis3mdl_set_data_rate(Adafruit_LIS3MDL *lis3mdl, double rate);

bool lis3mdl_get_data(Adafruit_LIS3MDL *lis3mdl, lis3mdl_data_t* lis3mdl_data);
