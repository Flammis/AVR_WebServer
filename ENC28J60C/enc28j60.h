/*
 * Defines the registers on ethernet controller enc28j60.
 * Contains an api for communication
 * Contains minimum functionality to configure the controller. 
 */

#ifndef ENC28J60_H
  #define ENC28J60_H
  
  /*
  ENC28J60 has 4 banks of registers,
  Bank 0,1,2,3.
  Each bank stretches from 0x0-0x1F
  However registers 0x1B-0x1F are common registers shared by all banks.
  These registers are used to control the main functions of the Ethernet controller.
  
  Groups:
  Control registers for the ENC28J60 are generically
  grouped as ETH, MAC and MII registers. Register
  names starting with “E” belong to the ETH group.
  Similarly, registers names starting with “MA” belong to
  the MAC group and registers prefixed with “MI” belong
  to the MII group.
  On page 29 in the datasheet it is described that the MAC group and MII group
  when reading these registers a dummy byte is first recieved.
  
  8 bits are used for total register range.
  Within each bank 5 bits are used for addressing the registers.
  2 bits are used for addressing the bank.
  1 bit is used to specify group (treated differently)
  Bits:
  
  |         Bit7|    Bit6|    Bit5|   Bit4|   Bit3|   Bit2|   Bit1|   Bit0|
  |Eth_Reg_Group|BankBit1|BankBit0|RegBit4|RegBit3|RegBit2|RegBit1|RegBit0|
  
  */
  
  #define ADDR_MASK 0x1F
  
  #define BANK0 0x00
  #define BANK1 0x20
  #define BANK2 0x40
  #define BANK3 0x60
  #define BANK_MASK 0x80

  #define ETH_REG_GROUP 0x80
  
  /*Common registers*/
  #define EIE         0x1B
  #define EIR         0x1C
  #define ESTAT       0x1D
  #define ECON2       0x1E
  #define ECON1       0x1F
  
 /*
  ************** BANK 0*************
  */
  #define ERDPTL      (0x00 | BANK0)
  #define ERDPTH      (0x01 | BANK0)
  #define EWRPTL      (0x02 | BANK0)
  #define EWRPTH      (0x03 | BANK0)
  #define ETXSTL      (0x04 | BANK0)
  #define ETXSTH      (0x05 | BANK0)
  #define ETXNDL      (0x06 | BANK0)
  #define ETXNDH      (0x07 | BANK0)
  #define ERXSTL      (0x08 | BANK0)
  #define ERXSTH      (0x09 | BANK0)
  #define ERXNDL      (0x0A | BANK0)
  #define ERXNDH      (0x0B | BANK0)
  #define ERXRDPTL    (0x0C | BANK0)
  #define ERXRDPTH    (0x0D | BANK0)
  #define ERXWRPTL    (0x0E | BANK0)
  #define ERXWRPTH    (0x0F | BANK0)
  #define EDMASTL     (0x10 | BANK0)
  #define EDMASTH     (0x11 | BANK0)
  #define EDMANDL     (0x12 | BANK0)
  #define EDMANDH     (0x13 | BANK0)
  #define EDMADSTL    (0x14 | BANK0)
  #define EDMADSTH    (0x15 | BANK0)
  #define EDMACSL     (0x16 | BANK0)
  #define EDMACSH     (0x17 | BANK0)
  
 /*
  ************BANK 1************
  */
  #define EHT0       (0x00|BANK1)
  #define EHT1       (0x01|BANK1)
  #define EHT2       (0x02|BANK1)
  #define EHT3       (0x03|BANK1)
  #define EHT4       (0x04|BANK1)
  #define EHT5       (0x05|BANK1)
  #define EHT6       (0x06|BANK1)
  #define EHT7       (0x07|BANK1)
  #define EPMM0      (0x08|BANK1)
  #define EPMM1      (0x09|BANK1)
  #define EPMM2      (0x0A|BANK1)
  #define EPMM3      (0x0B|BANK1)
  #define EPMM4      (0x0C|BANK1)
  #define EPMM5      (0x0D|BANK1)
  #define EPMM6      (0x0E|BANK1)
  #define EPMM7      (0x0F|BANK1)
  #define EPMCSL     (0x10|BANK1)
  #define EPMCSH     (0x11|BANK1)
  #define EPMOL      (0x14|BANK1)
  #define EPMOH      (0x15|BANK1)
  #define EWOLIE     (0x16|BANK1)
  #define EWOLIR     (0x17|BANK1)
  #define ERXFCON    (0x18|BANK1)
  #define EPKTCNT    (0x19|BANK1)
  
 /*
  ************BANK 2************
  */
  #define MACON1           (0x00|BANK2|ETH_REG_GROUP)
  #define MACON2           (0x01|BANK2|ETH_REG_GROUP)
  #define MACON3           (0x02|BANK2|ETH_REG_GROUP)
  #define MACON4           (0x03|BANK2|ETH_REG_GROUP)
  #define MABBIPG          (0x04|BANK2|ETH_REG_GROUP)
  #define MAIPGL           (0x06|BANK2|ETH_REG_GROUP)
  #define MAIPGH           (0x07|BANK2|ETH_REG_GROUP)
  #define MACLCON1         (0x08|BANK2|ETH_REG_GROUP)
  #define MACLCON2         (0x09|BANK2|ETH_REG_GROUP)
  #define MAMXFLL          (0x0A|BANK2|ETH_REG_GROUP)
  #define MAMXFLH          (0x0B|BANK2|ETH_REG_GROUP)
  #define MAPHSUP          (0x0D|BANK2|ETH_REG_GROUP)
  #define MICON            (0x11|BANK2|ETH_REG_GROUP)
  #define MICMD            (0x12|BANK2|ETH_REG_GROUP)
  #define MIREGADR         (0x14|BANK2|ETH_REG_GROUP)
  #define MIWRL            (0x16|BANK2|ETH_REG_GROUP)
  #define MIWRH            (0x17|BANK2|ETH_REG_GROUP)
  #define MIRDL            (0x18|BANK2|ETH_REG_GROUP)
  #define MIRDH            (0x19|BANK2|ETH_REG_GROUP)
  
 /*
  ************BANK 3************
  */  
  #define MAADR1           (0x00|BANK3|ETH_REG_GROUP)
  #define MAADR0           (0x01|BANK3|ETH_REG_GROUP)
  #define MAADR3           (0x02|BANK3|ETH_REG_GROUP)
  #define MAADR2           (0x03|BANK3|ETH_REG_GROUP)
  #define MAADR5           (0x04|BANK3|ETH_REG_GROUP)
  #define MAADR4           (0x05|BANK3|ETH_REG_GROUP)
  #define EBSTSD           (0x06|BANK3)
  #define EBSTCON          (0x07|BANK3)
  #define EBSTCSL          (0x08|BANK3)
  #define EBSTCSH          (0x09|BANK3)
  #define MISTAT           (0x0A|BANK3|ETH_REG_GROUP)
  #define EREVID           (0x12|BANK3)
  #define ECOCON           (0x15|BANK3)
  #define EFLOCON          (0x17|BANK3)
  #define EPAUSL           (0x18|BANK3)
  #define EPAUSH           (0x19|BANK3)
  
  /*
   * PHY registers
   */
  #define PHCON1            0x00
  #define PHSTAT1           0x01
  #define PHID1             0x02
  #define PHID2             0x03
  #define PHCON2            0x10
  #define PHSTAT2           0x11
  #define PHIE              0x12
  #define PHIR              0x13
  #define PHLCON            0x14
  
  // ENC28J60 ERXFCON Register Bit Definitions
  #define ERXFCON_UCEN     0x80
  #define ERXFCON_ANDOR    0x40
  #define ERXFCON_CRCEN    0x20
  #define ERXFCON_PMEN     0x10
  #define ERXFCON_MPEN     0x08
  #define ERXFCON_HTEN     0x04
  #define ERXFCON_MCEN     0x02
  #define ERXFCON_BCEN     0x01
  // ENC28J60 EIE Register Bit Definitions
  #define EIE_INTIE        0x80
  #define EIE_PKTIE        0x40
  #define EIE_DMAIE        0x20
  #define EIE_LINKIE       0x10
  #define EIE_TXIE         0x08
  #define EIE_WOLIE        0x04
  #define EIE_TXERIE       0x02
  #define EIE_RXERIE       0x01
  // ENC28J60 EIR Register Bit Definitions
  #define EIR_PKTIF        0x40
  #define EIR_DMAIF        0x20
  #define EIR_LINKIF       0x10
  #define EIR_TXIF         0x08
  #define EIR_WOLIF        0x04
  #define EIR_TXERIF       0x02
  #define EIR_RXERIF       0x01
  // ENC28J60 ESTAT Register Bit Definitions
  #define ESTAT_INT        0x80
  #define ESTAT_LATECOL    0x10
  #define ESTAT_RXBUSY     0x04
  #define ESTAT_TXABRT     0x02
  #define ESTAT_CLKRDY     0x01
  // ENC28J60 ECON2 Register Bit Definitions
  #define ECON2_AUTOINC    0x80
  #define ECON2_PKTDEC     0x40
  #define ECON2_PWRSV      0x20
  #define ECON2_VRPS       0x08
  // ENC28J60 ECON1 Register Bit Definitions
  #define ECON1_TXRST      0x80
  #define ECON1_RXRST      0x40
  #define ECON1_DMAST      0x20
  #define ECON1_CSUMEN     0x10
  #define ECON1_TXRTS      0x08
  #define ECON1_RXEN       0x04
  #define ECON1_BSEL1      0x02
  #define ECON1_BSEL0      0x01
  // ENC28J60 MACON1 Register Bit Definitions
  #define MACON1_LOOPBK    0x10
  #define MACON1_TXPAUS    0x08
  #define MACON1_RXPAUS    0x04
  #define MACON1_PASSALL   0x02
  #define MACON1_MARXEN    0x01
  // ENC28J60 MACON2 Register Bit Definitions
  #define MACON2_MARST     0x80
  #define MACON2_RNDRST    0x40
  #define MACON2_MARXRST   0x08
  #define MACON2_RFUNRST   0x04
  #define MACON2_MATXRST   0x02
  #define MACON2_TFUNRST   0x01
  // ENC28J60 MACON3 Register Bit Definitions
  #define MACON3_PADCFG2   0x80
  #define MACON3_PADCFG1   0x40
  #define MACON3_PADCFG0   0x20
  #define MACON3_TXCRCEN   0x10
  #define MACON3_PHDRLEN   0x08
  #define MACON3_HFRMLEN   0x04
  #define MACON3_FRMLNEN   0x02
  #define MACON3_FULDPX    0x01
  // ENC28J60 MICMD Register Bit Definitions
  #define MICMD_MIISCAN    0x02
  #define MICMD_MIIRD      0x01
  // ENC28J60 MISTAT Register Bit Definitions
  #define MISTAT_NVALID    0x04
  #define MISTAT_SCAN      0x02
  #define MISTAT_BUSY      0x01
  // ENC28J60 PHY PHCON1 Register Bit Definitions
  #define PHCON1_PRST      0x8000
  #define PHCON1_PLOOPBK   0x4000
  #define PHCON1_PPWRSV    0x0800
  #define PHCON1_PDPXMD    0x0100
  // ENC28J60 PHY PHSTAT1 Register Bit Definitions
  #define PHSTAT1_PFDPX    0x1000
  #define PHSTAT1_PHDPX    0x0800
  #define PHSTAT1_LLSTAT   0x0004
  #define PHSTAT1_JBSTAT   0x0002
  // ENC28J60 PHY PHCON2 Register Bit Definitions
  #define PHCON2_FRCLINK   0x4000
  #define PHCON2_TXDIS     0x2000
  #define PHCON2_JABBER    0x0400
  #define PHCON2_HDLDIS    0x0100

  // ENC28J60 Packet Control Byte Bit Definitions
  #define PKTCTRL_PHUGEEN  0x08
  #define PKTCTRL_PPADEN   0x04
  #define PKTCTRL_PCRCEN   0x02
  #define PKTCTRL_POVERRIDE 0x01 
  
  // SPI operation codes
  #define ENC28J60_READ_CTRL_REG       0x00
  #define ENC28J60_READ_BUF_MEM        0x3A
  #define ENC28J60_WRITE_CTRL_REG      0x40
  #define ENC28J60_WRITE_BUF_MEM       0x7A
  #define ENC28J60_BIT_FIELD_SET       0x80
  #define ENC28J60_BIT_FIELD_CLR       0xA0
  #define ENC28J60_SOFT_RESET          0xFF
  
  /*
  The RXSTART_INIT should be zero. See Rev. B4 Silicon Errata
  buffer boundaries applied to internal 8K ram
  the entire available packet buffer space is allocated

  start with recbuf at 0
  */
  #define RXSTART_INIT      0x0

  /*
    ERXST points to the start of the recieve buffer.
    ERXND points to the end of the recieve buffer.
    The buffer is fifo and will never write outside the boundaries
    of ERXST<->ERXND
    The pointers must not be modified while the receive
    logic is enabled (ECON1.RXEN is set).
  */
  /*Recieve buffer end 6400 bytes*/
  #define RXSTOP_INIT       (0x1FFF-0x0600-1)
  
  /*
  Transmitt buffer:
  ETXST points to the start of the transmit buffer.
  ETXND points to the end of the transmit buffer.
  It is recommended that an even address be used for
  ETXST.
  Whenever the host controller decides to transmit a packet, the ETXST and
  ETXND pointers are programmed with addresses
  specifying where, within the transmit buffer, the particular
  packet to transmit is located.
  0x600 is 1536 bytes.
  The transmitter buffer ranges from 0x19FF<->0x1FFD which is 1534 bytes.
  However only 1500 bytes are used for the frame.
  Since the controller adds the 18 byte header.
  which contains:
    Destation Address (MAC) 6 Bytes,
    Source Address (MAC) 6 Bytes,
    Type/Length of frame 2 Bytes,
    Data: 46-1500 Bytes,
    CRC (Cyclic redundacy check) 4 Bytes
  */
  #define TXSTART_INIT     (0x1FFF-0x0600)
  
  /*
  When the packet is finished transmitting or was aborted
  due to an error/cancellation, the ECON1.TXRTS bit will
  be cleared, a seven-byte transmit status vector will be
  written to the location pointed to by ETXND + 1
  */
  #define TXSTOP_INIT      0x1FFD
  
  #define MAX_FRAMELEN     1500

  // functions
  extern uint8_t Enc28j60ReadOp(uint8_t op, uint8_t address);
  extern void Enc28j60WriteOp(uint8_t op, uint8_t address, uint8_t data);
  extern void Enc28j60ReadBuffer(uint16_t len, uint8_t* data);
  extern void Enc28j60WriteBuffer(uint16_t len, uint8_t* data);
  extern void Enc28j60SetBank(uint8_t address);
  extern uint8_t Enc28j60Read(uint8_t address);
  extern void Enc28j60Write(uint8_t address, uint8_t data);
  extern void Enc28j60PhyWrite(uint8_t address, uint16_t data);
  extern void Enc28j60clkout(uint8_t clk);
  extern void InitPhy (void);
  extern void Enc28j60Init(uint8_t* macaddr);
  extern void Enc28j60PacketSend(uint16_t len, uint8_t* packet);
  extern uint16_t Enc28j60PacketReceive(uint16_t maxlen, uint8_t* packet);
  extern uint8_t Enc28j60getrev(void);
  
#endif