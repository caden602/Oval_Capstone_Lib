#include <RH_RF95_Luna.h>

#define RF95_FREQ   915.0

// LoRa Setup function, params are rf object and rst pin
void lora_setup(RH_RF95 *rf95, int rst, bool lunasat);