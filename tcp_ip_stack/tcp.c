/*
 * Copyright (c) 2012 by Paweł Lebioda <pawel.lebioda89@gmail.com>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <tcp.h>
#include <arp.h>

#include <string.h>
#include <stdint.h>
#include <avr/pgmspace.h>

#include "../debug.h"

#include <timer.h>

/* TCP Flags:
* URG:	Urgent Pointer field significant
* ACK:	Acknowledgment field significant
* PSH:	Push Function
* RST:	Reset the connection
* SYN:	Synchronize sequence numbers
* FIN:	No more data from sender
*/
#define TCP_FLAG_URG	0x20
#define TCP_FLAG_ACK	0x10
#define TCP_FLAG_PSH	0x08
#define TCP_FLAG_RST	0x04
#define TCP_FLAG_SYN	0x02
#define TCP_FLAG_FIN	0x01

/* TCP Options
* Kind     Length    Meaning
* ----     ------    -------
*  0         -       End of option list.
*  1         -       No-Operation.
*  2         4       Maximum Segment Size.
*  3         3       Window Scale (RFC 1323)
*  4         2       Selective Acknowledgement
*  8        10       Timestamps (RFC 1323)
*/
#define TCP_OPT_EOL		0
#define TCP_OPT_NOP		1
#define TCP_OPT_MSS		2
#define TCP_OPT_WS		3
#define TCP_OPT_SACK		4
#define TCP_OPT_TS		8

#define TCP_OPT_LENGTH_MSS 	4
#define TCP_OPT_LENGTH_WS	3
#define TCP_OPT_LENGTH_SACK	2
#define TCP_OPT_LENGTH_TS	10



/* TCP Header
*    0                   1                   2                   3
*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   |          Source Port          |       Destination Port        |
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   |                        Sequence Number                        |
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   |                    Acknowledgment Number                      |
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   |  Data |           |U|A|P|R|S|F|                               |
*   | Offset| Reserved  |R|C|S|S|Y|I|            Window             |
*   |       |           |G|K|H|T|N|N|                               |
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   |           Checksum            |         Urgent Pointer        |
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   |                    Options                    |    Padding    |
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   |                             data                              |
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

struct tcp_header
{
	uint16_t port_source;
	uint16_t port_destination;
	uint32_t seq;
	uint32_t ack;
	uint8_t offset;
	uint8_t flags;
	uint16_t window;
	uint16_t checksum;
	uint16_t urgent;
};

enum tcp_state
{
	tcp_state_unused = 0,
	tcp_state_listen,
	tcp_state_syn_sent,
	tcp_state_syn_received,
	tcp_state_established,
	tcp_state_fin_wait_1,
	tcp_state_fin_wait_2,
	tcp_state_close_wait,
	tcp_state_closing,
	tcp_state_last_ack,
	tcp_state_time_wait,
	tcp_state_closed,
	tcp_state_not_accepted,
	tcp_state_accepted,
	tcp_state_start_close
};

/* Transmission Control Block */
struct tcp_tcb
{
	enum tcp_state state;
	tcp_socket_callback callback;
	uint16_t port_local;
	uint16_t port_remote;
	ip_address ip_remote;
	uint32_t ack;
	uint32_t seq;
	uint16_t seq_next;
	uint16_t window;
	uint16_t mss;
	int8_t rtx;
  uint8_t* RxData;
  uint16_t RxLength;
  uint8_t* TxData;
  uint16_t TxLength;
	timer_t timer;
};



static struct tcp_tcb tcp_tcbs[TCP_MAX_SOCKETS];

#define FOREACH_TCB(tcb) for(tcb = &tcp_tcbs[0] ; tcb < &tcp_tcbs[TCP_MAX_SOCKETS] ; tcb++)

