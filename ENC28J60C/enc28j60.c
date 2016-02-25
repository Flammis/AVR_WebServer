
#include <avr/io.h>
#include <util/delay.h>
#include <enc28j60.h>
#include <lowlevelinit.h>


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

  
/*******************************************************************
There are 2 read commands:
  RCM - Read Control Register
    RCM uses address as input to specify which register in the bank.
  RBM - Read Buffer Memory
    RBM requires address = 0 and the opcode is 8 bits - 0x3A
    
  Reading a MAC and MII group first one recieves a dummy byte.
********************************************************************/
uint8_t Enc28j60ReadOp(uint8_t op, uint8_t address)
{
  CSACTIVE;
  // issue read command
  SPDR = op | (address & ADDR_MASK);
  waitspi();
  // read data
  SPDR = 0x00;
  waitspi();
  //dummy read if MII or MAC reg group
  if(address & M_REG_GROUP)
  {
    SPDR = 0x00;
    waitspi();
  }
  // release CS
  CSPASSIVE;
  return(SPDR);
}

/*******************************************************************
Input:
  op - operation
  address - 5bit address
  data - byte to write
There are 5 write commands:
  WCR - Write Control Register
  WBM - Write Buffer Memory
  BFS - Bit Field Set, ORed with current register.
    Set bitfields in the ETH registers in the current bank.
  BFC - Bit Field Clear
    Clears the high bitfields of data, (only high databits are cleared).
  SC - System Command (Soft Reset)
********************************************************************/
void Enc28j60WriteOp(uint8_t op, uint8_t address, uint8_t data)
{
  CSACTIVE;
  // issue write command
  SPDR = op | (address & ADDR_MASK);
  waitspi();
  // write data
  SPDR = data;
  waitspi();
  CSPASSIVE;
}

/*******************************************************************
Reads the buffer.
While CS pin is low the SCK starts each time SPDR is written to
and SCK stops when transfer is done.
********************************************************************/
void Enc28j60ReadBuffer(uint16_t len, uint8_t* data)
{
  CSACTIVE;
  // issue read command
  SPDR = ENC28J60_READ_BUF_MEM;
  waitspi();
  while(len)
  {
    len--;
    // read data
    SPDR = 0x00;
    waitspi();
    *data = SPDR;
    data++;
  }
  *data='\0';
  CSPASSIVE;
}

/*******************************************************************
Writes to the transmitter buffer.
While CS pin is low the SCK starts each time SPDR is written to
and SCK stops when transfer is done.

EWRPTL, EWRPTH, ETXNDL, ETXNDH (Transmitt start and end pointer)
must be set prior.
********************************************************************/
void Enc28j60WriteBuffer(uint16_t len, uint8_t* data)
{
  CSACTIVE;
  // issue write command
  SPDR = ENC28J60_WRITE_BUF_MEM;
  waitspi();
  while(len)
  {
    len--;
    // write data
    SPDR = *data;
    data++;
    waitspi();
  }
  CSPASSIVE;
}

/*******************************************************************
If CurrentBank!=NewBank:
  Clears the current bank select bits.
  Sets the new bank select bits.
  CurrentBank=NewBank.
end
********************************************************************/
void Enc28j60SetBank(uint8_t address)
{
  if((address & BANK_MASK) != Enc28j60Bank)
  {
    Enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1|ECON1_BSEL0));
    Enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, (address & BANK_MASK)>>5);
    Enc28j60Bank = (address & BANK_MASK);
  }
}

/*******************************************************************
Read control register
********************************************************************/
uint8_t Enc28j60Read(uint8_t address)
{
  // set the bank
  Enc28j60SetBank(address);
  // do the read
  return Enc28j60ReadOp(ENC28J60_READ_CTRL_REG, address);
}

/*******************************************************************
Write control register
********************************************************************/
void Enc28j60Write(uint8_t address, uint8_t data)
{
  // set the bank
  Enc28j60SetBank(address);
  // do the write
  Enc28j60WriteOp(ENC28J60_WRITE_CTRL_REG, address, data);  
}


/*******************************************************************
Writing a PHY register:
1.Write MIREGADR=address
2.Write MIWRL
3.Write MIWRH ->
  triggers update and updates register with address MIGEGADR with data:MIWR
  MISTAT.BUSY is automatically set.
4. After 10.24µs BUSY is cleared.
  (Controller should not start any other MIISCAN or MIIRD op)
********************************************************************/
void Enc28j60PhyWrite(uint8_t address, uint16_t data)
{
  // set the PHY register address
  Enc28j60Write(MIREGADR, address);
  // write the PHY data
  Enc28j60Write(MIWRL, data);
  Enc28j60Write(MIWRH, data>>8);
  // wait until the PHY write completes
  while(Enc28j60Read(MISTAT) & MISTAT_BUSY)
  {
    _delay_us(15);
  }
}

