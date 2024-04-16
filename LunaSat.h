#include <SPI.h>
#include <scheduler.h>
#include <ADXL313.h>
#include "Adafruit_BME680.h"
#include <LIS3MDL.h>
#include <BME688.h>
#include <LoRa.h>
#include <EEPROM.h>

#define LUNA_SAT_ID 2

#define PACKAGE_SIZE    sizeof(package_t)

#define EVENT_DATA_REQUEST 1

#define EEPROM_ADDR 0x50

struct package_t{
    bme_data_t bme_data;                        // 16 bytes
    adxl_data_t adxl_data;                      // 6  bytes
    lis_data_t lis_data;                        // 6  bytes
    uint8_t bme_error : 1;                      // 1  byte
    uint8_t adxl_error : 1;
    uint8_t lis3mdl_error : 1;
    unsigned long time_stamp;                   // 4  bytes
};                                              // total = 69 bytes

struct package_header_t{
    uint16_t num_packages;
    unsigned long current_time;
};


struct d_header_pack_t {  // 10 bytes
    uint16_t bme_n;
    uint16_t adxl_n;
    uint16_t lis_n;
    unsigned long local_time_stamp;
};

struct d_bme_pack_t {   // 20 bytes
    bme_data_t bme_data;
    unsigned long time_stamp;
};

struct d_adxl_pack_t {  // 10 bytes
    adxl_data_t adxl_data;
    unsigned long time_stamp;
};

struct d_lis_pack_t {   // 10 bytes
    lis_data_t lis_data;
    unsigned long time_stamp;
};




// ======================= Sensor Functions ============================

void bme_setup(Adafruit_BME680 *bme);

void bme_sample_data(Adafruit_BME680 *bme);

void adxl_setup(ADXL313* adxl);

void adxl_set_data_rate(ADXL313 *adxl, double rate);

void adxl_sample_data(ADXL313* adxl);

void lis_setup(Adafruit_LIS3MDL* lis3mdl);

void lis_set_data_rate(Adafruit_LIS3MDL *lis, double rate);

void lis_sample_data(Adafruit_LIS3MDL* lis3mdl);


// ================== Byte Conversion Functions =======================

void bytes_to_header(uint8_t* buf, d_header_pack_t *header);

void bytes_to_bme_data(uint8_t * buf, d_bme_pack_t* bme_pack);

void bytes_to_adxl_data(uint8_t * buf, d_adxl_pack_t* adxl_pack);

void bytes_to_lis_data(uint8_t * buf, d_lis_pack_t* lis_pack);

// ======================= DYNAMIC PACKAGES ============================

void send_data(RH_RF95* rf95);

/* 
Logic flow:
First messagge is to determine number of packages per sensor
    Contains number of packages
    and current time on LSAT

Future messages are packages of different sizes:
    package size = single pack size times number of packages 
    number of packages = package size divided by max pack size (128 bytes)
        Leftower data will have a smaller pack size which should be calculated as well
*/