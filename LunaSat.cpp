#include <LunaSat.h>

unsigned long last_bme_sample;
unsigned long last_adxl_sample;
unsigned long last_lis_sample;

uint16_t lis_sample_period;
uint16_t adxl_sample_period;

// ==============================  PRINT FUNCTIONS ============================

void print_bme_for_serial(d_bme_pack_t *bme_pack){
    Serial.print("BME:");

    Serial.print('T');
    Serial.print(bme_pack->time_stamp);  Serial.print(",");

    Serial.print('t');
    Serial.print(bme_pack->bme_data.temperature);  Serial.print(",");
    Serial.print('h');
    Serial.print(bme_pack->bme_data.humidity);  Serial.print(",");
    /*
    Serial.print(bme_pack->bme_data.gas_resistance);  Serial.print(",");
    Serial.print(bme_pack->bme_data.pressure);  Serial.print(",");
    */

    Serial.println();
}

void print_adxl_for_serial(d_adxl_pack_t *adxl_pack){
    Serial.print("ADXL:");
    Serial.print('T');
    Serial.print(adxl_pack->time_stamp);  Serial.print(",");

    Serial.print(adxl_pack->adxl_data.x); Serial.print(",");
    Serial.print(adxl_pack->adxl_data.y); Serial.print(",");
    Serial.print(adxl_pack->adxl_data.z); Serial.print(",");
    
    Serial.println();
}

void print_lis_for_serial(d_lis_pack_t *lis_pack){
    Serial.print("LIS:");
    Serial.print('T');
    Serial.print(lis_pack->time_stamp);  Serial.print(",");

    Serial.print(lis_pack->lis_data.x); Serial.print(",");
    Serial.print(lis_pack->lis_data.y); Serial.print(",");
    Serial.print(lis_pack->lis_data.z); Serial.print(",");
    
    Serial.println();
}

// ==============================  BYTE CONVERSIONS ============================

void bytes_to_header(uint8_t* buf, d_header_pack_t *header){
    header->bme_n = buf[0] | (buf[1] << 8);
    header->adxl_n = buf[2] | (buf[3] << 8);
    header->lis_n = buf[4] | (buf[5] << 8);
    header->local_time_stamp = buf[6] | ((uint32_t)buf[7] << 8) | ((uint32_t)buf[8] << 16) | ((uint32_t)buf[9] << 24);
}

void bytes_to_bme_data(uint8_t * buf, d_bme_pack_t* bme_pack){
    uint32_t temp = buf[0] | ((uint32_t)buf[1] << 8) | ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
    bme_pack->bme_data.temperature = *reinterpret_cast<float*>(&temp);
    bme_pack->bme_data.pressure = buf[4] | ((uint32_t)buf[5] << 8) | ((uint32_t)buf[6] << 16) | ((uint32_t)buf[7] << 24);
    uint32_t humidity = buf[8] | ((uint32_t)buf[9] << 8) | ((uint32_t)buf[10] << 16) | ((uint32_t)buf[11] << 24);
    bme_pack->bme_data.humidity = *reinterpret_cast<float*>(&humidity);
    bme_pack->bme_data.gas_resistance = buf[12] | ((uint32_t)buf[13] << 8) | ((uint32_t)buf[14] << 16) | ((uint32_t)buf[15] << 24);
    bme_pack->time_stamp = buf[16] | ((uint32_t)buf[17] << 8) | ((uint32_t)buf[18] << 16) | ((uint32_t)buf[19] << 24);
}

void bytes_to_adxl_data(uint8_t * buf, d_adxl_pack_t* adxl_pack){
    adxl_pack->adxl_data.x = buf[0] | (buf[1] << 8);
    adxl_pack->adxl_data.y = buf[2] | (buf[3] << 8);
    adxl_pack->adxl_data.z = buf[4] | (buf[5] << 8);
    adxl_pack->time_stamp = buf[6] | ((uint32_t)buf[7] << 8) | ((uint32_t)buf[8] << 16) | ((uint32_t)buf[9] << 24);
}

void bytes_to_lis_data(uint8_t * buf, d_lis_pack_t* lis_pack){
    lis_pack->lis_data.x = buf[0] | (buf[1] << 8);
    lis_pack->lis_data.y = buf[2] | (buf[3] << 8);
    lis_pack->lis_data.z = buf[4] | (buf[5] << 8);
    lis_pack->time_stamp = buf[6] | ((uint32_t)buf[7] << 8) | ((uint32_t)buf[8] << 16) | ((uint32_t)buf[9] << 24);
}





// =========================== SENSOR SETUPS AND SAMPLE ===========================


