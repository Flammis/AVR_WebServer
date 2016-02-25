

#ifndef _ADC_H
#define _ADC_H

#include <avr/io.h>
#include <stdint.h>

#define AVCC_REF (1<<REFS0)
#define ADC_PRESCALER_128 (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)
#define ADC_ENABLE (1<<ADEN)
#define ADC_CHANNEL_BITS 0x07

#define CHANNEL_0 0
#define CHANNEL_1 1
#define CHANNEL_2 2
#define CHANNEL_3 3
#define CHANNEL_4 4
#define CHANNEL_5 5
#define CHANNEL_6 6
#define CHANNEL_7 7

void adc_init(void);
uint16_t adc_read(uint8_t ch);

#endif