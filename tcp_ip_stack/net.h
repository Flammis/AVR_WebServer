

#ifndef _NET_H
#define _NET_H

#include <stdint.h>

#define NET_HEADER_SIZE_ETHERNET	14
#define NET_HEADER_SIZE_IP		20
#define NET_HEADER_SIZE_TCP		20
#define ETHERNET_MAX_PACKET_SIZE	1400


#if BIG_ENDIAN
#define HTON16(val) (val)
#define HTON32(val) (val)
#else
#define HTON16(val) 	(						\
			(((uint16_t) (val)) << 8) | 			\
			(((uint16_t) (val)) >> 8)	 		\
			)
#define HTON32(val) 	(						\
			((((uint32_t) (val)) & 0x000000ff) << 24) |	\
			((((uint32_t) (val)) & 0x0000ff00) <<	8) | 	\
			((((uint32_t) (val)) & 0x00ff0000) >>	8) | 	\
			((((uint32_t) (val)) & 0xff000000) >> 24)	\
			)
#endif

#define NTOH16(val)	HTON16((val))

#define NTOH32(val)	HTON32((val))

uint16_t 	hton16(uint16_t h);
uint32_t 	hton32(uint32_t h);

#define ntoh16(n)	hton16((n))
#define ntoh32(n)	hton32((n))

#define MAKEUINT16(x,y) 	(((x)<<8)|(y)) 

uint16_t net_get_checksum(uint16_t checksum,const uint8_t * data,uint16_t len,uint8_t skip);



/**********IP Protocol*************/
#define ETHERNET_TYPE_IP	0x0800
#define ETHERNET_TYPE_ARP	0x0806

#endif