void bme_setup(Adafruit_BME680 *bme){
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

void bme_sample_data(Adafruit_BME680 *bme){
    // Sample the BME every 1 second
    bme_data_t bme_data;

    // Check if enough time has elapsed since our last sample
    if(millis() - last_bme_sample >= 1000){
        Serial.println("Sampling BME");

        // Ensure reading was performed successfully
        if(bme->performReading()) {
            
            // Set bme data
            bme_data.temperature = bme->temperature;
            bme_data.pressure = bme->pressure / 100.0;
            bme_data.humidity = bme->humidity;
            bme_data.gas_resistance = bme->gas_resistance / 1000.0;

            // Set time for last sample
            last_bme_sample = millis();

            // Store in data package
            d_bme_pack_t bme_pack = {
                .bme_data = bme_data,
                .time_stamp = last_bme_sample
            };

            // Print data
            print_bme_for_serial(&bme_pack);

            // Convert data to bytes
            uint8_t *bytes = reinterpret_cast<uint8_t*>(&bme_pack);

            // Store package in EEPROM
            eeprom_store_data(bytes, 20U, bme_t);
        }
    }
}


void lis_setup(Adafruit_LIS3MDL *lis3mdl){
    // Try to initialize!
    if(! lis3mdl->begin_I2C()){
        Serial.println("Failed to find LIS3MDL");
        while (1);
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
    lis_sample_period = 1000 / 155;

    last_lis_sample = 0;

    delay(500);


}

void lis_set_data_rate(Adafruit_LIS3MDL *lis, double rate){
    lis3mdl_dataRate_t _rate;

    if(rate < 0.625){
        // Set sample period to max value (ie inf)
        lis_sample_period = -1;
        return;
    }
    else if(rate < 1.25){
        _rate = LIS3MDL_DATARATE_0_625_HZ;
        lis_sample_period = 1600;
    }
    else if(rate < 2.5){
        _rate = LIS3MDL_DATARATE_1_25_HZ;
        lis_sample_period = 800;
    }
    else if(rate < 5){
        _rate = LIS3MDL_DATARATE_2_5_HZ;
        lis_sample_period = 400;
    }
    else if(rate < 10){
        _rate = LIS3MDL_DATARATE_5_HZ;
        lis_sample_period = 200;
    }
    else if(rate < 20){
        _rate = LIS3MDL_DATARATE_10_HZ;
        lis_sample_period = 100;
    }
    else if(rate < 40){
        _rate = LIS3MDL_DATARATE_20_HZ;
        lis_sample_period = 50;
    }
    else if(rate < 80){
        _rate = LIS3MDL_DATARATE_40_HZ;
        lis_sample_period = 25;
    }
    else if(rate < 155){
        _rate = LIS3MDL_DATARATE_80_HZ;
        lis_sample_period = 12;
    }
    else if(rate < 300){
        _rate = LIS3MDL_DATARATE_155_HZ;
        lis_sample_period = 6;
    }
    else if(rate < 560){
        _rate = LIS3MDL_DATARATE_300_HZ;
        lis_sample_period = 3;
    }
    else if(rate < 1000){
        _rate = LIS3MDL_DATARATE_560_HZ;
        lis_sample_period = 2;
    }
    else{
        _rate = LIS3MDL_DATARATE_1000_HZ;
        lis_sample_period = 1;
    }

    Serial.print("Sample Rate: ");
    Serial.println(_rate);

    lis->setDataRate(_rate);

    delay(300);
}

void lis_sample_data(Adafruit_LIS3MDL *lis){
    lis_data_t lis_data;

    // Check if enough time has elapsed since our last sample
    if(millis() - last_lis_sample >= lis_sample_period){
        Serial.println("Sampling LIS");

        lis->read();

        lis_data.x = lis->x;
        lis_data.y = lis->y;
        lis_data.z = lis->z;

        // Set time for last sample
        last_lis_sample = millis();

        d_lis_pack_t lis_pack = {
            .lis_data = lis_data,
            .time_stamp = last_lis_sample
        };

        // Print data
        print_lis_for_serial(&lis_pack);

        // Convert data to bytes
        uint8_t *bytes = reinterpret_cast<uint8_t*>(&lis_pack);

        // Store package in EEPROM
        eeprom_store_data(bytes, 10U, lis_t);
    }
}

void adxl_setup(ADXL313 *myAdxl){
    if (!myAdxl->begin()){
        Serial.println("Failed to find AXDL313");
        while(1); 
    }
    Serial.println("ADXL313 Found!");
    
    myAdxl->measureModeOn(); // wakes up the sensor from standby and puts it into measurement mode

    last_adxl_sample = 0;

    delay(500);
}

void adxl_set_data_rate(ADXL313 *adxl, double rate){
    adxl->setRate(rate);

    if(rate == 0){
        adxl_sample_period = -1;
        return;
    }
    adxl_sample_period = 1000 / rate;
}

void adxl_sample_data(ADXL313 *adxl){
    adxl_data_t adxl_data;

    if(millis() - last_adxl_sample >= adxl_sample_period){
        Serial.println("Sampling ADXL");

        // Check data ready interrupt
        if(adxl->dataReady()){
            // Read accelerometer
            adxl->readAccel();

            // Store in data struct
            adxl_data.x = adxl->x;
            adxl_data.y = adxl->y;
            adxl_data.z = adxl->z;

            last_adxl_sample = millis();

            d_adxl_pack_t adxl_pack = {
                .adxl_data = adxl_data,
                .time_stamp = last_adxl_sample
            };

            // Print data
            print_adxl_for_serial(&adxl_pack);

            // Convert data to bytes
            uint8_t *bytes = reinterpret_cast<uint8_t*>(&adxl_pack);

            // Store package in EEPROM
            eeprom_store_data(bytes, 10U, adxl_t);
        }
    }
}

void send_sensor_data(RH_RF95* rf95, uint16_t samples, sensor_choice_t sens){

    if(samples == 0){
        return;
    }

    uint8_t size = 0;
    uint16_t page_start = 0;

    switch(sens){
        case bme_t:
            size = sizeof(d_bme_pack_t);
            page_start = bme_page_start;
            break;
        case adxl_t:
            size = sizeof(d_adxl_pack_t);
            page_start = adxl_page_start;
            break;
        case lis_t:
            size = sizeof(d_lis_pack_t);
            page_start = lis_page_start;
            break;
    }

    uint8_t samples_per_page = 128 / size;
    uint8_t pages = samples / samples_per_page;
    uint8_t remainder = samples % samples_per_page;

    // Iterate through filled in pages and send all data
    for(int i=0; i < pages; i++){
      
        // Get number of bytes on each page
        uint8_t num_bytes = size * samples_per_page;
        uint8_t bytes[num_bytes];

        // Calculate address based on start page
        uint16_t address = eeprom_get_address(page_start + i, 0);

        // Get bytes from address in eeprom and store in bytes array
        eeprom_get_bytes(bytes, num_bytes, address);

        // Small delay
        delay(100);

        // Send the bytes via LoRa
        rf95->send((uint8_t *)bytes, num_bytes);
        rf95->waitPacketSent();
    }

    // Send remaining bytes of data
    uint8_t remain_size = remainder*size;
    uint8_t bytes[remain_size];
    uint16_t address = eeprom_get_address(page_start + pages, 0);

    // Get bytes from address in eeprom and store in bytes array
    eeprom_get_bytes(bytes, remain_size, address);

    // Send the bytes via LoRa
    rf95->send((uint8_t *)bytes, size);
    rf95->waitPacketSent();
}


void send_data(RH_RF95* rf95){
    // Define header
    d_header_pack_t header;
    header.bme_n = get_and_set_bme_count();
    header.lis_n = get_and_set_lis_count();
    header.adxl_n = get_and_set_adxl_count();
    header.local_time_stamp = millis();

    // Send header
    char* header_bytes = reinterpret_cast<char*>(&header);
    rf95->send((uint8_t *)header_bytes, sizeof(d_header_pack_t));
    rf95->waitPacketSent();

    // Small delay
    delay(50);

    // Send sensor data one at a time
    send_sensor_data(rf95, header.bme_n, bme_t);
    send_sensor_data(rf95, header.adxl_n, adxl_t);
    send_sensor_data(rf95, header.lis_n, lis_t);

    // Reset eeprom now that everything has been sent
    eeprom_reset();

    // // Send stopper signifier
    // uint8_t data [] = "END OF DATA";
    // rf95->send(data, sizeof(data));
    // rf95->waitPacketSent();

    // Set back to RX mode
    rf95->setModeRx();
}



//  GENERIC

/*

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
    Serial.print("x: "); Serial.print(package->lis_data.x); Serial.print(", ");
    Serial.print("y: "); Serial.print(package->lis_data.y); Serial.print(", ");
    Serial.print("z: "); Serial.print(package->lis_data.z); Serial.println();

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

    Serial.print(package->lis_data.x); Serial.print(",");
    Serial.print(package->lis_data.y); Serial.print(",");
    Serial.print(package->lis_data.z);

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
        package->lis_data.x = x_mag;

        int16_t y_mag = (buf[25] << 8) | buf[24];
        package->lis_data.y = y_mag;

        int16_t z_mag = (buf[27] << 8) | buf[26];
        package->lis_data.z = z_mag;
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

*/