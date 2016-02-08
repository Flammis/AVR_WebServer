
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <stdio.h>
#include "lowlevelinit.h"
#include "uart.h"

#define BAUDRATE      1200

/*
External Clock:
Full Swing Crystal Oscillator 16 Mhz
Maximum startup time.
|CKSEL3|CKSEL2|CKSEL1|CKSEL0|: 0111
|SUT1|SUT0|: 11
*/


int main (void)
{
  char     outbuffer[20];
  
  InitIo();
  Timer1Init();
  ExternIntInit();
  uart_init(UART_BAUD_SELECT(BAUDRATE, F_CPU));
  _delay_ms(10);
  
  
  sei();            // enable global interrupt
  while (1)
  {
    _delay_ms(1000);
    sprintf(outbuffer, "%3u", 1);
    uart_puts("Hej");
  }
}