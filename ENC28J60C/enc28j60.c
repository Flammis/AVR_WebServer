
#include <avr/io.h>
#include "enc28j60.h"
#include "lowlevel.h"


/*
The DDRxn bit in the DDRx Register selects the direction of this pin. If DDxn is written logic one,
Pxn is configured as an output pin. If DDxn is written logic zero, Pxn is configured as an input
pin.

If PORTxn is written logic one when the pin is configured as an input pin, the pull-up resistor is
activated.

If PORTxn is written logic one when the pin is configured as an output pin, the port pin is driven
high (one). If PORTxn is written logic zero when the pin is configured as an output pin, the port
pin is driven low (zero).


SPI
PortB
BITS  | PB7|PB6|PB5| PB4| PB3|PB2|PB1|PB0|
      |    |   |SCK|MISO|MOSI| SS|   |   | 

MASTER: Microcontroller, SLAVE: ENC28J60
RBM - Read Buffer Memory
The RBM command is started by pulling the CS pin low.
The RBM command is terminated by raising the CS pin.
*/

static uint8_t Enc28j60Bank;
static uint16_t NextPacketPtr;

#define ENC28J60_CONTROL_PORT    PORTB
#define ENC28J60_CONTROL_DDR     DDRB
#define ENC28J60_CONTROL_CS      PORTB2
#define ENC28J60_CONTROL_MOSI    PORTB3
#define ENC28J60_CONTROL_MISO    PORTB4
#define ENC28J60_CONTROL_SCK     PORTB5

// set CS to 0 = active
#define CSACTIVE ENC28J60_CONTROL_PORT &= ~(1 << ENC28J60_CONTROL_CS)
// set CS to 1 = passive
#define CSPASSIVE ENC28J60_CONTROL_PORT |= (1 << ENC28J60_CONTROL_CS)
//
#define waitspi() while(!(SPSR&(1<<SPIF)))


uint8_t Enc28j60ReadOp(uint8_t op, uint8_t address);
void Enc28j60WriteOp(uint8_t op, uint8_t address, uint8_t data);
void Enc28j60ReadBuffer(uint16_t len, uint8_t* data);
void Enc28j60WriteBuffer(uint16_t len, uint8_t* data);
void Enc28j60SetBank(uint8_t address);
uint8_t Enc28j60Read(uint8_t address);
void Enc28j60Write(uint8_t address, uint8_t data);
void Enc28j60PhyWrite(uint8_t address, uint16_t data);
void Enc28j60clkout(uint8_t clk);
void InitPhy (void);


void Enc28j60Init(uint8_t* macaddr)
{
  //Configure CS pin as output
  ENC28J60_CONTROL_DDR |= (1 << ENC28J60_CONTROL_CS);
  //Set CS inactive (Set CS pin high)
  CSPASSIVE;
  
  //Set MOSI AND SCK as output
   ENC28J60_CONTROL_DDR |= (1 << ENC28J60_CONTROL_MOSI) | (1 << ENC28J60_CONTROL_SCK); // mosi, sck output
  //Set MISO as input  
  ENC28J60_CONTROL_DDR &= ~(1 << ENC28J60_CONTROL_MISO);
  //Set MOSI AND SCK LOW
  ENC28J60_CONTROL_PORT &= ~(1 << ENC28J60_CONTROL_MOSI);
  ENC28J60_CONTROL_PORT &= ~(1 << ENC28J60_CONTROL_SCK);
  
  //Enable SPI and set as master
  SPCR = (1 << SPE) | (1 << MSTR);
  //Set SCK clock to Fosc/2 clock:
  SPSR |= (1 << SPI2X);
  
  
  //Reset ethernet controller
  Enc28j60WriteOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
  //CLKRDY is not cleared after reset. Workaround wait at least 1 ms
  _delay_ms(50);
  
  // set receive buffer start address
  NextPacketPtr = RXSTART_INIT;
  // 16-bit transfers, must write low byte first
  Enc28j60Write(ERXSTL, RXSTART_INIT & 0xFF);
  Enc28j60Write(ERXSTH, RXSTART_INIT >> 8);
  
  // set receive pointer address
  Enc28j60Write(ERXRDPTL, RXSTART_INIT & 0xFF);
  Enc28j60Write(ERXRDPTH, RXSTART_INIT >> 8);
  // RX end
  Enc28j60Write(ERXNDL, RXSTOP_INIT & 0xFF);
  Enc28j60Write(ERXNDH, RXSTOP_INIT >> 8);
  
  // TX start
  Enc28j60Write(ETXSTL, TXSTART_INIT & 0xFF);
  Enc28j60Write(ETXSTH, TXSTART_INIT >> 8);
  // TX end
  Enc28j60Write(ETXNDL, TXSTOP_INIT & 0xFF);
  Enc28j60Write(ETXNDH, TXSTOP_INIT >> 8);
  
  /*
  Bank1
    Only accept unicast our mac address MAADR
    as well as broadcast packets for ARP packets:
    Ethernet frame:
      Destation Address (MAC) 6 Bytes,
      Source Address (MAC) 6 Bytes,
      Type/Length of frame 2 Bytes,
      Data: 46-1500 Bytes,
      CRC (Cyclic redundacy check) 4 Bytes
      
    Ethernet frame has Type=0x0608 for ARP packets.
    AND MACADDR for BROADCAST = ff ff ff ff ff ff
    EPMM0 - mask 0-7 byte
    EPMM1 - mask 8-15 byte
    EMPOH:EPMOL - offset for mask - default value 0x0000
    EPMCSH:EPMCSL - checksum resister
    Data bytes
    which have corresponding mask bits programmed to ‘0’
    are completely removed for purposes of calculating the
    checksum.
    EPMM0 = 0x3F
    EPMM1 = 0x30
    EPMCS = ~(0x0608)
  */
  Enc28j60Write(ERXFCON, ERXFCON_UCEN | ERXFCON_CRCEN | ERXFCON_PMEN);
  Enc28j60Write(EPMM0, 0x3f);
  Enc28j60Write(EPMM1, 0x30);
  Enc28j60Write(EPMCSL, 0xf9);
  Enc28j60Write(EPMCSH, 0xf7);
  
  /*
  Bank2:
    Enable MAC recieve.
    Inhibit tranmission when receiving a pause frame.
    Enable sending pause frames.
  */
  Enc28j60Write(MACON1, MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);
  //Disable all reset MAC flags.
  Enc28j60Write(MACON2, 0x00);
  
  // enable automatic padding to 60bytes and CRC operations
  Enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN);
}


void Enc28j60PacketSend(uint16_t len, uint8_t* packet);
uint16_t Enc28j60PacketReceive(uint16_t maxlen, uint8_t* packet);
uint8_t Enc28j60getrev(void);

