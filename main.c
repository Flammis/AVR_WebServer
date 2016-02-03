
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
#include <ethernet.h>
#include "webb_config.h"


/*
External Clock:
Full Swing Crystal Oscillator 16 Mhz
Maximum startup time.
|CKSEL3|CKSEL2|CKSEL1|CKSEL0|: 0111
|SUT1|SUT0|: 11
*/

/*
  Define MAC and IP in makefile.
*/
static const ethernet_address mac_address = MAC_ADDRESS;
static const uint8_t ip_address[4] = IP_ADDRESS;
static const uint16_t wwwport = WEBB_PORT;

void LanTask (void)
{
  uint16_t plen, dat_p = 0;
  bool send;

  while (1)
  {
    handle_ethernet_packet();
    
    
  }
  
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

int main (void)
{
  InitIo();
  Timer1Init();
  ExternIntInit();
  Enc28j60Init(mac_address);    //initialize enc28j60
  _delay_ms(10);
  InitPhy();
  //Init_ip_arp_udp_tcp(mac_address, ip_address, wwwport);  //init the ethernet/ip layer
  
  ethernet_init(&mac_address);
	ip_init(0,0,0); //Already set
	// arp_init();
	//tcp_init();
  
  
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