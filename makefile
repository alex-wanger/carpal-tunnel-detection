# ========= Project Config =========
MCU     := atmega328p
F_CPU   := 16000000UL
TARGET  := main

# 串口与烧录设置（按需修改）
PORT    := /dev/ttyACM0
BAUD    := 115200
PROGRAMMER := arduino
AVRDUDE_MCU := m328p

# 头文件路径
INCLUDES := -Iinclude -Isrc

# ========= Toolchain =========
CC      := avr-gcc
CXX     := avr-g++
OBJCOPY := avr-objcopy
SIZE    := avr-size
AVRDUDE := avrdude

# 编译参数
CFLAGS   := $(INCLUDES) -std=gnu11 -Wall -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU)
CXXFLAGS := $(INCLUDES) -std=gnu++17 -Wall -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU) -fno-exceptions -fno-rtti
LDFLAGS  := -mmcu=$(MCU)

# ========= Sources / Objects (auto) =========
CSRC    := $(wildcard src/*.c)
CXXSRC  := $(wildcard src/*.cpp)
OBJS    := $(CSRC:.c=.o) $(CXXSRC:.cpp=.o)

# ========= Default target =========
all: $(TARGET).hex

# 生成 ELF
$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@
	$(SIZE) --mcu=$(MCU) --format=avr $@

# C 源文件编译
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# C++ 源文件编译
src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 生成 HEX
$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

# ========= Flash (upload) =========
upload: $(TARGET).hex
	$(AVRDUDE) -c $(PROGRAMMER) -P $(PORT) -p $(AVRDUDE_MCU) -b $(BAUD) -U flash:w:$<

# ========= Clean =========
clean:
	rm -f src/*.o $(TARGET).elf $(TARGET).hex

# 便捷别名
make: all

.PHONY: all upload clean make
