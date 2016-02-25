
#include <adc.h>

#include <avr/io.h>
#include <string.h>
#include <stdint.h>

void adc_init(void)
{
  // AREF = AVCC
  ADMUX = 0x40; //AVCC_REF;

  // ADC Enable and prescaler of 128
  ADCSRA = 0x87;//ADC_ENABLE | ADC_PRESCALER_128;
}

uint16_t adc_read(uint8_t ch)
{
  // Channel can be 0-7
  ch &= 0x07;//ADC_CHANNEL_BITS;
  ADMUX = (ADMUX & 0xF8);// | ch;// (ADMUX & (~ADC_CHANNEL_BITS))|ch;
 
  // start single convertion
  // write ’1′ to ADSC
  ADCSRA |= 0x40;//(1<<ADSC);
 
  //Wait until conversion completes
  // while(ADCSRA & 0x40);//while(ADCSRA & (1<<ADSC));
  //Wait for conversion to complete
  while(!(ADCSRA & (1<<ADIF)));

  //Clear ADIF by writing one to it
  ADCSRA|=(1<<ADIF);

  return (ADC);
}