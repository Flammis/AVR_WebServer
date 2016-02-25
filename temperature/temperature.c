
#include <temperature.h>
#include <adc.h>
#include <timer.h>
#include <string.h>
#include <stdint.h>
#include "../debug.h"


static void temperature_calculate(timer_t timer, void * arg);
static void print_temperature(void);

static uint16_t temperature;
static timer_t timer;

void temperature_init(void)
{
  //Set as ADC0 pin as input
  DDRC &= 0xFE;
  PORTC &= 0xFE;
  
  timer = timer_alloc(temperature_calculate, TEMPERATURE_UPDATE_MS);
  if(timer < 0){
    DBG_STATIC("Failed to allocate timer.");
  } else {
    DBG_STATIC("Successfully allocated timer.");    
  }
  timer_reset(timer);
}

void temperature_calculate(timer_t timer, void * arg)
{
  DBG_STATIC("ADC:");
  temperature = adc_read(CHANNEL_0);
  print_temperature();
  timer_reset(timer);
}

void print_temperature(void)
{    
  char buffer[25];
  sprintf(buffer, "Temperature value: %" PRIu16, temperature);
  DBG_DYNAMIC(buffer);
}