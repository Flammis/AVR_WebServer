
#include <ip.h>

/**
 * Internet Protocol version 4
 */
#define IP_V4		0x4

#define IP_VIHL_HL_MASK	0xf
/**
 * Internet Protocol Header
 */
struct ip_header
{
	/**
 	 * Version and header length
	 */
	union
	{
		/**
		 * Version - 4 bits
		 */
		uint8_t version; 

		/**
 		 * Header length in 4 bytes words - 4 bits
		 */
		uint8_t header_length;
	} vihl;

	/**
	 * Type of service
	 */
	uint8_t 	type_of_service;
	
	/**
	 * Data length
	 */
	uint16_t 	length;

	/**
	 * Packet ID
	 */
	uint16_t 	id;
	
	/**
	 * Flags and fragment offset
	 */
	union
	{
		/**
		 * Flags
		 */
		uint16_t flags;
		
		/**
		 * Fragment offset
 		 */
		uint16_t fragment_offset;
	} ffo;
	
	/**
	 * More fragments
	 */
	#define IP_FLAGS_MORE_FRAGMENTS 0x01

	/**
	 * Do not fragment
	 */
	#define IP_FLAGS_DO_NOT_FRAGMENT 0x02

	/**
	 * Time to live
	 */
	uint8_t 	ttl;
	
	/**
	 * Protocol
	 */
	uint8_t 	protocol;
	
	/**
	 * Checksum
	 */
	uint16_t 	checksum;

	/**
	 * Source IP Address
	 */
	ip_address	src;

	/**
	 * Destination IP Address
	 */
	ip_address 	dst;
};

/**
 * IP Address
 */
static ip_address ip_addr = NET_IP_ADDRESS;

/**
 * Netmask
 */
static ip_address ip_netmask = NET_IP_NETMASK;

/**
 * Gateway address
 */
static ip_address ip_gateway = NET_IP_GATEWAY;

/**
 * Broadcast address
 */
static ip_address ip_broadcast;

void ip_init(const ip_address * addr,const ip_address * netmask,const ip_address * gateway)
{
	if(addr)
		memcpy(&ip_addr,addr,sizeof(ip_address));
	if(netmask)
		memcpy(&ip_netmask,netmask,sizeof(ip_address));
	if(gateway)
		memcpy(&ip_gateway,gateway,sizeof(ip_address));
}

uint8_t ip_handle_packet(struct ip_header * header, uint16_t packet_len,const ethernet_address * mac )
{	
	if(packet_len < sizeof(struct ip_header))
		return 0;

	/* Check IP version */
	if((header->vihl.version>>4) != IP_V4)
		return 0;

	/* get header length */
	uint8_t header_length = (header->vihl.header_length & IP_VIHL_HL_MASK)*4;
	
	/* get packet length */
	uint16_t packet_length = ntoh16(header->length);
	
	/* check packet length */
	if(packet_length > packet_len)
		return 0;

	/* do not support fragmentation */
	if(ntoh16(header->ffo.flags) & (IP_FLAGS_MORE_FRAGMENTS << 13) || ntoh16(header->ffo.fragment_offset) & 0x1fff)
		return 0;

	/* check destination ip address */
	if(memcmp(&header->dst,ip_get_addr(),sizeof(ip_address)))
	{
		/* check if this is broadcast packet */
		if(	header->dst[0] != 0xff ||
			header->dst[1] != 0xff ||
			header->dst[2] != 0xff ||
			header->dst[3] != 0xff)
			return 0;
	}

	/* check checksum */
	if(ntoh16(header->checksum) != (~net_get_checksum(0,(const uint8_t*)header,header_length,10)))
		return 0;

	/* add to arp table if ip does not exist */
	//arp_table_insert((const ip_address*)&header->src,mac);
	
	/* redirect packet to the proper upper layer */
	switch(header->protocol)
	{
// #if NET_ICMP		
		case IP_PROTOCOL_ICMP:
			icmp_handle_packet(
				(const ip_address*)&header->src,
				(const struct icmp_header*)((const uint8_t*)header + header_length),
				packet_length-header_length);
			break;
// #endif //NET_ICMP
#if NET_UDP
		case IP_PROTOCOL_UDP:
			udp_handle_packet(
				(const ip_address*)&header->src,
				(const struct udp_header*)((const uint8_t*)header + header_length),
				packet_length-header_length);
			break;
#endif //NET_UDP			
		case IP_PROTOCOL_TCP:
			tcp_handle_packet(
				(const ip_address*)&header->src,
				(const struct tcp_header*)((const uint8_t*)header + header_length),
				packet_length-header_length);
			break;
		default:
			break;
	}
	return 0;
}


