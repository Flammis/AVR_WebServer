#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <stdio.h>
#include <lowlevelinit.h>
#include <enc28j60.h>
#include <ip_arp_udp_tcp.h>
#include <net.h>
#include "webserver.h"

//**************************************************************************************
/*
   Copyright: 
   This code is modified from code I found for an Arduino system by Guido Socher and the 
   Tux Graphics web server system. Big thanks to both. It is supposed to be open.
   
   SW tools:
   This system is designed to operate on an Atmel ATmega32 micro using AVR Studio version 4. 
   This version supports the Olimex ICE called AVR-JTAG-USB whereas the newer versions do not!

   Notes to hardware:
   The mega32 runs with a 16 MHz crystal.
   A 7-segment display is connected to PORTA. It will display the second counter from the 
   time task. The web page gives you the possibility to count up or down.
   The LAN interface is Olimex ENC28J60-H using Microchip IC with SPI interface. This 
   version of the code uses the interrupt from the LAN module.

   Notes to code:
   This version gives examples of HTML strings based in ROM aswell as in RAM. It uses 
   interrupt from the LAN module to ease the integration into an existing scheduler. It 
   enables you to decode incomming commands  - in this case only single character commands.
   S-optimization is used to keep the size of the code down - switch it off for debugging.
   Using sprinft like me will cost you 1.5 KB of code space.

   Contact: 
   Erik Rasmussen, Lecturer, ITET department, Copenhagen School of Design and Technology, 
   KEA, Copenhagen, Denmark
   mail at erikrasmussen.dk
*/
//**************************************************************************************

// please modify the following two lines. mac and ip have to be unique
// in your local area network. You can not have the same numbers in
// two devices:
static uint8_t mymac[6] = {0x19,0x21,0x68,0x00,0x00,0x29}; 
//static uint8_t myip[4] = {192,168,0,29};
//169.254.222.183
static uint8_t myip[4] = {169,254,222,182};
static uint16_t mywwwport = 80; // listen port for tcp/www (max range 1-254)

#define BUFFER_SIZE 500
static uint8_t buf[BUFFER_SIZE+1];
bool int28j60 = false;
uint16_t isrCount = 0;


//**************************************************************************************
uint16_t PrintWebpage (uint8_t *buf)
{
   uint16_t plen;
   char isrString[8];

   plen = Fill_tcp_data_p(buf, 0, PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n"));
   plen = Fill_tcp_data_p(buf, plen, PSTR("<center><h1>Welcome to AVR webserver<hr><button>Click me!</button>"));
   return plen;
}


/*****Interrupt from ENC28J60**************/
ISR(INT0_vect)
{
   int28j60 = true;   
   isrCount++;
}


//*****************************************************************************
void LanTask (void)
{
   uint16_t plen, dat_p = 0;
   bool send;
   
   while (1)
   {
      plen = Enc28j60PacketReceive(BUFFER_SIZE, buf);
         /*plen will unequal to zero if there is a valid packet (without crc error) */
      if (plen == 0) return;

      // arp is broadcast if unknown but a host may also verify the mac address by sending it to a unicast address.
      if (Eth_type_is_arp_and_my_ip(buf, plen))
      {
         Make_arp_answer_from_request(buf);
         continue;
      }
      // check if the ip packet is for us:
      if (Eth_type_is_ip_and_my_ip(buf, plen) == 0)
      {
         continue;
      }
      // ping
      if((buf[IP_PROTO_P] == IP_PROTO_ICMP_V) && (buf[ICMP_TYPE_P] == ICMP_TYPE_ECHOREQUEST_V))
      {
         Make_echo_reply_from_request(buf, plen);
         continue;
      }

      // tcp port www start, compare only the lower byte
      if ((buf[IP_PROTO_P] == IP_PROTO_TCP_V) && (buf[TCP_DST_PORT_H_P] == 0) && (buf[TCP_DST_PORT_L_P] == mywwwport))
      {
         if (buf[TCP_FLAGS_P] & TCP_FLAGS_SYN_V)
         {
            Make_tcp_synack_from_syn(buf); // make_tcp_synack_from_syn does already send the syn,ack
            continue;
         }
         if (buf[TCP_FLAGS_P] & TCP_FLAGS_ACK_V)
         {
            Init_len_info(buf); // init some data structures
            dat_p = Get_tcp_data_pointer();
            if (dat_p == 0)
            { 
               // we can possibly have no data, just ack:
               if (buf[TCP_FLAGS_P] & TCP_FLAGS_FIN_V)
               {
                  Make_tcp_ack_from_any(buf);
               }
               continue;
            }
         }
         send = false;
         if (strncmp("GET ",(char *) & (buf[dat_p]), 4) != 0)
         {
            // head, post and other methods for possible status codes see:
            // http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
            plen = Fill_tcp_data_p(buf, 0, PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>200 OK</h1>"));
            send = true;
         }

	      if (!send)
         {
          plen = PrintWebpage(buf);
          send = true;
         }

         Make_tcp_ack_from_any(buf); // send ack for http get
         Make_tcp_ack_with_data(buf, plen); // send data       
      }
   }
}

//*****************************************************************************
int main (void)
{
   InitIo();
   Timer1Init();
   ExternIntInit();
   Enc28j60Init(mymac);    //initialize enc28j60
   _delay_ms(10);
   InitPhy();
   Init_ip_arp_udp_tcp(mymac,myip,80);  //init the ethernet/ip layer
   sei();            // enable global interrupt
   while (1)
   {
      // if (int28j60)
      // {
         // int28j60 = false;
         // LanTask();
      // }
      LanTask();
      _delay_ms(1000);
      // insert your own tasks here !!
   }
}


