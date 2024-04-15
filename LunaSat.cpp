#include <LunaSat.h>

unsigned long last_bme_sample;
unsigned long last_adxl_sample;
unsigned long last_lis_sample;

int bme_page_start;
int adxl_page_start;
int lis_page_start;

int bme_offset;
int adxl_offset;
int lis_offset;

int bme_cur_page;
int adxl_cur_page;
int lis_cur_page;


void print_package(package_t *package){
    Serial.println("BME DATA: ");
    Serial.print("Humi: "); Serial.println(package->bme_data.humidity);
    Serial.print("Gas Res: "); Serial.println(package->bme_data.gas_resistance);
    Serial.print("Temp: "); Serial.println(package->bme_data.temperature);
    Serial.print("Pres: "); Serial.println(package->bme_data.pressure);

    Serial.println();

    Serial.println("ADXL DATA: ");
    Serial.print("x: "); Serial.print(package->adxl_data.x); Serial.print(", ");
    Serial.print("y: "); Serial.print(package->adxl_data.y); Serial.print(", ");
    Serial.print("z: "); Serial.print(package->adxl_data.z); Serial.println();

    Serial.println();

    Serial.println("LIS3MDL DATA: ");
    Serial.print("x: "); Serial.print(package->lis3mdl_data.x); Serial.print(", ");
    Serial.print("y: "); Serial.print(package->lis3mdl_data.y); Serial.print(", ");
    Serial.print("z: "); Serial.print(package->lis3mdl_data.z); Serial.println();

    Serial.println();
}

void print_package_for_serial(package_t *package){
    Serial.print(package->time_stamp);  Serial.print(",");

    Serial.print(package->bme_data.temperature);  Serial.print(",");
    Serial.print(package->bme_data.humidity);  Serial.print(",");
    Serial.print(package->bme_data.gas_resistance);  Serial.print(",");
    Serial.print(package->bme_data.pressure);  Serial.print(",");

    Serial.print(package->adxl_data.x); Serial.print(",");
    Serial.print(package->adxl_data.y); Serial.print(",");
    Serial.print(package->adxl_data.z); Serial.print(",");

    Serial.print(package->lis3mdl_data.x); Serial.print(",");
    Serial.print(package->lis3mdl_data.y); Serial.print(",");
    Serial.print(package->lis3mdl_data.z);

    Serial.println();
}

void bytes_to_header(package_header_t *header, uint8_t* buf){
    header->num_packages = buf[0] | (buf[1] << 8);
    header->current_time = buf[2] | (buf[3] << 8) | (buf[4] << 16) | (buf[5] << 24);
}

void bytes_to_package(package_t *package, uint8_t* buf){

    // Get error bits first
    package->bme_error = (buf[28]) & 1;
    package->adxl_error = (buf[28] >> 1) & 1;
    package->lis3mdl_error = (buf[28] >> 2) & 1;

    if(!package->bme_error){
        // FOr some reason these bit calcultions are not working for floats, not sure why??
        // they were working last week :/
        uint32_t temp = ((uint32_t)buf[3] << 24) | ((uint32_t)buf[2] << 16) | ((uint32_t)buf[1] << 8) | buf[0];

        // uint32_t temp_low = (buf[1] << 8) | buf[0] & 0x0000FFFF;
        // uint32_t temp_high = (buf[3] << 8) | (buf[2]);
        // uint32_t temp = (temp_high << 16) | temp_low;

        package->bme_data.temperature = *reinterpret_cast<float*>(&temp);

        uint32_t pressure = ((uint32_t)buf[7] << 24) | ((uint32_t)buf[6] << 16) | ((uint32_t)buf[5] << 8) | buf[4];
        package->bme_data.pressure = pressure;

        uint32_t humidity = ((uint32_t)buf[11] << 24) | ((uint32_t)buf[10] << 16) | ((uint32_t)buf[9] << 8) | buf[8];
        // uint32_t humi_low = (buf[9] << 8) | buf[8] & 0x0000FFFF;
        // uint32_t humi_high = (buf[11] << 8) | (buf[10]);
        // uint32_t humidity = (humi_high << 16) | humi_low;
        package->bme_data.humidity = *reinterpret_cast<float*>(&humidity);

        uint32_t gas_res = ((uint32_t)buf[15] << 24) | ((uint32_t)buf[14] << 16) | ((uint32_t)buf[13] << 8) | buf[12];
        package->bme_data.gas_resistance = gas_res;
    }
    else{
        //Serial.println("BME Data error!");
    }

    if(!package->adxl_error){
        int16_t x = (buf[17] << 8) | buf[16];
        package->adxl_data.x = x;

        int16_t y = (buf[19] << 8) | buf[18];
        package->adxl_data.y = y;

        int16_t z = (buf[21] << 8) | buf[20];
        package->adxl_data.z = z;
    }
    else{
        //Serial.println("ADXL Data error!");
    }

    if(!package->lis3mdl_error){
        int16_t x_mag = (buf[23] << 8) | buf[22];
        package->lis3mdl_data.x = x_mag;

        int16_t y_mag = (buf[25] << 8) | buf[24];
        package->lis3mdl_data.y = y_mag;

        int16_t z_mag = (buf[27] << 8) | buf[26];
        package->lis3mdl_data.z = z_mag;
    }
    else{
        // Serial.println("LIS3MDL Data error!");
    }

    unsigned long time = ((uint32_t)buf[32] << 24) | ((uint32_t)buf[31] << 16) | ((uint32_t)buf[30] << 8) | buf[29];

    // uint32_t time_low = (buf[30] << 8) | buf[29] & 0x0000FFFF;
    // uint32_t time_high = (buf[32] << 8) | buf[31];
    // unsigned long time = (time_high << 16) | time_low;

    package->time_stamp = time;

}

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

