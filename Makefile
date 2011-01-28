GCCFLAGS=-g -Os -Wall -mmcu=atmega168 
LINKFLAGS=-Wl,-u,vfprintf -lprintf_flt -Wl,-u,vfscanf -lscanf_flt -lm
AVRDUDEFLAGS=-c avr109 -p m168 -b 115200 -P /dev/ttyUSB0
LINKOBJECTS=../hd44780_lcd/hd44780_lcd.o

all:	avr_tictactoe-upload

avr_tictactoe.hex:	avr_tictactoe.c	
	make -C ../hd44780_lcd
	avr-gcc ${GCCFLAGS} ${LINKFLAGS} -o avr_tictactoe.o avr_tictactoe.c ${LINKOBJECTS}
	avr-objcopy -j .text -O ihex avr_tictactoe.o avr_tictactoe.hex
	
avr_tictactoe.ass:	avr_tictactoe.hex
	avr-objdump -S -d avr_tictactoe.o > avr_tictactoe.ass
	
avr_tictactoe-upload:	avr_tictactoe.hex
	avrdude ${AVRDUDEFLAGS} -U flash:w:avr_tictactoe.hex:a

build:	avr_tictactoe.c	
	make -C ../hd44780_lcd
	avr-gcc ${GCCFLAGS} ${LINKFLAGS} -o avr_tictactoe.o avr_tictactoe.c ${LINKOBJECTS}
