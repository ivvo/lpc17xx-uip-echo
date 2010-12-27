PATH:=$(PATH):/usr/local/arm-cortex-codesourcery/bin
BIN = server
BIN_OBJECTS= server.o
LIB = libuip.a
LIB_OBJECTS = psock.o  timer.o  uip_arp.o  uip.o  uip-fw.o  uiplib.o  uip-neighbor.o  uip-split.o
LIBS= -lc -lm -luip -llpc1768
CFLAGS = -mcpu=cortex-m3 -mthumb -msoft-float -O0 -Wall -I. -I./include/ -I/usr/local/lpc1768/include
LDFLAGS = -mcpu=cortex-m3 -mthumb -msoft-float -O0 -Wl,-Map=$(BIN).map -T /usr/local/lpc1768/lpc1768.ld -L. -L./lib/ -L/usr/local/lpc1768/ $(LIBS) 

CC = arm-none-eabi-gcc
AS = arm-none-eabi-as
LD = arm-none-eabi-ld
AR = arm-none-eabi-ar
RM = rm -f 
STRIP = arm-none-eabi-strip -s
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

all: $(BIN)
	gcc client.c -o client

$(BIN): $(LIB) $(BIN_OBJECTS)
	$(CC) $(BIN_OBJECTS) $(LDFLAGS) -o $(BIN).elf
	$(OBJCOPY) -O binary -j .text -j .data $(BIN).elf $(BIN).bin

$(LIB): $(LIB_OBJECTS)
	$(AR) -r $(LIB) $(LIB_OBJECTS)

clean:
stats: $(BIN).elf
	$(SIZE) $(BIN).elf

clean:
	$(RM) $(LIB)
	$(RM) $(BIN).elf
	$(RM) $(BIN).bin
	$(RM) $(BIN).map
	$(RM) client
	$(RM) *.o

install:
	cp $(BIN).bin /media/MBED/ && sync

.c.o :
	$(CC) $(CFLAGS) -c $<
