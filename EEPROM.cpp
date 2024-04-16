#include <EEPROM.h>

uint16_t bme_cur_page;
uint16_t adxl_cur_page;
uint16_t lis_cur_page;
uint16_t pwr_cur_page;

static uint16_t bme_count = 0;
static uint16_t lis_count = 0;
static uint16_t adxl_count = 0;

uint16_t get_and_set_bme_count(){
    uint16_t count = bme_count;
    bme_count = 0;
    return count;
}

uint16_t get_and_set_adxl_count(){
    uint16_t count = adxl_count;
    adxl_count = 0;
    return count;
}

uint16_t get_and_set_lis_count(){
    uint16_t count = lis_count;
    lis_count = 0;
    return count;
}


void eeprom_map_pages(uint16_t bme_f, uint16_t adxl_f, uint16_t lis_f) {
    uint16_t tot = bme_f * 2 + adxl_f * 1 + lis_f * 1;

    uint16_t a1 = 512 * bme_f * 2 / tot;
    uint16_t a2 = (512 * adxl_f / tot) + a1;

    uint16_t bme_add = 0;
    uint16_t accel_add = a1;
    uint16_t lis_add = a2;

    // Set page start values
    bme_page_start = bme_add;
    adxl_page_start = accel_add;
    lis_page_start = lis_add;

    // Set current page to start page
    bme_cur_page = bme_page_start;
    adxl_cur_page = adxl_page_start;
    lis_cur_page = lis_page_start;

    Serial.print("PAGE STARTS: ");
    Serial.print(bme_page_start);
    Serial.print(" ");
    Serial.print(adxl_page_start);
    Serial.print(" ");
    Serial.print(lis_page_start);
    Serial.println();


    // TODO: Set power pages
    pwr_page_start = 512;
}


uint16_t eeprom_get_address(uint16_t page, uint8_t offset){
    // page should be < 512
    if(page < 512 && offset < 128){
        return page << 7 | (uint16_t)offset;
    }
    Serial.println("EEPROM ADDRESS ERROR");
    return -1;
}

void eeprom_store_bytes(uint8_t *bytes, uint8_t size, uint16_t address){
    uint8_t chunk_size = 24; // Need to send in chunk sizes < 30;

    uint8_t num_chunks = size / chunk_size;

    // Calculate the page number in bytes
    uint8_t address_high = address >> 8;
    uint8_t address_low = address;

    // Serial.print("IN DATA:  ");
    // Serial.print("@ addr ");
    // Serial.print(address, HEX);
    // Serial.print("  : ");
    // Iterate through number of chunks we need to store
    for(uint8_t i = 0; i < num_chunks; i++){
        // Start I2C transmission
        Wire.beginTransmission(EEPROM_ADDR);

        // Select write address register
        Wire.write(address_high);
        Wire.write(address_low + (i * chunk_size));

        // Send the number of bytes in each chunk
        for(int j = chunk_size*i; j < chunk_size*(i+1); j++){
            Wire.write(bytes[j]);
            // Serial.print(bytes[j], HEX);
            // Serial.print(' ');
        }
        // End I2C transmission
        Wire.endTransmission();

        // Delay before reconnecting to I2C line
        delay(50);
    }

    // Send the remaining bytes 

    // Start I2C transmission
    Wire.beginTransmission(EEPROM_ADDR);

    // Select write address register
    Wire.write(address_high);
    Wire.write(address_low+(chunk_size * num_chunks));

    // Send the remaining bytes
    for(int i=chunk_size*num_chunks; i < size; i++){
        Wire.write(bytes[i]);
        // Serial.print(bytes[i], HEX);
        // Serial.print(' ');
    }
    Serial.println();

    // End I2C transmission
    Wire.endTransmission();

    delay(50);
}

