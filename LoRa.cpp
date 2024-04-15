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