/*******************************************************************
Flash the 2 RJ45 LEDs twice to show that the interface works.

PHLCON (LED PHY Control Register):
| Bit15| Bit14| Bit13| Bit12| Bit11| Bit10|  Bit9|  Bit8|
|      |      |      |      |LACFG3|LACFG2|LACFG1|LACFG0|
|  Bit7|  Bit6|  Bit5|  Bit4|  Bit3|  Bit2|  Bit1|  Bit0|
|LBCFG3|LBCFG2|LBCFG1|LBCFG0| LFRQ1| LFRQ0| STRCH|      |
LEDA=green LEDB=yellow

LACFG (LEDA Configuration bits)
LBCFG (LEDB Configuration bits)
    1000 -> Turn On
    1001 -> Turn Off
    0100 -> Display Link Status
    0111 -> Display transmit and receive activity
      
LFRQ (Pulse Stretch Time Configuration)
    00 -> Stretch LED events to approximately 40 ms
    01 -> Stretch LED events to approximately 73 ms
    
STRCH (LED Pulse Stretching Enable bit)
    0 -> Ignore Stretch time.
    1 -> Enable Stretch time.
********************************************************************/
void InitPhy (void)
{
  //Turn LEDA, LEDB on
	// Enc28j60PhyWrite(PHLCON,0x880);
	// _delay_ms(2000);
	
  // //Turn LEDA, LEDB off
	// Enc28j60PhyWrite(PHLCON,0x990);
	// _delay_ms(500);
	
  // //Turn LEDA, LEDB on
	// Enc28j60PhyWrite(PHLCON,0x880);
	// _delay_ms(500);
	
  // //Turn LEDA, LEDB off
	// Enc28j60PhyWrite(PHLCON,0x990);
	// _delay_ms(500);
  
  // //Turn LEDA on
	// Enc28j60PhyWrite(PHLCON,0x890);
	// _delay_ms(500);

  // //Turn LEDA, LEDB on
	// Enc28j60PhyWrite(PHLCON,0x880);
	// _delay_ms(500);

  // //Turn LEDB on
	// Enc28j60PhyWrite(PHLCON,0x980);
	// _delay_ms(500);
	
  //Turn LEDA, LEDB off
	// Enc28j60PhyWrite(PHLCON,0x990);
	// _delay_ms(500);
  
  //LEDA=links status
  //LEDB=transmit/receive activity
  //Stretch 73 ms
  Enc28j60PhyWrite(PHLCON,0x476);
  // _delay_ms(100);
}

/*******************************************************************
Initialize Ethernet Controller
********************************************************************/
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
  /*
  Bank 0
  */
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
      Type/Length of frame 2 Bytes, Field<1500 -> Length of Data
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
  //Disable all reset MAC flags. (Go out of reset)
  Enc28j60Write(MACON2, 0x00);
  
  /*
  Reg MACON3
  |   Bit7|   Bit6|   Bit5|   Bit4|   Bit3|  Bit2|   Bit1|  Bit0|
  |PADCFG2|PADCFG1|PADCFG0|TXCRCEN|PHDRLEN|HFRMEN|FRMLNEN|FULDPX|
  PADCFG2:PACDFG0 = 001 ->  Padded to 60 bytes and a valid CRC (4 bytes) will then be appended.
  TXCRCEN (Transmit CRC Enable bit) = 1 - Always add a CRC regardless of frame size. (required for the above option)
  HFRMEN (Huge frame bit enable) = 0 -> Frames bigger than MAXFL will be aborted.
  FRMLNEN: Frame Length Checking Enable bit = 1 -> If the type/length field is of type length:
  Compare it with actual and if mismatch report in transmit status vector.
  Type/Length field is of lengthtype if Field<=1500.
  */
  // enable automatic padding to 60bytes and CRC operations
  Enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN);

  // set inter-frame gap (non-back-to-back)
  Enc28j60Write(MAIPGL, 0x12);
  Enc28j60Write(MAIPGH, 0x0C);
  // set inter-frame gap (back-to-back)
  Enc28j60Write(MABBIPG, 0x12);
  // Set the maximum packet size which the controller will accept
  // Do not send packets longer than MAX_FRAMELEN:
  Enc28j60Write(MAMXFLL, MAX_FRAMELEN & 0xFF);	
  Enc28j60Write(MAMXFLH, MAX_FRAMELEN >> 8);

  // write MAC address
  // NOTE: MAC address in ENC28J60 is byte-backward
  Enc28j60Write(MAADR5, macaddr[0]);
  Enc28j60Write(MAADR4, macaddr[1]);
  Enc28j60Write(MAADR3, macaddr[2]);
  Enc28j60Write(MAADR2, macaddr[3]);
  Enc28j60Write(MAADR1, macaddr[4]);
  Enc28j60Write(MAADR0, macaddr[5]);
  
  // no loopback of transmitted frames
  Enc28j60PhyWrite(PHCON2, PHCON2_HDLDIS);
  // switch to bank 0
  Enc28j60SetBank(ECON1);
  
  /*
  Enable interrupts.
  
  Register EIE (ETHERNET INTERRUPT ENABLE):
  | Bit7| Bit6| Bit5|  Bit4|Bit3| Bit2|  Bit1|  Bit0|
  |INTIE|PKTIE|DMAIE|LINKIE|TXIE|WOLIE|TXERIE|RXERIE|
  
    INTIE (Global INT Interrupt Enable bit)
        1 -> Allow interrupt events to drive the interrupt pin.
    PKTIE (Receive Packet Pending Interrupt Enable bit)
        1 -> Enable receive packet pending interrupt
  */
  Enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE | EIE_PKTIE);
  // enable packet reception
  Enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
}

