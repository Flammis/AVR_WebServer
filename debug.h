

#ifndef _DEBUG_H
  #define _DEBUG_H
  #include <avr/io.h>
  #include <uart.h>
  #include <string.h>
  #include <stdio.h>
  #include <avr/pgmspace.h>
  
  #define BAUDRATE      9600

  
  
  #define DBG_STATIC(x) {    \
    uart_puts_p(PSTR(x));    \
    uart_puts_p(PSTR("\n")); \
  }
    
  #define DBG_DYNAMIC(x) {     \
    uart_puts((const char*)x); \
    uart_puts_p(PSTR("\n"));   \
  }
  
  
  
#endif