static void tcp_print_packet(const struct tcp_header * tcp, uint16_t length);
static uint8_t 	tcp_send_packet(struct tcp_tcb * tcb,uint8_t flags,uint8_t send_data);
static uint8_t 	tcp_state_machine(struct tcp_tcb * tcb,const ip_address * ip_remote,const struct tcp_header * tcp,uint16_t length);
static uint8_t 	tcp_send_rst(const ip_address * ip_remote,const struct tcp_header * tcp,uint16_t length);
static uint8_t 	tcp_get_options(struct tcp_tcb * tcb,const struct tcp_header * tcp,uint16_t length);
static uint16_t tcp_get_checksum(const ip_address * ip_remote,const struct tcp_header * tcp,uint16_t length);
static tcp_socket_t tcp_get_socket_num(struct tcp_tcb * tcb);  
static uint8_t tcp_socket_valid(tcp_socket_t socket);
static uint8_t tcp_tcb_valid(struct tcp_tcb * tcb);
static uint8_t tcp_free_port(uint16_t port);
static void tcp_tcb_free(struct tcp_tcb * tcb);
static void tcp_timeout(timer_t timer,void * arg);

  
static void tcp_print_packet(const struct tcp_header * tcp, uint16_t length){
  DBG_STATIC("Printing TCP Packet:");
  char buffer[80];
  sprintf(buffer,"  port src: %" PRIu16 "port dst: %" PRIu16, ntoh16(tcp->port_source), ntoh16(tcp->port_destination));
  DBG_DYNAMIC(buffer);
  
  memset(buffer,0,80);
  sprintf(buffer,"  seq: %" PRIu32  "ack: %" PRIu32 , ntoh32(tcp->seq), ntoh32(tcp->ack));
  DBG_DYNAMIC(buffer);
  
  memset(buffer,0,80);
  sprintf(buffer, "  SYN=%x ACK=%x FIN=%x RST=%x", (tcp->flags & TCP_FLAG_SYN) != 0, (tcp->flags & TCP_FLAG_ACK) != 0, (tcp->flags & TCP_FLAG_FIN) != 0, (tcp->flags & TCP_FLAG_RST) != 0);
  DBG_DYNAMIC(buffer);
  
  uint8_t data_offset = (tcp->offset>>4)<<2;
  uint16_t data_length = length - data_offset;
  memset(buffer,0,80);
  sprintf(buffer, "  Packet length: %" PRIu16, data_length);
  DBG_DYNAMIC(buffer); 
}  
  
uint8_t tcp_init(void)
{
  memset(tcp_tcbs,0,sizeof(tcp_tcbs));
  return 1;
}

tcp_socket_t tcp_socket_alloc(tcp_socket_callback callback)
{
  struct tcp_tcb * tcb;
  tcp_socket_t socket_num = -1;
  FOREACH_TCB(tcb)
  {
    if(tcb->state != tcp_state_unused)
      continue;
    socket_num = tcp_get_socket_num(tcb);
    if(socket_num < 0)
      continue;
    tcb->state = tcp_state_closed;
    tcb->callback = callback;
    break;
  }
  return socket_num;
}

uint8_t tcp_listen(tcp_socket_t socket,uint16_t port)
{
  if(!tcp_socket_valid(socket))
    return 0;
  if(!tcp_free_port(port))
    return 0;
  struct tcp_tcb * tcb = &tcp_tcbs[socket];
  if(tcb->state != tcp_state_closed)
    return 0;
  tcb->port_local = port;
  tcb->state = tcp_state_listen;
  tcb->timer = timer_alloc(tcp_timeout, TCP_TIMEOUT_MS);
  timer_set_arg(tcb->timer,(void*)tcb);
  return 1;
}

uint8_t tcp_handle_packet(const ip_address * ip_remote,const struct tcp_header * tcp,uint16_t length)
{
  //tcp_print_packet(tcp, length);
  if(length < sizeof(struct tcp_header))
    return 0;
  if(ntoh16(tcp->checksum) != tcp_get_checksum(ip_remote,tcp,length))
    return 0;
  struct tcp_tcb * tcb;
  struct tcp_tcb * tcb_selected = 0;
  FOREACH_TCB(tcb)
  { 
    if(tcb->state == tcp_state_unused)
      continue;
    if(tcb->port_local != ntoh16(tcp->port_destination))
      continue;
    if(tcb->state == tcp_state_listen || tcb->state == tcp_state_closed)
    {
      tcb_selected = tcb;
      continue;
    }
    if(tcb->port_remote != ntoh16(tcp->port_source))
      continue;
    if(memcmp(&tcb->ip_remote,ip_remote,sizeof(ip_address)))
      continue;
    tcb_selected = tcb;
    break;
  }
  if(tcb_selected != 0)
    return tcp_state_machine(tcb_selected,ip_remote,tcp,length); 
  tcp_send_rst(ip_remote,tcp,length);
  return 0;
}

