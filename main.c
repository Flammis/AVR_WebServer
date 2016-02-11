
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
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

/*TCP*/
tcp_socket_t socket;


/*DEBUG*/
#include "debug.h"

/*
External Clock:
Full Swing Crystal Oscillator 16 Mhz
Maximum startup time.
|CKSEL3|CKSEL2|CKSEL1|CKSEL0|: 0111
|SUT1|SUT0|: 11
*/

static void httpd_socket_callback(tcp_socket_t socket,enum tcp_event event);
static uint8_t httpd_start(void);

static const ethernet_address my_mac = MAC_ADDRESS;
static uint8_t int28j60 = 0;

int main (void)
{
  InitIo();
  Timer1Init();
  ExternIntInit();
  timer_init();
  //initalize uart 
  uart_init(UART_BAUD_SELECT(BAUDRATE, F_CPU));
  _delay_ms(10);
  sei();            // enable global interrupt
  
  //initialize enc28j60
  Enc28j60Init((uint8_t*)&my_mac);
  _delay_ms(10);

  InitPhy();
  _delay_ms(10);
  ethernet_init(&my_mac);
	ip_init(0,0,0); //Already set
	arp_init();
	tcp_init();
  
  _delay_ms(10);
  if(httpd_start()){
    DBG_STATIC("Successfully initialized HTTP socket."); 
  } else {
    DBG_STATIC("FAILURE to initialize HTTP socket.");     
  }
  
  while (1)
  {
    if(int28j60){
      while(handle_ethernet_packet());
    }
    //_delay_us(1);
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
        tcp_write_p(socket, (const uint8_t *)PSTR("<center><h1>AVR Webserver</h1><hr><table><thead><tr colspan=\"2\"><th>Server status</th></tr></thead><tbody><tr><td>Temp</td></tr></tbody></table></center>"));
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

/*****Interrupt from ENC28J60**************/
ISR(INT0_vect)
{
  int28j60 = 1;
}

static uint8_t counter;

/*100 Hz clock*/
ISR (TIMER1_COMPA_vect)
{
  counter++; // incr the counter every 10 ms
  if (counter == 100)
  {
    counter = 0;
    timer_tick();
  }
}


