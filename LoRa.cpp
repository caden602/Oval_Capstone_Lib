#include <LoRa.h>

void lora_setup(RH_RF95 *rf95, int rst, bool lunasat){
    // Pins for LoRa Module
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(rst, OUTPUT);
    digitalWrite(rst, HIGH);

    // Init LoRa
    // Serial.println("Begin LoRa Setup!");

    // manual reset
    digitalWrite(rst, LOW);
    delay(10);
    digitalWrite(rst, HIGH);
    delay(10);
    while (!rf95->init(lunasat)) {
        Serial.println("LoRa Setup failed!");
        while (1);
    }

    // Set frequency to 915 MHz
    if (!rf95->setFrequency(RF95_FREQ)) {
        Serial.println("setFrequency failed");
        while (1);
    }

     Serial.println("LoRa Setup Successful!");

    // Setting TX power changes the current consumption durring tranmission (23 is max)
    rf95->setTxPower(6, false);

    if(lunasat){
        rf95->setModeRx();
    }

}

/*
void lora_send_data(){
    // get number of each transmission

    // send header

    header.current_time = millis();

    char* header_bytes = reinterpret_cast<char*>(&header);
    rf95.send((uint8_t *)header_bytes, 6);
    rf95.waitPacketSent();

    delay(50);

    //Serial.println("Send Data");
    package_t package_new;

    // Send all data from EEPROM (TODO: Condense packages into minimal # of TX)
    for(int i=0; i < page_num; i++){    
        // delay(300);
        get_package(&package_new, i);

        char* package_bytes = reinterpret_cast<char*>(&package_new);
        rf95.send((uint8_t *)package_bytes, PACKAGE_SIZE);
        rf95.waitPacketSent();
    }
    // Reset number of packages in EEPROM
    page_num = 0;

    // Send stopper signifier
    uint8_t data [] = "END OF DATA";

    // Get current time and convert to bytes
    //unsigned long cur_time = millis();
    //char* time_bytes = reinterpret_cast<char*>(cur_time);

    // add to end signifier 
    //strcat(data, time_bytes);

    rf95.send(data, sizeof(data));
    rf95.waitPacketSent();

    // Serial.println("Done Sending Packages");
    
    rf95.setModeRx();

}

*/