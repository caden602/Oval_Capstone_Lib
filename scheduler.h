#include <stdint.h>

void scheduler_open(void);

uint8_t get_scheduled_events(void);

void add_scheduled_event(uint8_t event);

void remove_scheduled_event(uint8_t event);