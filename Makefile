# Makefile for AVR C++ projects
# From: https://gist.github.com/rynr/72734da4b8c7b962aa65

# ----- Update the settings of your project here -----

# Hardware
MCU     = atmega328p # see `make show-mcu`
F_CPU   = 8000000UL
PROJECT = Wecker-2

# ----- These configurations are quite likely not to be changed -----

# Binaries
GCC     = avr-gcc
G++     = avr-g++
RM      = rm -f
AVRDUDE = avrdude

# Files
EXT_C   = c
EXT_CPP = cpp
EXT_CXX = cxx
EXT_ASM = asm

OUTPUT_DIR	:= ./build
OBJDIR		:= ./build/obj
SRCDIR		:= ./src

SOURCES	:= $(wildcard $(SRCDIR)/*.$(EXT_C) $(SRCDIR)/*.$(EXT_CPP) $(SRCDIR)/*.$(EXT_CXX))

# ----- No changes should be necessary below this line -----

# Object files
TMP1			:= $(patsubst $(SRCDIR)/%.$(EXT_C), $(OBJDIR)/%.o, $(SOURCES))
TMP2			:= $(patsubst $(SRCDIR)/%.$(EXT_CPP), $(OBJDIR)/%.o, $(TMP1))
OBJECTS			:= $(patsubst $(SRCDIR)/%.$(EXT_CXX), $(OBJDIR)/%.o, $(TMP2))

# TODO explain these flags, make them configurable
CFLAGS = $(INC)
CFLAGS += -Os
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -Wall -Wstrict-prototypes
CFLAGS += -DF_CPU=$(OSC)
CFLAGS += -mmcu=$(MCU)

C++FLAGS = $(INC)
C++FLAGS += -std=c++17
C++FLAGS += -Os
C++FLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
C++FLAGS += -Wall
C++FLAGS += -DF_CPU=$(F_CPU)
C++FLAGS += -mmcu=$(MCU)
C++FLAGS += -I$(SRCDIR)

ASMFLAGS = $(INC)
ASMFLAGS += -Os
ASMFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
ASMFLAGS += -Wall -Wstrict-prototypes
ASMFLAGS += -DF_CPU=$(F_CPU)
ASMFLAGS += -x assembler-with-cpp
ASMFLAGS += -mmcu=$(MCU)

.PHONY: clean help show-mcu
.SECONDARY:

default: $(OUTPUT_DIR)/$(PROJECT).elf

print-%:
	@echo $*=$($*)

%.elf: $(OBJECTS)
	@printf "  LD      $@\n"
	@$(GCC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.$(EXT_C)
	@printf "  CC      $<\n"
	@mkdir -p $(dir $@)
	@$(GCC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.$(EXT_CPP)
	@printf "  CXX     $<\n"
	@mkdir -p $(dir $@)
	@$(G++) $(C++FLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.$(EXT_CXX)
	@printf "  CXX     $<\n"
	@mkdir -p $(dir $@)
	@$(G++) $(C++FLAGS) -c $< -o $@

$(OBJDIR)/%.o : %.$(EXT_ASM)
	@printf "  ASM     $<\n"
	@mkdir -p $(dir $@)
	@$(G++) $(ASMFLAGS) -c $< -o $@

clean:
	$(RM) $(OUTPUT_DIR)/$(PROJECT).elf $(OBJECTS)

help:
	@echo "usage:"
	@echo "  make <target>"
	@echo ""
	@echo "targets:"
	@echo "  clean     Remove any non-source files"
	@echo "  config    Shows the current configuration"
	@echo "  help      Shows this help"
	@echo "  show-mcu  Show list of all possible MCUs"

config:
	@echo "configuration:"
	@echo ""
	@echo "Binaries for:"
	@echo "  C compiler:   $(GCC)"
	@echo "  C++ compiler: $(G++)"
	@echo "  Programmer:   $(AVRDUDE)"
	@echo "  remove file:  $(RM)"
	@echo ""
	@echo "Hardware settings:"
	@echo "  MCU: $(MCU)"
	@echo "  OSC: $(OSC)"
	@echo ""
	@echo "Defaults:"
	@echo "  C-files:   *.$(EXT_C)"
	@echo "  C++-files: *.$(EXT_CPP) & *.$(EXT_CXX)" 
	@echo "  ASM-files: *.$(EXT_ASM)"

show-mcu:
	$(G++) --help=target


