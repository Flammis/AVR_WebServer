#include <avr/io.h>
#include <util/delay.h>
#include <lowlevelinit.h>

/******************************************************************************/
extern void ExternIntInit (void)
{
  /*
  The DDRxn bit in the DDRx Register selects the direction of this pin. If DDxn is written logic one,
  Pxn is configured as an output pin. If DDxn is written logic zero, Pxn is configured as an input
  pin.
  
  If PORTxn is written logic one when the pin is configured as an input pin, the pull-up resistor is
  activated.
  */
  DDRD &= ~(0x04);     // INT 0 = PD2 = input with pull up
  PORTD |= 0x04;

  EICRA &= 0xFE;//INT 0 triggers on falling edge
  EICRA |= 0x02;

  EIMSK |= 0x01;          //enable interrupt on INT 0
  
  // MCUCR &= 0xFE;       // INT 0 sense on falling edge
  // MCUCR |= 0x02;
  // GICR |= 0x40;        // enable interrupt on INT 0
}


/******************************************************************************/
extern void Timer1Init (void)
{
  TCNT1 = 0;                       // count from 0
  OCR1A = 19999;                   // divide 16 MHz with 8 * 20000 to 100 Hz
  /*
  TIMSK.OCIE1A=1
  Timer1 Output Compare A Match Interrupt Enable
  //TIMSK = TIMSK | 0x10;            // enable intr on OCR1A
  */
  //TIMSK1 = TIMSK1 | 0x02;

  /*
  Bit:     |  Bit7|   Bit6|  Bit5|   Bit4|   Bit3|  Bit2|  Bit1| Bit0|
  TCCR1A : |COM1A1| COM1A0|COM1B1| COM1B0|       |      | WGM11|WGM10|
  TCCR1B : | ICNC1|  ICES1|      |  WGM13|  WGM12|  CS12|  CS11| CS10|
  WGM13,WGM12,WGM11,WGM10 = 0100 Mode 4 CTC (Clear Timer on Compare)
  */
  TCCR1A = 0x00;                   // mode = 4, 
  TCCR1B = 0x0A;                   // prescaler = 8, start running
}


//*****************************************************************************
extern void InitIo (void)
{
  // all output high
  DDRB = 0xFF;
  DDRC = 0xFF;
  DDRD = 0xFB; //Int0, Int1
  PORTB = 0xFF;
  PORTC = 0xFF;
  PORTD = 0xFF;
}