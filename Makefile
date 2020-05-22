mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(notdir $(patsubst %/,%,$(dir $(mkfile_path))))

PROJECT_NAME := $(CURRENT_DIR)
SOURCES := \
main.cpp \
lib/avr-misc/avr-eeprom.cpp \
./lib/avr-misc/avr-misc.cpp \
./lib/avr-oneWire/oneWire.cpp \
./lib/avr-debug/debug.cpp \
./lib/avr-debug/RAM_guard.cpp \
./lib/DS1820/DS1820.cpp \
./disp7seg.cpp \

#MCU := atmega88p
MCU := atmega328p
ADAPTER := usbasp

OUTPUT_DIR=bin
CC=avr-g++
CFLAGS=-flto -O -Wextra -Wall -std=c++11 -x c++ -save-temps=obj
LD_FLAGS=-nodefaultlibs
HEADERS=/usr/lib/avr/include
MORE_HEADERS=include lib
#DEFINES=__AVR_ATmega88PA__ F_CPU=16000000UL
DEFINES=__AVR_ATmega328P__ F_CPU=16000000UL


LIBS=c gcc $(MCU)
INCLUDES=$(foreach dir,$(HEADERS),-I$(dir)) $(foreach dir,$(MORE_HEADERS),-I$(dir))
DEFS=$(foreach def,$(DEFINES),-D$(def))
LD_LIBS=$(foreach lib,$(LIBS),-l$(lib))
OBJECTS=$(SOURCES:.cpp=.o)
OUTPUT_FILENAME=$(OUTPUT_DIR)/$(PROJECT_NAME).elf
HEX_FILE=$(OUTPUT_DIR)/$(PROJECT_NAME).hex


all: before $(OBJECTS) $(OUTPUT_FILENAME) after

%.o: %.cpp
	$(CC) -c $(CFLAGS) $(INCLUDES) $(DEFS) $< -o $@

$(OUTPUT_FILENAME): $(OBJECTS)
	$(CC) -o $(OUTPUT_FILENAME) $(OBJECTS) $(LD_FLAGS) -mmcu=$(MCU) $(LD_LIBS)

before:
	mkdir -p bin
	rm -f $(HEX_FILE)

after: $(OUTPUT_FILENAME)
	avr-strip --strip-debug -R .comment $(OUTPUT_FILENAME)
	avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex $(OUTPUT_FILENAME) $(HEX_FILE)
	avr-size -C --mcu=$(MCU) -d $(OUTPUT_FILENAME)

	
.PHONY clean:
	rm -rf $(OBJECTS) 
	rm -f `find . -name '*.ii'`
	rm -f `find . -name '*.s'`
	rm -rf ./bin

install: $(HEX_FILE)
	avrdude -p $(MCU) -c $(ADAPTER) -V -U flash:w:$(HEX_FILE)
