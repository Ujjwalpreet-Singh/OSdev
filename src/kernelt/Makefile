TARGET_ASMFLAGS += -f elf
TARGET_CFLAGS += -ffreestanding -nostdlib -I.
TARGET_LIBS += -lgcc
TARGET_LINKFLAGS += -T linker.ld -nostdlib

HEADERS_C = $(wildcard *.h) \
			$(wildcard */*.h) \
			$(wildcard */*/*.h) \
			$(wildcard */*/*/*.h)

SOURCES_C = $(wildcard *.c) \
			$(wildcard */*.c) \
			$(wildcard */*/*.c) \
			$(wildcard */*/*/*.c) \

OBJECTS_C = $(patsubst %.c, $(BUILD_DIR)/kernelt/c/%.obj, $(SOURCES_C))

HEADERS_ASM = $(wildcard *.inc) \
			  $(wildcard */*.inc) \
			  $(wildcard */*/*.inc) \
			  $(wildcard */*/*/*.inc) \

SOURCES_ASM = $(wildcard *.asm) \
			  $(wildcard */*.asm) \
			  $(wildcard */*/*.asm) \
			  $(wildcard */*/*/*.asm)


OBJECTS_ASM = $(patsubst %.asm, $(BUILD_DIR)/kernelt/asm/%.obj, $(SOURCES_ASM))

.PHONY: all kernelt clean always

all: kernelt

kernelt: $(BUILD_DIR)/kernelt.bin

$(BUILD_DIR)/kernelt.bin: $(OBJECTS_ASM) $(OBJECTS_C)
	@$(TARGET_LD) $(TARGET_LINKFLAGS) -Wl,-Map=$(BUILD_DIR)/kernelt.map -o $@ $^ $(TARGET_LIBS)
	@echo "--> Created:  kernelt.bin"

$(BUILD_DIR)/kernelt/c/%.obj: %.c $(HEADERS_C)
	@mkdir -p $(@D)
	@$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/kernelt/asm/%.obj: %.asm $(HEADERS_ASM)
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

arch/i686/isrs_gen.c arch/i686/isrs_gen.inc:
	@$(SOURCE_DIR)/build_tools/generate_isrs.sh $@

clean:
	rm -f $(BUILD_DIR)/kernelt.bin