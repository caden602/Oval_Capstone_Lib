#include <Wire.h>
#include <Arduino.h>

#define EEPROM_ADDR 0x50

typedef enum{
    bme_t,
    adxl_t,
    lis_t
} sensor_choice_t;


static uint16_t bme_page_start;
static uint16_t adxl_page_start;
static uint16_t lis_page_start;
static uint16_t pwr_page_start;

static uint8_t bme_offset = 0;
static uint8_t adxl_offset = 0;
static uint8_t lis_offset = 0;
static uint8_t pwr_offset = 0;

void eeprom_map_pages(uint16_t bme_f, uint16_t adxl_f, uint16_t lis_f);

void eeprom_store_data(uint8_t * bytes, uint8_t size, sensor_choice_t sens);

void eeprom_get_bytes(uint8_t *bytes, uint8_t size, uint16_t address);

uint16_t eeprom_get_address(uint16_t page, uint8_t offset);

uint16_t get_and_set_bme_count();

uint16_t get_and_set_adxl_count();

uint16_t get_and_set_lis_count();

void eeprom_reset();

void eeprom_get_bytes(uint8_t *bytes, uint8_t size, uint16_t address);

void eeprom_store_bytes(uint8_t *bytes, uint8_t size, uint16_t address);
