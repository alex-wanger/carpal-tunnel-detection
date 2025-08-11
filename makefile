# 工程名称和器件配置
TARGET     = main
MCU        = atmega328p
F_CPU      = 16000000UL
BAUD       = 115200

# 工具链
CC         = avr-gcc
OBJCOPY    = avr-objcopy
PROGRAMMER = arduino
UPLOAD_PORT= /dev/ttyACM0    # 根据实际情况改为 ttyUSB0、COMx 等

# 编译选项
CFLAGS   = -Iinclude -std=gnu11 -Wall -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU)
CPPFLAGS = -Iinclude -std=gnu++17 -Wall -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU)
LDFLAGS  = -mmcu=$(MCU)

# 源文件列表
SRC_C    = src/twi.c \
           src/usart.c \
           src/biquad.c

SRC_CPP  = src/mpu6050_dual.cpp \
           src/main.cpp

# 目标文件
OBJ_C    = $(SRC_C:.c=.o)
OBJ_CPP  = $(SRC_CPP:.cpp=.o)

.PHONY: all clean upload

# 1) 默认：编译并生成 HEX
all: $(TARGET).hex

# 2) C 文件编译
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 3) C++ 文件编译
%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@

# 4) 链接 ELF
$(TARGET).elf: $(OBJ_C) $(OBJ_CPP)
	$(CC) $(LDFLAGS) $^ -o $@

# 5) 生成 Intel HEX
$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

# 6) 烧录：将 HEX 写入 AVR
upload: $(TARGET).hex
	@echo "Uploading $(TARGET).hex to $(UPLOAD_PORT)..."
	avrdude -v \
		-p$(MCU) \
		-c$(PROGRAMMER) \
		-P$(UPLOAD_PORT) \
		-b$(BAUD) \
		-D \
		-Uflash:w:$(TARGET).hex:i

# 7) 清理构建文件
clean:
	rm -f $(OBJ_C) $(OBJ_CPP) $(TARGET).elf $(TARGET).hex
