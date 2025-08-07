TARGET = main
MCU = atmega328p
F_CPU = 16000000UL
BAUD = 115200
UPLOAD_PORT = /dev/ttyACM0
PROGRAMMER = arduino
CC = avr-gcc
OBJCOPY = avr-objcopy
# 包含头文件目录
CFLAGS   = -Iinclude -std=gnu11 -Wall -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU)
CPPFLAGS = -Iinclude -std=gnu++17 -Wall -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU)
LDFLAGS  = -mmcu=$(MCU)
SRC_C   = src/twi.c src/usart.c
SRC_CPP = src/mpu6050_dual.cpp src/main.cpp
OBJ_C   = $(SRC_C:.c=.o)
OBJ_CPP = $(SRC_CPP:.cpp=.o)

.PHONY: all clean upload
all: $(TARGET).hex

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@
$(TARGET).elf: $(OBJ_C) $(OBJ_CPP)
	$(CC) $(LDFLAGS) $^ -o $@
$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

upload: $(TARGET).hex
	@echo "Uploading $(TARGET).hex to $(UPLOAD_PORT)..."
	avrdude -v \
	  -p$(MCU) \
	  -c$(PROGRAMMER) \
	  -P$(UPLOAD_PORT) \
	  -b$(BAUD) \
	  -D \
	  -Uflash:w:$(TARGET).hex:i
clean:
	rm -f $(OBJ_C) $(OBJ_CPP) $(TARGET).elf $(TARGET).hex







