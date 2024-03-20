#include <SPI.h>
#include <RH_RF95_Luna.h>
#include <scheduler.h>
#include <ADXL313.h>
#include <BME688.h>
#include <LIS3MDL.h>
#include <LoRa.h>

#define LUNA_SAT_ID 1

#define PACKAGE_SIZE    37

#define EVENT_DATA_REQUEST 1

struct package_t{
    bme_data_t bme_data;                        // 16 bytes
    adxl_data_t adxl_data;                      // 6  bytes
    lis3mdl_data_t lis3mdl_data;                // 6  bytes
    uint8_t bme_error : 1;                      // 1  byte
    uint8_t adxl_error : 1;
    uint8_t lis3mdl_error : 1;
    unsigned long time_stamp;                   // 8  bytes
};                                     // total = 37 bytes

void print_package(package_t *package);

void print_package_for_serial(package_t *package);

package_t bytes_to_package(uint8_t* buf);