
#include <arp.h>
#include <string.h>

#include "../debug.h"

#define ARP_TABLE_SIZE			2


enum arp_status
{
  arp_unused = 0,
  arp_used,
  arp_waiting
};

struct arp_table_entry
{
	enum arp_status			status;
	ip_address 		ip_addr;
	ethernet_address 	ethernet_addr;
};

static struct arp_table_entry 	arp_table[ARP_TABLE_SIZE];
#define FOREACH_ARP_ENTRY(entry) for(entry = &arp_table[0] ; entry < &arp_table[ARP_TABLE_SIZE] ; entry++)
static uint8_t rotating_arp;


uint8_t arp_send_reply(const struct arp_header * header);
uint8_t arp_send_request(const ip_address * ip_addr);

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


uint8_t arp_init(void)
{
	/* Clear arp table */
	memset(arp_table, 0 ,sizeof(arp_table));
  return 1;
}


uint8_t arp_handle_packet(struct arp_header * header, uint16_t packet_length)
{
	/* Check packet length */
	if(packet_length < sizeof(struct arp_header))
		return 0;
  //DBG_STATIC("ARP PASSED PHASE 1!");
	/* Check hardware address size */
	if(header->hardware_addr_len != ARP_HW_ADDR_SIZE_ETHERNET)
		return 0;
  //DBG_STATIC("ARP PASSED PHASE 2!");
	/* Check protocol address size */
	if(header->protocol_addr_len != ARP_PROTO_ADDR_SIZE_IP)
		return 0;
  //DBG_STATIC("ARP PASSED PHASE 3!");
	/* Check hardware address type */
	if(header->hardware_type != HTON16(ARP_HW_ADDR_TYPE_ETHERNET))
		return 0;
  //DBG_STATIC("ARP PASSED PHASE 4!");
	/* Check protocol address type */
	if(header->protocol_type != HTON16(ARP_PROTO_ADDR_TYPE_IP))
		return 0;
  //DBG_STATIC("ARP PASSED PHASE 5!");
	/* Check whether target protocol address is our's */
	if(memcmp(header->target_protocol_addr,ip_get_addr(),sizeof(ip_address)))
		return 0;
  //DBG_STATIC("ARP PASSED PHASE 6!");
	/* Parse operation code of packet */
	if(header->operation_code != HTON16(ARP_OPERATION_REQUEST) && header->operation_code != HTON16(ARP_OPERATION_REPLY))
		return 0;
  //DBG_STATIC("ARP PASSED PHASE 7!");
	arp_table_insert((const ip_address*)&header->sender_protocol_addr,(const ethernet_address*)&header->sender_hardware_addr);
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

void arp_table_insert(const ip_address * ip_addr,const ethernet_address * ethernet_addr)
{
	struct arp_table_entry * entry;
	struct arp_table_entry * match_entry = 0;
	
  FOREACH_ARP_ENTRY(entry)
	{
		/* Check if there is already entry in arp table with specified ip addr */
		if(!memcmp(ip_addr,&entry->ip_addr,sizeof(ip_address)))
		{
      match_entry = entry;
      break;
		}
    if(entry->status == arp_unused){
      match_entry = entry;
      continue;
    }
	}
  
	/* If all are occupied overwrite the oldest*/
	if(match_entry == 0)
	{
    match_entry = &arp_table[rotating_arp];
    rotating_arp++;
    
    if(rotating_arp >= ARP_TABLE_SIZE)
      rotating_arp = 0;
  }
  
  match_entry->status = arp_used;	
  /* copy ip address */
  memcpy(&match_entry->ip_addr,ip_addr,sizeof(ip_address));
  /* copy mac address */
  memcpy(&match_entry->ethernet_addr,ethernet_addr,sizeof(ethernet_address));

}

uint8_t arp_get_mac(const ip_address * ip_addr,ethernet_address * ethernet_addr)
{
	if(ip_addr == 0)
		return 0;
	struct arp_table_entry * entry;
	struct arp_table_entry * empty = 0;
	FOREACH_ARP_ENTRY(entry)
	{
		if(entry->status == arp_unused)
		{
			empty = entry;
			continue;
		}
		if(!memcmp(&entry->ip_addr,ip_addr,sizeof(ip_address)))
		{
			switch(entry->status)
			{
				/* There is ethernet address in arp table */
				case arp_used:
					if(ethernet_addr)
						memcpy(ethernet_addr,&entry->ethernet_addr,sizeof(ethernet_address));
					return 1;
				/* There is ip address but waiting for reply*/
				case arp_waiting:
					return 0;
				default:
					continue;
			}
		}
	}
	if(empty != 0)
	{
		memcpy(&empty->ip_addr,ip_addr,sizeof(ip_address));
		empty->status = arp_waiting;
	}
	arp_send_request(ip_addr);
	return 0;
}

uint8_t arp_send_request(const ip_address * ip_addr)
{
	struct arp_header * arp_request = (struct arp_header*)ethernet_get_buffer();
	/* Set protocol and hardware addresses type and length */
	arp_request->hardware_addr_len = ARP_HW_ADDR_SIZE_ETHERNET;
	arp_request->protocol_addr_len = ARP_PROTO_ADDR_SIZE_IP;
	arp_request->hardware_type = HTON16(ARP_HW_ADDR_TYPE_ETHERNET);
	arp_request->protocol_type = HTON16(ARP_PROTO_ADDR_TYPE_IP);
	
	/* Set sedner and target hardware and protocol addresses */
	memset(&arp_request->target_hardware_addr,0,sizeof(ethernet_address));
	memcpy(&arp_request->target_protocol_addr,ip_addr,sizeof(ip_address));
	memcpy(&arp_request->sender_hardware_addr,ethernet_get_mac(),sizeof(ethernet_address));
	memcpy(&arp_request->sender_protocol_addr,ip_get_addr(),sizeof(ip_address));
	
	/* Set operation code */
	arp_request->operation_code = HTON16(ARP_OPERATION_REQUEST);
	/* Send packet */
	return ethernet_send_packet(ETHERNET_ADDR_BROADCAST,ETHERNET_TYPE_ARP,sizeof(struct arp_header));
}




