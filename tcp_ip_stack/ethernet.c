
#include <ethernet.h>

struct ethernet_header
{
  ethernet_address 	dst;
  ethernet_address 	src;
  uint16_t 		type;
};

struct ethernet_stats
{
	uint32_t rx_packets;
	uint32_t tx_packets;
};

static struct ethernet_stats ethernet_stats;
static ethernet_address ethernet_mac;

uint8_t ethernet_tx_buffer[ETHERNET_MAX_PACKET_SIZE + NET_HEADER_SIZE_ETHERNET]; // EXMEM
uint8_t ethernet_rx_buffer[ETHERNET_MAX_PACKET_SIZE + NET_HEADER_SIZE_ETHERNET]; //EXMEM


void ethernet_init(const ethernet_address * mac)
{
	memset(&ethernet_stats,0,sizeof(ethernet_stats));
	memset(ethernet_mac,1,sizeof(ethernet_mac));
	if(mac) {
    memcpy(&ethernet_mac,mac,sizeof(ethernet_mac));  
  }
}

uint16_t handle_ethernet_packet()
{
  uint16_t packet_size = 0;
  
  packet_size = Enc28j60PacketReceive(sizeof(ethernet_rx_buffer), ethernet_rx_buffer);

  if (packet_size == 0) return 0;

  struct ethernet_header * header = (struct ethernet_header*)ethernet_rx_buffer;

  packet_size -=sizeof(*header);
  
  uint8_t* data = (uint8_t*)(header +1);
  
  uint8_t ret=1;
  switch(header->type)
  {
    case HTON16(ETHERNET_TYPE_IP):
      //IP handle packet
      ret = ip_handle_packet((struct ip_header*)data,packet_size,(const ethernet_address*)&header->src);
      break;
    case HTON16(ETHERNET_TYPE_ARP):
      //ARP handle packet
      ret = arp_handle_packet((struct arp_header*)data,packet_size);
      break;
    default:
      return 0;
  }
  ethernet_stats.rx_packets++;
  return ret;
}

uint8_t ethernet_send_packet(ethernet_address * dst,uint16_t type,uint16_t len)
{
	if(len > ETHERNET_MAX_PACKET_SIZE -NET_HEADER_SIZE_ETHERNET){
		return 0;
  }
	struct ethernet_header * header = (struct ethernet_header*)ethernet_tx_buffer;
	if(dst == ETHERNET_ADDR_BROADCAST){
		memset(&header->dst,0xff,sizeof(ethernet_address));
	} else {
		memcpy(&header->dst,dst,sizeof(ethernet_address));
	}
  memcpy(&header->src,&ethernet_mac,sizeof(ethernet_address));
	header->type = hton16(type);
	ethernet_stats.tx_packets++;
	return Enc28j60PacketSend((len + NET_HEADER_SIZE_ETHERNET), ethernet_tx_buffer);			
}