uint8_t tcp_state_machine(struct tcp_tcb * tcb,const ip_address * ip_remote,const struct tcp_header * tcp,uint16_t length)
{
	if(!tcb || !ip_remote || !tcp || length < sizeof(struct tcp_header))
		return 0;
	tcp_get_options(tcb,tcp,length);
	tcp_socket_t socket = tcp_get_socket_num(tcb);
	if(socket < 0)
		return 0;
  
  /*Reset timer*/
  timer_reset(tcb->timer);
  
  if(tcp->flags & TCP_FLAG_SYN){
    /* set remote ip address */
    memcpy(tcb->ip_remote,ip_remote,sizeof(ip_address));
    tcb->port_remote = ntoh16(tcp->port_source);
    /* set mss */
    tcb->mss = tcb->mss;
    /* Send information to user about incoming new connection */
    tcb->callback(socket,tcp_event_connection_incoming);
    /* User accepted the connection so we have to establish a connection */
    /* set ack */
    tcb->ack = ntoh32(tcp->seq) + 1;
    /* send SYN, ACK segment */
    tcp_send_packet(tcb,TCP_FLAG_SYN|TCP_FLAG_ACK,0);
    
    tcb->state = tcp_state_syn_received;
    return 1;
  }
  
  //Should receive seq which is previous ack number 
	if(ntoh32(tcp->seq) != tcb->ack)
	{
    return 0;
	}
  
  /* check ACK field */
  /* if ACK bit is off drop the segment and return */
  if(!(tcp->flags & TCP_FLAG_ACK))
  return 0;
  
  tcb->seq = ntoh32(tcp->ack);
  //Either an syn ack or data
  uint8_t data_offset = (tcp->offset>>4)<<2;
  uint16_t data_length = length - data_offset;
  //Connection established
  if(data_length == 0){
    if(tcp->flags & TCP_FLAG_FIN){
      tcb->ack = ntoh32(tcp->seq) + 1;
      tcp_send_packet(tcb,TCP_FLAG_FIN|TCP_FLAG_ACK,0);
      tcb->callback(socket,tcp_event_connection_closing);
      tcb->state = tcp_state_listen;
      timer_stop(tcb->timer);
    } else {
      tcb->callback(socket,tcp_event_connection_established);
      tcb->state = tcp_state_established;
    }
  } else {
    tcb->ack = ntoh32(tcp->seq) + data_length;
    // if(data_length> TCB_RX_BUFFERSIZE){
      // data_length = TCB_RX_BUFFERSIZE;
    // }
    tcb->RxData = (const uint8_t*)tcp + data_offset;
    // memcpy(tcb->RxData, (const uint8_t*)tcp + data_offset,data_length);
    tcb->RxLength = data_length;
    tcb->callback(socket,tcp_event_data_received);
    //Send ack
    tcp_send_packet(tcb,TCP_FLAG_ACK,0);
    if(tcb->TxLength > 0){
      //Send ACK and the reply data and FIN
      tcp_send_packet(tcb, TCP_FLAG_ACK|TCP_FLAG_FIN|TCP_FLAG_PSH, 1);
    }
    tcb->state = tcp_state_listen;
    timer_stop(tcb->timer);
  }
  return 1;

}

void tcp_timeout(timer_t timer,void * arg)
{
	DBG_STATIC("tcp_timeout!");
  
	if(!arg)
		return;
	struct tcp_tcb * tcb = (struct tcp_tcb*)arg;
	tcp_socket_t socket = tcp_get_socket_num(tcb);
	if(socket < 0)
		return;
	if(timer != tcb->timer)
		return;
  tcb->state = tcp_state_listen;
}


