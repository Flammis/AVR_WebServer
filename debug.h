

#ifndef _DEBUG_H
  #define _DEBUG_H
  #include <avr/io.h>
  #include <uart.h>
  #define BAUDRATE      9600

  #define DBG_STATIC(x){ \
      uart_puts((x)); \
      uart_puts("\n"); \
    }
    
    
  #define DBG_DYNAMIC(x) { \
    uart_puts((const char*)x); \
    uart_puts("\n"); \
  }
  
  
  
#endif