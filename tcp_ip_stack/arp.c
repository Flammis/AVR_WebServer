
#include <arp.h>

struct arp_header
{
  uint16_t 		hardware_type;
  uint16_t 		protocol_type;
  uint8_t 		hardware_addr_len;
  uint8_t 		protocol_addr_len;
  uint16_t 		operation_code;
  ethernet_address 	sender_hardware_addr;
  ip_address 		sender_protocol_addr;
  ethernet_address 	target_hardware_addr;
  ip_address 		target_protocol_addr;
};



uint8_t arp_handle_packet(struct arp_header * header, uint16_t packet_length)
{
	/* Check packet length */
	if(packet_length < sizeof(struct arp_header))
		return 0;
	/* Check hardware address size */
	if(header->hardware_addr_len != ARP_HW_ADDR_SIZE_ETHERNET)
		return 0;
	/* Check protocol address size */
	if(header->protocol_addr_len != ARP_PROTO_ADDR_SIZE_IP)
		return 0;
	/* Check hardware address type */
	if(header->hardware_type != HTON16(ARP_HW_ADDR_TYPE_ETHERNET))
		return 0;
	/* Check protocol address type */
	if(header->protocol_type != HTON16(ARP_PROTO_ADDR_TYPE_IP))
		return 0;		
	/* Check whether target protocol address is our's */
	if(memcmp(header->target_protocol_addr,ip_get_addr(),sizeof(ip_address)))
		return 0
  if(header->operation_code == HTON16(ARP_OPERATION_REQUEST))
    return arp_send_reply(header);
  return 1;
}

uint8_t arp_send_reply(const struct arp_header * header)
{
	struct arp_header * arp_reply = (struct arp_header*)ethernet_get_buffer();
	
	/* set hardware address length */
	arp_reply->hardware_addr_len = header->hardware_addr_len;
	/* set hardware address type */
	arp_reply->hardware_type = header->hardware_type;
	/* set protocol address length */
	arp_reply->protocol_addr_len = header->protocol_addr_len;
	/* set protocol address type */
	arp_reply->protocol_type = header->protocol_type;
	/* set operation code to echo reply */
	arp_reply->operation_code = HTON16(ARP_OPERATION_REPLY);
	/* set target's hardware address */
	memcpy(&arp_reply->target_hardware_addr,&header->sender_hardware_addr,sizeof(ethernet_address));
	/* set target's protocol address */
	memcpy(&arp_reply->target_protocol_addr,&header->sender_protocol_addr,sizeof(ip_address));
	/* set our protocol address */
	memcpy(&arp_reply->sender_protocol_addr,ip_get_addr(),sizeof(ip_address));
	/* set our hardware address */
	memcpy(&arp_reply->sender_hardware_addr,ethernet_get_mac(),sizeof(ethernet_address));
	
	return ethernet_send_packet(&arp_reply->target_hardware_addr,ETHERNET_TYPE_ARP,sizeof(struct arp_header));
}