uint8_t tcp_send_packet(struct tcp_tcb * tcb,uint8_t flags,uint8_t send_data)
{
	if(!tcb)
		return 0;
	struct tcp_header * tcp = (struct tcp_header*)ip_get_buffer();
 
	memset(tcp,0,sizeof(struct tcp_header));
	/* set destination port */
	tcp->port_destination = hton16(tcb->port_remote);
	/* set source port */
	tcp->port_source = hton16(tcb->port_local);
	/* not using urgent */
	tcp->urgent = HTON16(0x0000);
	/* set acknowledgment number */
	tcp->ack = hton32(tcb->ack);
	/* set flags */
	tcp->flags = flags;
	/* set window to buffer free space length */
	tcp->window = hton16(TCB_RX_BUFFERSIZE);
	uint16_t packet_header_len = sizeof(struct tcp_header);
	uint16_t max_packet_size = tcp_get_buffer_size();
	uint8_t * data_ptr = (uint8_t*)tcp + sizeof(struct tcp_header);
	/* if SYN packet send maximum segment size in options field */
	if(tcp->flags & TCP_FLAG_SYN)
	{
    *((uint32_t*)data_ptr) = HTON32(((uint32_t)TCP_OPT_MSS<<24)|((uint32_t)TCP_OPT_LENGTH_MSS<<16)|(uint32_t)TCP_MSS);
    data_ptr += sizeof(uint32_t);
    packet_header_len += sizeof(uint32_t);
    max_packet_size -= sizeof(uint32_t);
	}
	
	tcp->offset = (packet_header_len>>2)<<4;
	uint8_t packet_sent = 1;	
	uint16_t packet_total_len;
	uint16_t data_length = 0;
  if(send_data)
  {
    data_length = tcb->TxLength;
    memcpy(data_ptr, (const uint8_t*)tcb->TxData, data_length);
    tcb->TxLength = 0;
  }
  packet_total_len = data_length + packet_header_len;
  tcp->flags = flags;
  
  tcp->seq = hton32(tcb->seq);
  tcp->checksum = hton16(tcp_get_checksum((const ip_address*)&tcb->ip_remote,tcp,packet_total_len));
  
  DBG_STATIC("Trasmitting TCP:");
  //tcp_print_packet(tcp, packet_total_len);
  
  packet_sent = ip_send_packet((const ip_address*)&tcb->ip_remote,IP_PROTOCOL_TCP,packet_total_len);
	return packet_sent;
}

uint8_t tcp_send_rst(const ip_address * ip_remote,const struct tcp_header * tcp_rcv,uint16_t length)
{
  DBG_STATIC("Reseting TCP.");
	/* Behaviour due to RFC-793 [Page 36] point 1.*/
	if(!tcp_rcv)
		return 0;
	if(tcp_rcv->flags & TCP_FLAG_RST)
		return 0;
	struct tcp_header * tcp_rst = (struct tcp_header*)ip_get_buffer();
	memset(tcp_rst,0,sizeof(struct tcp_header));
	tcp_rst->port_destination = tcp_rcv->port_source;
	tcp_rst->port_source = tcp_rcv->port_destination;
	/* the number of 32-bit owrds shifted by 4 positions due to reserved bits*/
	tcp_rst->offset = (sizeof(struct tcp_header)/4)<<4;
	
	if(tcp_rcv->flags & TCP_FLAG_ACK)
	{
		/*If the incoming segment has an ACK field, the reset takes its
			sequence number from the ACK field of the segment..*/
		tcp_rst->seq = tcp_rcv->ack;
		tcp_rst->flags = TCP_FLAG_RST;
	}
	else
	{
		/*..otherwise 
		the reset has sequence number zero and the ACK field is set to the sum
		of the sequence number and segment length of the incoming segment*/
		uint32_t ack = ntoh32(tcp_rcv->seq) + length - ((tcp_rcv->offset>>4)<<2);

		tcp_rst->flags = TCP_FLAG_RST | TCP_FLAG_ACK;
		tcp_rst->ack = hton32(ack);
	}
	tcp_rst->checksum = hton16(tcp_get_checksum(ip_remote,tcp_rst,sizeof(struct tcp_header)));
	return ip_send_packet(ip_remote,IP_PROTOCOL_TCP,sizeof(struct tcp_header));
}

tcp_socket_t tcp_get_socket_num(struct tcp_tcb * tcb)
{
  tcp_socket_t socket_num = (tcp_socket_t)(((uint16_t)tcb - (uint16_t)tcp_tcbs)/sizeof(struct tcp_tcb));
  if(tcp_socket_valid(socket_num))
    return socket_num;
  return -1;
}

uint8_t tcp_free_port(uint16_t port)
{
  struct tcp_tcb * tcb;
  FOREACH_TCB(tcb)
  {
    if(tcb->state != tcp_state_unused && tcb->port_local == port)
      return 0;
  }	
  return 1;
}

uint8_t tcp_socket_free(tcp_socket_t socket)
{
  if(!tcp_socket_valid(socket))
    return 0;
  struct tcp_tcb * tcb = &tcp_tcbs[socket];
  tcp_tcb_free(tcb);
  return 1;
}

void tcp_tcb_free(struct tcp_tcb * tcb)
{
	if(!tcp_tcb_valid(tcb))
		return;
	tcb->state = tcp_state_unused;
  timer_free(tcb->timer);
	memset(tcb,0,sizeof(struct tcp_tcb));
}

