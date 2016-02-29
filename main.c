
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <string.h>
#include <stdio.h>
#include <lowlevelinit.h>


#include <enc28j60.h>
#include <ethernet.h>
#include <ip.h>
#include <arp.h>
#include <net.h>
#include <tcp.h>
#include <webb_config.h>

/*Timer*/
#include <timer.h>

/*
 Temperature read.
*/
#include <adc.h>
#include <temperature.h>

/*TCP*/
tcp_socket_t socket;


/*DEBUG*/
#include "debug.h"

/*Web page"*/
#include "webpage.h"
/*
External Clock:
Full Swing Crystal Oscillator 16 Mhz
Maximum startup time.
|CKSEL3|CKSEL2|CKSEL1|CKSEL0|: 0111
|SUT1|SUT0|: 11
*/

static void watchdog_init(void);
static void httpd_socket_callback(tcp_socket_t socket,enum tcp_event event);
static uint8_t httpd_start(void);

static const ethernet_address my_mac = MAC_ADDRESS;
static uint8_t int28j60 = 0;

/*
  Initalize watchdog 2 seconds reset.
*/
void watchdog_init(void)
{
  wdt_enable(WDTO_2S);
}



/*****Interrupt from ENC28J60**************/
ISR(INT0_vect)
{
  int28j60 = 1;
}

/*****Interrupt from Timer1 Compare**************/

/*
100 Hz clock.
10 ms per tick.
*/
ISR (TIMER1_COMPA_vect)
{
  timer_tick();
}

int main (void)
{
  //watchdog_init();
  InitIo();
  Timer1Init();
  ExternIntInit();
  adc_init();
  
  //inialize software timer (timer.c)
  timer_init();

  //initalize uart 
  uart_init(UART_BAUD_SELECT(BAUDRATE, F_CPU));
  _delay_ms(10);


  //wdt_reset();

  sei(); // enable global interrupt
  
  //initialize enc28j60
  Enc28j60Init((uint8_t*)&my_mac);
  
  //wdt_disable();
  _delay_ms(2000);
  // wdt_enable(WDTO_2S);
  
  InitPhy();
  _delay_ms(10);
  ethernet_init(&my_mac);
	ip_init(0,0,0); //Already set
	arp_init();
	tcp_init();
  
  //wdt_reset();
  
  _delay_ms(10);
  if(httpd_start()){
    DBG_STATIC("Successfully initialized HTTP socket."); 
  } else {
    DBG_STATIC("FAILURE to initialize HTTP socket.");     
  }
  
  /*Temperature initialzie*/
  temperature_initialize();
  
  while (1)
  {
    // wdt_reset();
    if(int28j60){
      while(handle_ethernet_packet());
    }
  }
}

uint8_t httpd_start(void)
{
  socket = tcp_socket_alloc(httpd_socket_callback);
  
  if(socket < 0){
    return 0;
  }
  if(!tcp_listen(socket, WEBB_PORT)){
    return 0;
  }
  return 1;
}

void httpd_socket_callback(tcp_socket_t socket,enum tcp_event event)
{
	switch(event)
	{
	case tcp_event_connection_incoming:
  {
    DBG_STATIC("tcp_event_connection_incoming");
    break;
  }
	case tcp_event_connection_established:
  {
    DBG_STATIC("tcp_event_connection_established");
    break;
  }
	case tcp_event_data_received:
  {
    DBG_STATIC("tcp_event_data_received");
    uint16_t len;
    const uint8_t* msg = tcp_read(socket, &len);
    DBG_DYNAMIC(msg);
    
    char buffer[40];
    sprintf(buffer, "Data length: %" PRIu16, len);
    DBG_DYNAMIC(buffer);
    
    if(len > 0){
      if (strncmp("GET ",(char *)msg, 4) != 0){
        tcp_write_p(socket, (const uint8_t *)PSTR("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>200 OK</h1>"));
      } else {
        tcp_write_p(socket, (const uint8_t *)PSTR("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"));     
        tcp_write_p(socket, (const uint8_t *)WEB_PAGE_1);
        const struct temperature_t* temperature = get_temperature();
        char tempbuff[10];
        sprintf(tempbuff, "%" PRId16 ".%" PRIu8, temperature->temp_integer, temperature->temp_decimal);
        tcp_write(socket, (const uint8_t *)tempbuff);
        tcp_write_p(socket, (const uint8_t *)WEB_PAGE_2);
        // tcp_write_p(socket, (const uint8_t *)WEB_PAGE_2);
      }
    } else {
      DBG_STATIC("No data received");
      return;
    }
    break;
  }
	case tcp_event_connection_closing:
  {
    DBG_STATIC("tcp_event_connection_closing");
    break;
  }
	default:
	break;
	}
}




