

#ifndef _DEBUG_H
  #define _DEBUG_H
  
  #include <uart.h>
  #define BAUDRATE      1200

  #define DBG_STATIC(x) {  \
      uart_puts(PSTR(x));  \
    }
  #define DBG_DYNAMIC(x) uart_puts(x);
  
#endif