uint8_t tcp_socket_valid(tcp_socket_t socket)
{
		return (socket >=0 && socket < TCP_MAX_SOCKETS);
}

uint8_t tcp_tcb_valid(struct tcp_tcb * tcb)
{
		return (tcb && tcb >= &tcp_tcbs[0] && tcb < &tcp_tcbs[TCP_MAX_SOCKETS]);
}

uint8_t tcp_get_options(struct tcp_tcb * tcb,const struct tcp_header * tcp,uint16_t length)
{
		if(!tcb || !tcp || length < sizeof(struct tcp_header))
			return 0;
		uint16_t offset = (tcp->offset>>4)<<2;
		uint8_t * options = (uint8_t*)tcp + sizeof(struct tcp_header);
		uint8_t * options_end = (uint8_t*)tcp + offset;
		for(;options < options_end;options++)
		{
			if(*options == TCP_OPT_EOL)
			{
					/* end of options list*/
					break;
			}
			else if (*options == TCP_OPT_NOP)
			{
					/* no operation */
			}
			else if(*options == TCP_OPT_MSS && *(options+1) == TCP_OPT_LENGTH_MSS)
			{
				/* maximum segment size */
				options+=2;
				tcb->mss = ntoh16(*((uint16_t*)options));
				options++;
			}
			else
			{
					options += *(options+1);
			}
		}
		return 1;
}

const uint8_t* tcp_read(tcp_socket_t socket, uint16_t* len)
{
  struct tcp_tcb * tcb = &tcp_tcbs[socket];
  if(!tcp_socket_valid(socket)){
    *len = 0;
  } else {
    *len = tcb->RxLength;
  }
  return (const uint8_t*)tcb->RxData;
}

uint16_t tcp_write(tcp_socket_t socket, const uint8_t * data)
{
  if(!tcp_socket_valid(socket))
		return -1;
	struct tcp_tcb * tcb = &tcp_tcbs[socket];
  
  struct tcp_header * tcp = (struct tcp_header*)ip_get_buffer();
  uint16_t max_packet_size = tcp_get_buffer_size();
  uint8_t * data_ptr = (uint8_t*)tcp + sizeof(struct tcp_header);
  data_ptr += sizeof(uint32_t);
  max_packet_size -= sizeof(uint32_t);
  
  tcb->TxData = data_ptr;
  
  uint8_t c;
  while ((c = *data)) 
  {
    if(tcb->TxLength < max_packet_size){
      tcb->TxData[tcb->TxLength++] = c;
      data++;
    } else {
      DBG_STATIC("Reply too big.");
      break;
    }
  }
	return tcb->TxLength;
}

uint16_t tcp_write_p(tcp_socket_t socket, const uint8_t * data_p)
{
	if(!tcp_socket_valid(socket))
		return -1;
	struct tcp_tcb * tcb = &tcp_tcbs[socket];
  
  struct tcp_header * tcp = (struct tcp_header*)ip_get_buffer();
  uint16_t max_packet_size = tcp_get_buffer_size();
  uint8_t * data_ptr = (uint8_t*)tcp + sizeof(struct tcp_header);
  data_ptr += sizeof(uint32_t);
  max_packet_size -= sizeof(uint32_t);
  
  tcb->TxData = data_ptr;
  
  uint8_t c;
  while ((c = pgm_read_byte(data_p++))) 
  {
    if(tcb->TxLength < max_packet_size){
      tcb->TxData[tcb->TxLength++] = c;
    } else {
      DBG_STATIC("Reply too big.");
      break;
    }
  }
	return tcb->TxLength;
  
}


uint16_t tcp_get_checksum(const ip_address * ip_remote,const struct tcp_header * tcp,uint16_t length)
{
    /* tcp pseudo header :
             +--------+--------+--------+--------+
             |           Source Address          |
             +--------+--------+--------+--------+
             |         Destination Address       |
             +--------+--------+--------+--------+
             |  zero  |  PTCL  |    TCP Length   |
             +--------+--------+--------+--------+
    */
    /* PTCL + TCP length */
	uint16_t checksum = IP_PROTOCOL_TCP + length;
	/* source/destination ip address */
	checksum = net_get_checksum(checksum,(const uint8_t*)ip_remote,sizeof(ip_address),4);
	/* our ip address */
	checksum = net_get_checksum(checksum,(const uint8_t*)ip_get_addr(),sizeof(ip_address),4);
	
	/* TCP header + data */
	return ~net_get_checksum(checksum,(const uint8_t*)tcp,length,16);
}

