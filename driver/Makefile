MCU        = atmega328p
F_CPU      = 16000000UL
BAUD       = 115200
PORT       = /dev/ttyUSB0
PROGRAMMER = arduino

CFLAGS = -Iinclude -Idriver -std=gnu11 -Wall -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU)

# 自动收集 driver/ 里的所有 .c 文件
SRCS := $(wildcard driver/*.c)
OBJS := $(SRCS:.c=.o)

all: main.elf

main.elf: $(OBJS)
	avr-gcc $(CFLAGS) $(OBJS) -o $@

driver/%.o: driver/%.c
	avr-gcc $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) main.elf main.hex

upload: main.hex
	avrdude -F -V -c $(PROGRAMMER) -p $(MCU) -P $(PORT) -b $(BAUD) -U flash:w:main.hex

main.hex: main.elf
	avr-objcopy -O ihex -R .eeprom main.elf main.hex

.PHONY: all clean upload