void eeprom_get_bytes(uint8_t *bytes, uint8_t size, uint16_t address){

    uint8_t chunk_size = 24;
    uint8_t num_chunks = size / chunk_size;
    uint8_t i;
    uint8_t j = 0;

    // Calculate the page number in bytes
    uint8_t address_high = address >> 8;
    uint8_t address_low = address;

    // Serial.print("OUT DATA: ");
    // Serial.print("@ addr ");
    // Serial.print(address, HEX);
    // Serial.print("  : ");
    for(i = 0; i < num_chunks; i++){
        // Start I2C transmission
        Wire.beginTransmission(EEPROM_ADDR);

        // Select write address register
        Wire.write(address_high);
        Wire.write(address_low + (i * chunk_size));

        // End I2C transmission
        Wire.endTransmission();

        // Request bytes of data in chunks of 24 (32 is max, but get errors > 30)
        Wire.requestFrom(EEPROM_ADDR, chunk_size);

        while(Wire.available() > 0){
            // Read into raw bytes array
            bytes[j] = Wire.read();
            // Serial.print(bytes[j], HEX);
            // Serial.print(' ');
            j++;
        }
        delay(50);
    }

    // Start I2C transmission
    Wire.beginTransmission(EEPROM_ADDR);

    // Select write address register, offset by chunk_size
    Wire.write(address_high);
    Wire.write(address_low + (num_chunks * chunk_size));

    // End I2C transmission
    Wire.endTransmission();

    // Request bytes of data equal to size of package
    Wire.requestFrom(EEPROM_ADDR, size-(chunk_size * num_chunks));

    while(Wire.available() > 0)
    {
        // Read into raw bytes array
        bytes[j] = Wire.read();
        // Serial.print(bytes[j], HEX);
        // Serial.print(' ');
        j++;
    }

    // Serial.println();

    delay(50);
}

void eeprom_store_data(uint8_t * bytes, uint8_t size, sensor_choice_t sens){
    uint16_t cur_page;
    uint8_t offset;
    uint16_t next_sens_page;

    // Get the current page and offset based on sensor
    if(sens == bme_t){
        cur_page = bme_cur_page;
        offset = bme_offset;
        next_sens_page = adxl_page_start;
    }
    else if(sens = adxl_t){
        cur_page = adxl_cur_page;
        offset = adxl_offset;
        next_sens_page = lis_page_start;
    }
    else{
        cur_page = lis_cur_page;
        offset = lis_offset;
        next_sens_page = pwr_page_start;
    }

    // Check here if sensor data page is overflowing into next sensors 
    if(cur_page >= next_sens_page){
        // could also set some global variable that says stop reading this sensor to save power
        return;
    }

    // Get address to store data
    uint16_t address = eeprom_get_address(cur_page, offset);

    // Serial.print("ADDRESS: ");
    // Serial.println(address);

    // Store data bytes in eeprom at address
    eeprom_store_bytes(bytes, size, address);

    // if the next data package will overflow the page
    if(offset + size > 128 - size){
        // iterate the sens page and set offset back to zero
        cur_page += 1;
        offset = 0;
    }
    // Otherwise, iterate the offset
    else{
        offset += size;
    }

    // Iterate the global variables
    switch(sens){
        case bme_t: 
            bme_count ++;
            bme_offset = offset;
            bme_cur_page = cur_page;
            break;
        case adxl_t: 
            adxl_count ++;
            adxl_offset = offset;
            adxl_cur_page = cur_page;
            break;
        case lis_t: 
            lis_count ++;
            lis_offset = offset;
            lis_cur_page = cur_page;
            break;
    }
}

void eeprom_get_data(uint8_t * bytes, uint8_t size, sensor_choice_t sens){

}

void eeprom_reset(){
    bme_count = 0;
    bme_offset = 0;
    bme_cur_page = bme_page_start;

    adxl_count = 0;
    adxl_offset = 0;
    adxl_cur_page = adxl_page_start;

    lis_count = 0;
    lis_offset = 0;
    lis_cur_page = lis_page_start;

    delay(50);
}


