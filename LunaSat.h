#include <SPI.h>
#include <RH_RF95_Luna.h>
#include <scheduler.h>
#include <ADXL313.h>
#include <BME688.h>
#include <LIS3MDL.h>
#include <LoRa.h>

#define LUNA_SAT_ID 2

#define PACKAGE_SIZE    33

#define EVENT_DATA_REQUEST 1

#define EEPROM_ADDR 0x50

struct package_t{
    bme_data_t bme_data;                        // 16 bytes
    adxl_data_t adxl_data;                      // 6  bytes
    lis3mdl_data_t lis3mdl_data;                // 6  bytes
    uint8_t bme_error : 1;                      // 1  byte
    uint8_t adxl_error : 1;
    uint8_t lis3mdl_error : 1;
    unsigned long time_stamp;                   // 4  bytes
};                                     // total = 33 bytes

struct package_header_t{
    uint16_t num_packages;
    unsigned long current_time;
};

void sample_data(package_t *package, Adafruit_BME680* bme, ADXL313* adxl, Adafruit_LIS3MDL* lis3mdl);

void print_package(package_t *package);

void print_package_for_serial(package_t *package);

void bytes_to_header(package_header_t *header, uint8_t* buf);

void bytes_to_package(package_t *package, uint8_t* buf);

void store_package(package_t *package, uint8_t page);

void get_package(package_t *package, uint8_t page);


// ======================= DYNAMIC PACKAGES ============================


struct d_header_pack_t {  // 10 bytes
    uint16_t bme_n;
    uint16_t adxl_n;
    uint16_t lis3mdl_n;
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
    lis3mdl_data_t lis3mdl_data;
    unsigned long time_stamp;
};


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