/*******************************************************************
Transmitt Packet
********************************************************************/
uint8_t Enc28j60PacketSend(uint16_t len, uint8_t* packet)
{
  // Set the write pointer to start of transmit buffer area
  Enc28j60Write(EWRPTL, TXSTART_INIT & 0xFF);
  Enc28j60Write(EWRPTH, TXSTART_INIT >> 8);
  // Set the TXND pointer to correspond to the packet size given
  Enc28j60Write(ETXNDL, (TXSTART_INIT + len) & 0xFF);
  Enc28j60Write(ETXNDH, (TXSTART_INIT + len) >> 8);
  /*
  Additionally, the ENC28J60 requires a single per packet
  control byte to precede the packet for transmission.
  0x00 means use MACON3 settings (no overriding)
  */
  Enc28j60WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);
  // copy the packet into the transmit buffer
  Enc28j60WriteBuffer(len, packet);
  // send the contents of the transmit buffer onto the network
  Enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
  // Reset the transmit logic problem. See Rev. B4 Silicon Errata point 12.
  if((Enc28j60Read(EIR) & EIR_TXERIF))
  {
    Enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS);
  }
  return 1;
}

/*******************************************************************
// Gets a packet from the network receive buffer, if one is available.
// The packet will be headed by an ethernet header.
//      maxlen  The maximum acceptable length of a retrieved packet.
//      packet  Pointer where packet data should be stored.
// Returns: Packet length in bytes if a packet was retrieved, zero otherwise.
********************************************************************/
uint16_t Enc28j60PacketReceive(uint16_t maxlen, uint8_t* packet)
{
  uint16_t rxstat;
  uint16_t len;
  /* 
  When a packet is accepted and completely
  written into the buffer, the EPKTCNT
  register will increment, 
  the EIR.PKTIF bit will be set, 
  an interrupt will be generated.
  
  if( !(enc28j60Read(EIR) & EIR_PKTIF) ){}
  The above does not work. See Rev. B4 Silicon Errata point 6.
  
  */
  if( Enc28j60Read(EPKTCNT) ==0 )
  {
    return(0);
  }

  // Set the read pointer to the start of the received packet
  Enc28j60Write(ERDPTL, NextPacketPtr);
  Enc28j60Write(ERDPTH, NextPacketPtr >> 8);

  // read the next packet pointer
  NextPacketPtr  = Enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
  NextPacketPtr |= Enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0) << 8;

  // read the packet length (see datasheet page 43)
  len  = Enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
  len |= Enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0) << 8;
  len -= 4; //remove the CRC count

  // read the receive status (see datasheet page 43)
  rxstat  = Enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
  rxstat |= Enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0) << 8;

  // limit retrieve length
  if (len>maxlen-1)
  {
    len=maxlen-1;
  }
  
  /*
  StatusVector 16:31
  Bit23:
      Received Ok Indicates that at the packet
      had a valid CRC and no symbol errors.
      
      Check CRC and symbol errors (see datasheet page 44, table 7-3):
      The ERXFCON.CRCEN is set by default. Normally we should not
      need to check this.
  */
  if ((rxstat & 0x80)==0)
  {
    // invalid
    len=0;
  }
  else
  {
    // copy the packet from the receive buffer
    Enc28j60ReadBuffer(len, packet);
  }
   
  // Move the RX read pointer to the start of the next received packet
  // This frees the memory we just read out
  Enc28j60Write(ERXRDPTL, NextPacketPtr);
  Enc28j60Write(ERXRDPTH, NextPacketPtr >> 8);

  /*
  In addition to advancing the receive buffer read pointer,
  after each packet is fully processed, the host controller
  must write a ‘1’ to the ECON2.PKTDEC bit. Doing so
  will cause the EPKTCNT register to decrement by 1  
  */
  Enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
  return(len);
}

/*******************************************************************
read the revision of the chip
********************************************************************/
uint8_t Enc28j60getrev(void)
{
  return(Enc28j60Read(EREVID));
}

