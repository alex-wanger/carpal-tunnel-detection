# ==== Board / Tools ====
MCU  = atmega328p
F_CPU= 16000000UL
PORT = /dev/ttyACM0
BAUD = 115200
PROGRAMMER = arduino

# ==== Dirs ====
SRCDIR   = driver
INCDIR   = include
BUILDDIR = build

# ==== Sources / Objects ====
# 自动抓取 driver/*.c
SRCS  := $(wildcard $(SRCDIR)/*.c)
# 把 driver/xxx.c 映射到 build/xxx.o
OBJS  := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))

TARGET = main
ELF    = $(TARGET).elf
HEX    = $(TARGET).hex

# ==== Flags ====
CFLAGS  = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall -Wextra -std=gnu11 -I$(INCDIR)
LDFLAGS = -mmcu=$(MCU)

# ==== Rules ====
all: $(HEX)

$(HEX): $(ELF)
	avr-objcopy -O ihex -R .eeprom $< $@

$(ELF): $(OBJS)
	avr-gcc $(LDFLAGS) $^ -o $@

# build/xxx.o <- driver/xxx.c
$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	avr-gcc $(CFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

upload: $(HEX)
	avrdude -v -p $(MCU) -c $(PROGRAMMER) -P $(PORT) -b $(BAUD) -D -U flash:w:$(HEX):i

clean:
	rm -rf $(BUILDDIR) $(ELF) $(HEX)

# 调试：看看抓到哪些源文件
list:
	@echo "SRCS = $(SRCS)"
	@echo "OBJS = $(OBJS)"
