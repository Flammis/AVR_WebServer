
#ifndef _ARP_H
#define _ARP_H

  #include <ethernet.h>
  #include <ip.h>

  struct arp_header;

  #define ARP_HW_ADDR_TYPE_ETHERNET	0x0001
  #define ARP_HW_ADDR_SIZE_ETHERNET	0x06
  #define ARP_PROTO_ADDR_TYPE_IP		0x0800
  #define ARP_PROTO_ADDR_SIZE_IP		0x04
  #define ARP_OPERATION_REQUEST		0x0001
  #define ARP_OPERATION_REPLY		0x0002
  
  uint8_t arp_init(void);
  uint8_t arp_handle_packet(struct arp_header * header,uint16_t packet_length);
  uint8_t arp_get_mac(const ip_address * ip_addr,ethernet_address * ethernet_addr);
  void arp_table_insert(const ip_address * ip_addr,const ethernet_address * ethernet_addr);
#endif