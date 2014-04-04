OBJS=main.o\
     usart.o\
     led.o\
     rf.o

all: firmware.hex

%.o: %.c
	avr-gcc -c -DDEBUG -Wall -Werror -std=c99 -flto -Os -mmcu=atmega128rfa1 -DF_CPU=16000000 $< -o $@

firmware.hex: $(OBJS)
	avr-gcc -flto -Os -mmcu=atmega128rfa1 $(OBJS) -o firmware.elf
	avr-objcopy -I elf32-avr -O ihex firmware.elf firmware.hex
	avr-size --mcu=atmega128rfa1 -C firmware.elf

clean:
	rm -f $(OBJS) firmware.hex firmware.elf

fuses:
	avrdude -c usbasp -P usb -B 10 -p m128rfa1 -U lfuse:w:0xe6:m -U hfuse:w:0x91:m -U efuse:w:0xfe:m 

flash:
	avrdude -c usbasp -P usb -B 10 -p m128rfa1 -U flash:w:firmware.hex
