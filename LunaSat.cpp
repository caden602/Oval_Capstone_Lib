#include <LunaSat.h>

void sample_data(package_t *package, Adafruit_BME680* bme, ADXL313* adxl, Adafruit_LIS3MDL* lis3mdl){
    bme_data_t bme_data;
    adxl_data_t adxl_data;
    lis3mdl_data_t lis3mdl_data;

    // Get sensor data
    // Serial.println("Sampling BME688");
    bool bme_error = bme_get_data(bme, &bme_data);
    package->bme_data = bme_data;
    package->bme_error = bme_error;

    // Serial.println("Sampling ADXL313");
    bool adxl_error = adxl_get_data(adxl, &adxl_data);
    package->adxl_data = adxl_data;
    package->adxl_error = adxl_error;

    // Serial.println("Sampling LIS3MDL");
    bool lis3mdl_error = lis3mdl_get_data(lis3mdl, &lis3mdl_data);
    package->lis3mdl_data = lis3mdl_data;
    package->lis3mdl_error = lis3mdl_error;

    package->time_stamp = millis();
}


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
    // uint8_t left_over_chunks = PACKAGE_SIZE % chunk_size;

    // Calculate the page number in bytes
    uint16_t address = page << 7;
    uint8_t address_high = address >> 8;
    uint8_t address_low = address;

    // Convert package to bytes
    uint8_t* package_bytes = reinterpret_cast<uint8_t*>(package);

    // Iterate through number of chunks we need to store
    for(uint8_t i = 0; i < num_chunks; i++){
        // Start I2C transmission
        Wire.beginTransmission(EEPROM_ADDR);

        // Select write address register
        Wire.write(address_high);
        Wire.write(address_low + (i * chunk_size));

        // Serial.println(sizeof(*package));
        // Serial.print("IN DATA: ");
        for(int i=0; i < chunk_size; i++){
            Wire.write(package_bytes[i]);
            // Serial.print(package_bytes[i], HEX);
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
        // Serial.print(package_bytes[i], HEX);
        // Serial.print(' ');
    }
    // Serial.println();

    // End I2C transmission
    Wire.endTransmission();

    delay(300);

}

void get_package(package_t *package, uint8_t page){

    uint8_t raw_bytes[PACKAGE_SIZE];
    uint8_t chunk_size = 24;
    uint8_t num_chunks = PACKAGE_SIZE / chunk_size;
    uint8_t i;

    // Calculate the page number in bytes
    uint16_t address = page << 7;
    uint8_t address_high = address >> 8;
    uint8_t address_low = address;

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

        // Serial.print("OUT DATA:");
        while(Wire.available() > 0){
            // Read into raw bytes array
            raw_bytes[i] = Wire.read();
            // Serial.print(raw_bytes[i], HEX);
            // Serial.print(' ');
            i++;
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
        raw_bytes[i] = Wire.read();
        // Serial.print(raw_bytes[i], HEX);
        // Serial.print(' ');
        i++;
    }

    // Serial.println();\

    bytes_to_package(package, raw_bytes);

    delay(50);
 
}
