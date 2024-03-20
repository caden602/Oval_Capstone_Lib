#include <scheduler.h>

//***********************************************************************************
// private variables
//***********************************************************************************

static uint8_t event_scheduled;

//***********************************************************************************
// function defintions
//***********************************************************************************
void scheduler_open(void){
    event_scheduled = 0;
}

uint8_t get_scheduled_events(void){
  return event_scheduled;
}

void add_scheduled_event(uint8_t event){
  event_scheduled |= event;
}

void remove_scheduled_event(uint8_t event){
  event_scheduled &= ~event;
}
