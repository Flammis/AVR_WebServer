
#include <temperature.h>
#include <adc.h>
#include <timer.h>
#include <string.h>
#include <stdint.h>
#include "../debug.h"

#define TEMPERATURE_START -550
#define TEMPERATURE_STEP 50
#define MIN_TEMPERATURE -550
#define MAX_TEMPERATURE 1550

typedef uint16_t temperature_table_entry;
typedef uint8_t temperature_table_index_type;
#define TEMPERATURE_TABLE_READ(i) pgm_read_word(&digital_temperature_table[i])

// struct temperature_t
// {
  // int16_t temp_integer;
  // uint8_t temp_decimal;
// };

static void temperature_recalculate(timer_t timer, void * arg);
static int16_t temperature_calculate(temperature_table_entry adc_value);
static void print_temperature(void);

static struct temperature_t temperature;
static timer_t timer;


static const temperature_table_entry digital_temperature_table[] PROGMEM = {
  10, 15, 21, 29, 40, 54, 73, 95, 123,
  156, 195, 239, 288, 341, 397, 454,
  510, 565, 618, 666, 711, 751, 787,
  818, 846, 870, 890, 908, 924, 937,
  948, 958, 966, 973, 979, 984,
  989, 993, 996, 999, 1002, 1004, 1006
};

void temperature_initialize(void)
{
  //Set as ADC0 pin as input
  DDRC &= 0xFE;
  PORTC &= 0xFE;
  
  timer = timer_alloc(temperature_recalculate, TEMPERATURE_UPDATE_MS);
  if(timer < 0){
    DBG_STATIC("Failed to allocate timer.");
  } else {
    DBG_STATIC("Successfully allocated timer.");    
  }
  timer_reset(timer);
}

void temperature_recalculate(timer_t timer, void * arg)
{
  uint16_t adc_value = adc_read(CHANNEL_0);
  
  int16_t temp = temperature_calculate((temperature_table_entry)adc_value);
  temperature.temp_integer = temp/10;
  temperature.temp_decimal = temp % 10;
  
  
  timer_reset(timer);
  //print_temperature();
}

/*
Calculates temperature with 1 decimal point accuracy.
*/
int16_t temperature_calculate(temperature_table_entry adc_value)
{
  temperature_table_index_type low_index = 0;
  temperature_table_index_type high_index = (sizeof(digital_temperature_table) / sizeof(digital_temperature_table[0])) - 1;
  
  /*
   * Check boundaries 
   */
  if(adc_value >= TEMPERATURE_TABLE_READ(high_index)){
    return MAX_TEMPERATURE;
  }
  
  if(adc_value <= TEMPERATURE_TABLE_READ(low_index)){
    return MIN_TEMPERATURE;
  }
  
  /*
   * BINARY SEARCH
   */
  while ((high_index - low_index) > 1) {
    temperature_table_index_type middle = (high_index + low_index) >> 1;
    temperature_table_entry middle_value = TEMPERATURE_TABLE_READ(middle);
    if (adc_value > middle_value) {
      low_index = middle;
    } else {
      high_index = middle;
    }
  }
  
  temperature_table_entry v_high = TEMPERATURE_TABLE_READ(high_index);
  if(adc_value >= v_high){
    return (int16_t)(TEMPERATURE_START + (TEMPERATURE_STEP*high_index));
  }
  
  temperature_table_entry v_low = TEMPERATURE_TABLE_READ(low_index);
  temperature_table_entry v_delta = v_high - v_low;
  
  int16_t temp = (int16_t)(TEMPERATURE_START + (TEMPERATURE_STEP*low_index));
  
  if(v_delta){
    temperature_table_entry interpolatate = (adc_value-v_low)*1000;
    interpolatate = interpolatate/v_delta;
    temperature_table_entry rest = interpolatate % 100;
    interpolatate = interpolatate/100;
    interpolatate = interpolatate * (TEMPERATURE_STEP/10);
    interpolatate = interpolatate + (rest*(TEMPERATURE_STEP/10)/100);
    temp += interpolatate;
  }
  return temp;
}

const struct temperature_t* get_temperature(void)
{
  return (const struct temperature_t*)&temperature;
}


void print_temperature(void)
{    
  char buffer[25];
  sprintf(buffer, "Temperature value: %" PRId16 ".%" PRIu8, temperature.temp_integer, temperature.temp_decimal);
  DBG_DYNAMIC(buffer);
}