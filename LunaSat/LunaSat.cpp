#include <LunaSat.h>

void print_package(package_t *package){
    Serial.println("BME DATA: ");
    Serial.print("Humi: "); Serial.print(package->bme_data.humidity);
    Serial.print("Gas Res: "); Serial.print(package->bme_data.gas_resistance);
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
    // Serial.print(package->bme_data.gas_resistance);  Serial.print(",");
    // Serial.print(package->bme_data.pressure);  Serial.print(",");

    Serial.print(package->adxl_data.x); Serial.print(",");
    Serial.print(package->adxl_data.y); Serial.print(",");
    Serial.print(package->adxl_data.z); Serial.print(",");

    Serial.print(package->lis3mdl_data.x); Serial.print(",");
    Serial.print(package->lis3mdl_data.y); Serial.print(",");
    Serial.print(package->lis3mdl_data.z); Serial.print(",");

    Serial.println();
}

package_t bytes_to_package(uint8_t* buf){
    package_t package;

    if(package.bme_error){
        uint32_t temp = (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0];
        package.bme_data.temperature = *reinterpret_cast<float*>(&temp);

        uint32_t pressure = (buf[7] << 24) | (buf[6] << 16) | (buf[5] << 8) | buf[4];
        package.bme_data.pressure = pressure;

        uint32_t humidity = (buf[11] << 24) | (buf[10] << 16) | (buf[9] << 8) | buf[8];
        package.bme_data.humidity = *reinterpret_cast<float*>(&humidity);

        uint32_t gas_res = (buf[15] << 24) | (buf[14] << 16) | (buf[13] << 8) | buf[12];
        package.bme_data.gas_resistance = gas_res;
    }
    else{
        Serial.println("BME Data error!");
    }

    if(package.adxl_error){
        int16_t x = (buf[17] << 8) | buf[16];
        package.adxl_data.x = x;

        int16_t y = (buf[19] << 8) | buf[18];
        package.adxl_data.y = y;

        int16_t z = (buf[21] << 8) | buf[20];
        package.adxl_data.z = z;
    }
    else{
        Serial.println("ADXL Data error!");
    }

    if(package.lis3mdl_error){
        int16_t x_mag = (buf[23] << 8) | buf[22];
        package.lis3mdl_data.x = x_mag;

        int16_t y_mag = (buf[25] << 8) | buf[24];
        package.lis3mdl_data.y = y_mag;

        int16_t z_mag = (buf[27] << 8) | buf[26];
        package.lis3mdl_data.z = z_mag;
    }
    else{
        Serial.println("LIS3MDL Data error!");
    }

    package.bme_error = buf[28] & 1;
    package.adxl_error = (buf[28] >> 1) & 1;
    package.lis3mdl_error = (buf[28] >> 2) & 1;

    unsigned long time = 0;
    time = (buf[32] << 24) | (buf[31] << 16) | (buf[30] << 8) | buf[29];
    time |= (buf[36] << 56) | (buf[35] << 48) | (buf[34] << 40) | (buf[33] << 32);
    package.time_stamp = time;

    return package;
}