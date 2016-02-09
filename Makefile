
##########------------------------------------------------------##########
##########              Project-specific Details                ##########
##########    Check these every time you start a new project    ##########
##########------------------------------------------------------##########

MCU   = atmega328
F_CPU = 16000000UL
BAUD  = 9600UL
## Also try BAUD = 19200 or 38400 if you're feeling lucky.

## A directory for common include files and the simple USART library.
## If you move either the current folder or the Library folder, you'll 
##  need to change this path to match.
LIBDIR = C:/Users/GustavWi/Documents/AVR/avr8-gnu-toolchain/avr
LCDDIR = C:/Users/GustavWi/Documents/AVR/ProgrammeringAvEnkapselDatorer/LCD
ENC28JDIR = ./ENC28J60C
LowLvlInit = ./LowLevelInit
TCP_IP = ./tcp_ip_stack
UART_DIR = ./uart

##########------------------------------------------------------##########
##########                 Programmer Defaults                  ##########
##########          Set up once, then forget about it           ##########
##########        (Can override.  See bottom of file.)          ##########
##########------------------------------------------------------##########

PROGRAMMER_TYPE = usbasp
# extra arguments to avrdude: baud rate, chip type, -F flag, etc.
PROGRAMMER_ARGS = 	

##########------------------------------------------------------##########
##########                  Program Locations                   ##########
##########     Won't need to change if they're in your PATH     ##########
##########------------------------------------------------------##########

CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
AVRSIZE = avr-size
AVRDUDE = avrdude

##########------------------------------------------------------##########
##########                   Makefile Magic!                    ##########
##########         Summary:                                     ##########
##########             We want a .hex file                      ##########
##########        Compile source files into .elf                ##########
##########        Convert .elf file into .hex                   ##########
##########        You shouldn't need to edit below.             ##########
##########------------------------------------------------------##########

## The name of your project (without the .c)
# TARGET = blinkLED
## Or name it automatically after the enclosing directory
TARGET = main

# Object files: will find all .c/.h files in current directory
#  and in LIBDIR.  If you have any other (sub-)directories with code,
#  you can add them in to SOURCES below in the wildcard statement.
SOURCES=$(wildcard *.c $(LIBDIR)/*.c $(LCDDIR)/*.c $(ENC28JDIR)/*.c $(LowLvlInit)/*.c $(TCP_IP)/*.c $(UART_DIR)/*.c)
OBJECTS=$(SOURCES:.c=.o)
HEADERS=$(SOURCES:.c=.h)

## Compilation options, type man avr-gcc if you're curious.
CPPFLAGS = -DF_CPU=$(F_CPU) -DBAUD=$(BAUD) -I. -I$(LIBDIR) -I$(LCDDIR) -I$(ENC28JDIR) -I$(LowLvlInit) -I$(TCP_IP) -I$(UART_DIR)
CFLAGS = -Os -g -std=gnu99 -Wall
## Use short (8-bit) data types 
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums 
## Splits up object files per function
CFLAGS += -ffunction-sections -fdata-sections 
LDFLAGS = -Wl,-Map,$(TARGET).map 
## Optional, but often ends up with smaller code
LDFLAGS += -Wl,--gc-sections 
## Relax shrinks code even more, but makes disassembly messy
## LDFLAGS += -Wl,--relax
## LDFLAGS += -Wl,-u,vfprintf -lprintf_flt -lm  ## for floating-point printf
## LDFLAGS += -Wl,-u,vfprintf -lprintf_min      ## for smaller printf
TARGET_ARCH = -mmcu=$(MCU)

## Explicit pattern rules:
##  To make .o files from .c files 
%.o: %.c $(HEADERS) Makefile
	 $(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<

$(TARGET).elf: $(OBJECTS)
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LDLIBS) -o $@

%.hex: %.elf
	 $(OBJCOPY) -j .text -j .data -O ihex $< $@

%.eeprom: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@ 

%.lst: %.elf
	$(OBJDUMP) -S $< > $@

## These targets don't have files named after them
.PHONY: all disassemble disasm eeprom size clean squeaky_clean flash fuses

all: $(TARGET).hex 

debug:
	@echo
	@echo "Source files:"   $(SOURCES)
	@echo "MCU, F_CPU, BAUD:"  $(MCU), $(F_CPU), $(BAUD)
	@echo	

# Optionally create listing file from .elf
# This creates approximate assembly-language equivalent of your code.
# Useful for debugging time-sensitive bits, 
# or making sure the compiler does what you want.
disassemble: $(TARGET).lst

disasm: disassemble

# Optionally show how big the resulting program is 
size:  $(TARGET).elf
	$(AVRSIZE) -C --mcu=$(MCU) $(TARGET).elf

clean:
	rm -f $(TARGET).elf $(TARGET).hex $(TARGET).obj \
	$(TARGET).o $(TARGET).d $(TARGET).eep $(TARGET).lst \
	$(TARGET).lss $(TARGET).sym $(TARGET).map $(TARGET)~ \
	$(TARGET).eeprom $(LCDDIR)/*.o $(ENC28JDIR)/*.o \
  $(LowLvlInit)/*.o $(TCP_IP)/*.o $(UART_DIR)/*.o
  
  
  
squeaky_clean:
	rm -f *.elf *.hex *.obj *.o *.d *.eep *.lst *.lss *.sym *.map *~ *.eeprom

##########------------------------------------------------------##########
##########              Programmer-specific details             ##########
##########           Flashing code to AVR using avrdude         ##########
##########------------------------------------------------------##########
# External Clock:
# Full Swing Crystal Oscillator 16 Mhz
# Maximum startup time.
# |CKSEL3|CKSEL2|CKSEL1|CKSEL0|: 0111
# |SUT1|SUT0|: 11

show_fuses:
	$(AVRDUDE) -B 5 -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) -nv

flash: $(TARGET).hex 
	$(AVRDUDE) -B 5 -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) -U flash:w:$<

## Mega 328 default values
LFUSE = 0x62
FUSE_STRING_DEFAULT = -U lfuse:w:$(LFUSE):m

LFUSE = 0x62
FUSE_STRING_DEFAULT = -U lfuse:w:$(LFUSE):m
SetDefaultFuse: 
	$(AVRDUDE) -B 5 -c $(PROGRAMMER_TYPE) -p $(MCU) \
	           $(PROGRAMMER_ARGS) $(FUSE_STRING_DEFAULT)

#SetDefaultFuse: $(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) $(FUSE_STRING_DEFAULT)


LFUSE_EXT = 0xf7
FUSE_STRING_EXT = -U lfuse:w:$(LFUSE_EXT):m

setfuseextclock: 
	$(AVRDUDE) -B 5 -c $(PROGRAMMER_TYPE) -p $(MCU) \
	           $(PROGRAMMER_ARGS) $(FUSE_STRING_EXT)

#setfuseextclock: $(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(MCU) $(FUSE_STRING_EXT)
