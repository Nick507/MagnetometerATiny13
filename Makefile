SKETCH  = MagnetometerATiny13.c
TARGET  = MagnetometerATiny13

# Compiler Options
DEVICE  = attiny13a
CLOCK   = 9600000

# Programmer Options
PROGRMR = usbasp
TGTDEV  = attiny13
LFUSE   = 0x7a
HFUSE   = 0xff

AVRPATH = C:\\Users\\Nikolay\\AppData\\Local\\Arduino15\\packages\\arduino\\tools\\avr-gcc\\7.3.0-atmel3.6.1-arduino7/bin
# Commands
AVRDUDE = C:\Users\Nikolay\AppData\Local\Arduino15\packages\MicroCore\tools\avrdude\7.1-arduino.1/bin/avrdude -c $(PROGRMR) -p $(TGTDEV)
COMPILE = $(AVRPATH)/avr-gcc -Wall -Os -flto -mmcu=$(DEVICE) -DF_CPU=$(CLOCK) -x c++ $(SKETCH)
#COMPILE = avr-gcc -Wall -Os -mmcu=$(DEVICE) -DF_CPU=$(CLOCK) -x c++ $(SKETCH)
CLEAN   = #del *.lst *.obj *.cof *.list *.map *.eep.hex *.o *.s

upload:	hex
	@echo "Uploading to $(DEVICE) ..."
	@$(AVRDUDE) -U flash:w:$(TARGET).hex:i

# Symbolic Targets
help:
	@echo "Use the following commands:"
	@echo "make all       compile and build $(TARGET).bin/.hex/.asm for $(DEVICE)"
	@echo "make hex       compile and build $(TARGET).hex for $(DEVICE)"
	@echo "make asm       compile and disassemble to $(TARGET).asm for $(DEVICE)"
	@echo "make bin       compile and build $(TARGET).bin for $(DEVICE)"
	@echo "make upload    compile and upload to $(DEVICE) using $(PROGRMR)"
	@echo "make fuses     burn fuses of $(DEVICE) using $(PROGRMR) programmer"
	@echo "make install   compile, upload and burn fuses for $(DEVICE)"
	@echo "make clean     remove all build files"

all:	buildbin buildhex buildasm removetemp size

bin:  buildbin removetemp size

hex:	buildbin buildhex size

asm:	buildbin buildasm removetemp size removebin

install: upload fuses

fuses:
	@echo "Burning fuses of $(DEVICE) ..."
	@$(AVRDUDE) -U lfuse:w:$(LFUSE):m  -U hfuse:w:$(HFUSE):m

clean:
	@echo "Cleaning all up ..."
	@$(CLEAN)
	@cmd /C del $(TARGET).bin $(TARGET).hex $(TARGET).asm

buildbin:
	@echo "Building $(TARGET).bin for $(DEVICE) @ $(CLOCK)Hz ..."
	@$(COMPILE) -o $(TARGET).bin

buildhex:
	@echo "Building $(TARGET).hex ..."
	@avr-objcopy -j .text -j .data -O ihex $(TARGET).bin $(TARGET).hex

buildasm:
	@echo "Disassembling to $(TARGET).asm ..."
	@avr-objdump -d $(TARGET).bin > $(TARGET).asm

size:
	@$(AVRPATH)/avr-size -d $(TARGET).bin
#	@echo "FLASH: $(shell $(AVRPATH)/avr-size -d $(TARGET).bin | awk '/[0-9]/ {print $$1 + $$2}') bytes"
#	@echo "SRAM:  $(shell $(AVRPATH)/avr-size -d $(TARGET).bin | awk '/[0-9]/ {print $$2 + $$3}') bytes"

removetemp:
	@echo "Removing temporary files ..."
	@$(CLEAN)

removebin:
	@echo "Removing $(TARGET).bin ..."
	@cmd /C del $(TARGET).bin
