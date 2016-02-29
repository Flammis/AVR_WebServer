

#ifndef _TEMP_H
#define _TEMP_H

#include <stdint.h>

#define TEMPERATURE_UPDATE_MS 1000

void temperature_initialize(void);

struct temperature_t
{
  int16_t temp_integer;
  uint8_t temp_decimal;
};
const struct temperature_t* get_temperature(void);

#endif