// ======================= DYNAMIC PACKAGES ============================

void map_eprom(uint16_t bme_f, uint16_t adxl_f, uint16_t lis_f) {
    uint16_t tot = bme_f * 2 + adxl_f * 1 + lis_f * 1;

    uint16_t bme_r = bme_f * 2 / tot;
    uint16_t adxl_r = adxl_f / tot;
    uint16_t lis_r = lis_f / tot;

    uint8_t a1 = 512 * bme_r;
    uint8_t a2 = 512 * adxl_r + a1;

    uint8_t bme_add = 0;
    uint8_t accel_add = a1;
    uint8_t lis_add = a2;
}


uint16_t eeprom_get_address(uint16_t page, uint8_t offset){
    // page should be < 512
    if(page < 512 && offset < 128){
        return page << 7 | offset;
    }
    return -1;
}

void eeprom_store_bytes(uint8_t *bytes, uint8_t size, uint16_t address){
    uint8_t chunk_size = 24; // Need to send in chunk sizes < 30;

    uint8_t num_chunks = size / chunk_size;
    Serial.println(size);

    // Calculate the page number in bytes
    uint8_t address_high = address >> 8;
    uint8_t address_low = address;

    Serial.print("IN DATA:  ");
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
            Serial.print(bytes[j], HEX);
            // Serial.print(' ');
        }
        // End I2C transmission
        Wire.endTransmission();

        // Delay before reconnecting to I2C line
        delay(300);
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
        Serial.print(bytes[i], HEX);
        // Serial.print(' ');
    }
    Serial.println();

    // End I2C transmission
    Wire.endTransmission();

    delay(300);
}

void eeprom_get_bytes(uint8_t *bytes, uint8_t size, uint16_t address){

    uint8_t chunk_size = 24;
    uint8_t num_chunks = size / chunk_size;
    uint8_t i;
    uint8_t j = 0;

    // Calculate the page number in bytes
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
            bytes[j] = Wire.read();
            Serial.print(bytes[j], HEX);
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
        bytes[j] = Wire.read();
        Serial.print(bytes[j], HEX);
        // Serial.print(' ');
        j++;
    }

    Serial.println();

    delay(50);
 
}

void bme_setup(Adafruit_BME680 *bme) {
    if(!bme->begin(0x76)){
        Serial.println("Failed to find BME688");
        while(1);
    }
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
    // Sample the BME every 1 second
    bme_data_t bme_data;

    // Check if enough time has elapsed since our last sample
    if(millis() - last_bme_sample >= 1000){
        // Ensure reading was performed successfully
        if(bme->performReading()) {
            
            // Set bme data
            bme_data.temperature = bme->temperature;
            bme_data.pressure = bme->pressure / 100.0;
            bme_data.humidity = bme->humidity;
            bme_data.gas_resistance = bme->gas_resistance / 1000.0;

            // Set time for last sample
            last_bme_sample = millis();

            // Check here if BME page is overflowing into next sensors 
            if(bme_cur_page >= adxl_page_start){
                // could also set some global variable that says stop reading this sensor to save power
                return;
            }

            // Get address to store data
            uint16_t address = eeprom_get_address(bme_cur_page, bme_offset);

            // Convert data to bytes
            uint8_t *bytes = reinterpret_cast<uint8_t*>(&bme_data);

            // Store data bytes in eeprom at address
            eeprom_store_bytes(bytes, sizeof(bme_data), address);

            // if the next bme_data package will overflow the page
            if(bme_offset + sizeof(bme_data) > 128){
                // iterate the bme page and set offset back to zero
                bme_cur_page += 1;
                bme_offset = 0;
            }
            // Otherwise, iterate the offset
            else{
                bme_offset += sizeof(bme_data);
            }
        }
    }
}