
#ifndef _ETHERNET_H
#define _ETHERNET_H

  #include <net2.h>

  typedef uint8_t ethernet_address[6];

  struct ethernet_header;
  struct ethernet_stats;

  #define ETHERNET_ADDR_BROADCAST	0
  
  extern uint8_t ethernet_tx_buffer[];
  
  void ethernet_init(const ethernet_address * mac);
  uint16_t handle_ethernet_packet(uint16_t maxlen, uint8_t* buffer);
  uint8_t ethernet_send_packet(ethernet_address * dst,uint16_t type,uint16_t len);

  #define ethernet_get_buffer()	(&ethernet_tx_buffer[NET_HEADER_SIZE_ETHERNET])
  #define ethernet_get_broadcast()
  #define ethernet_get_buffer_size() (ETHERNET_MAX_PACKET_SIZE -NET_HEADER_SIZE_ETHERNET)

  
#endif