/*GENERIC EEPROM PACKAGE STORAGE 

void store_package(package_t *package, uint8_t page){
    // Notes
    // Seems to only write in ~30 byte chunks before messing up
    // so we are going to send them in 24 byte chunks for now

    uint8_t chunk_size = 24; // Need to send in chunk sizes < 30;

    uint8_t num_chunks = PACKAGE_SIZE / chunk_size;
    Serial.println(PACKAGE_SIZE);
    // uint8_t left_over_chunks = PACKAGE_SIZE % chunk_size;

    // Calculate the page number in bytes
    uint16_t address = uint16_t(page) << 7;
    uint8_t address_high = address >> 8;
    uint8_t address_low = address;

    // Convert package to bytes
    uint8_t* package_bytes = reinterpret_cast<uint8_t*>(package);

    Serial.print("IN DATA:  ");
    // Iterate through number of chunks we need to store
    for(uint8_t i = 0; i < num_chunks; i++){
        // Start I2C transmission
        Wire.beginTransmission(EEPROM_ADDR);

        // Select write address register
        Wire.write(address_high);
        Wire.write(address_low + (i * chunk_size));

        // Serial.println(sizeof(*package));
        for(int j = chunk_size*i; j < chunk_size*(i+1); j++){
            Wire.write(package_bytes[j]);
            Serial.print(package_bytes[j], HEX);
            // Serial.print(' ');
        }
        // End I2C transmission
        Wire.endTransmission();

        delay(300);
    }

    // Send the remaining bytes 

    // Start I2C transmission
    Wire.beginTransmission(EEPROM_ADDR);

    // Select write address register
    Wire.write(address_high);
    Wire.write(address_low+(chunk_size * num_chunks));

    for(int i=chunk_size*num_chunks; i < PACKAGE_SIZE; i++){
        Wire.write(package_bytes[i]);
        Serial.print(package_bytes[i], HEX);
        Serial.print(' ');
    }
    Serial.println();

    // End I2C transmission
    Wire.endTransmission();

    delay(300);

}

void get_package(package_t *package, uint8_t page){

    uint8_t raw_bytes[PACKAGE_SIZE];
    uint8_t chunk_size = 24;
    uint8_t num_chunks = PACKAGE_SIZE / chunk_size;
    uint8_t i;
    uint8_t j = 0;

    // Calculate the page number in bytes
    uint16_t address = page << 7;
    uint8_t address_high = address >> 8;
    uint8_t address_low = address;

    Serial.print("OUT DATA: ");
    for(i = 0; i < num_chunks; i++){
        // Start I2C transmission
        Wire.beginTransmission(EEPROM_ADDR);

        // Select write address register
        Wire.write(address_high);
        Wire.write(address_low + (i * chunk_size));

        // End I2C transmission
        Wire.endTransmission();

        // Request bytes of data in chunks of 24 (32 is max, but get errors > 30)
        Wire.requestFrom(EEPROM_ADDR, chunk_size);

        while(Wire.available() > 0){
            // Read into raw bytes array
            raw_bytes[j] = Wire.read();
            Serial.print(raw_bytes[j], HEX);
            // Serial.print(' ');
            j++;
        }
        delay(50);
    }

    // Start I2C transmission
    Wire.beginTransmission(EEPROM_ADDR);

    // Select write address register, offset by chunk_size
    Wire.write(address_high);
    Wire.write(address_low + (num_chunks * chunk_size));

    // End I2C transmission
    Wire.endTransmission();

    // Request bytes of data equal to size of package
    Wire.requestFrom(EEPROM_ADDR, PACKAGE_SIZE-(chunk_size * num_chunks));

    while(Wire.available() > 0)
    {
        // Read into raw bytes array
        raw_bytes[j] = Wire.read();
        Serial.print(raw_bytes[j], HEX);
        // Serial.print(' ');
        j++;
    }

    // Serial.println();
    Serial.println();

    bytes_to_package(package, raw_bytes);

    delay(50);
 
}

*/
