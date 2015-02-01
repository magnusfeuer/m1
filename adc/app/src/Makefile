#
# Makefile for Magen ADC 
#

# MCU name and submodel
MCU = arm7tdmi
SUBMDL = LPC2129

#THUMB    = -mthumb
#THUMB_IW = -mthumb-interwork


## Create ROM-Image (final)
RUN_MODE=ROM_RUN
## Create RAM-Image (debugging)
#RUN_MODE=RAM_RUN

# Output format. (can be srec, ihex, binary)
FORMAT = ihex
ADC_SWVSN    = 1

TARGET = $(OUTDIR)/$(APP)_$(HWVSN)_$(SWVSN)

OUTDIR = ../out
OBJDIR = ../obj/$(APP)_$(HWVSN)
LSTDIR = ../listing
INCDIR = ../include

# List C source files here. (C dependencies are automatically generated.)
# use file-extension c for "c-only"-files
COMMON_SRC = \


ADC_SRC = \
	time.c \
	uart.c \
	mux.c \
	ad.c \
	digi.c \
	can.c \
	gpio.c \
	power.c \
	adc_main.c


# List Assembler source files here which must be assembled in ARM-Mode..
ASRC = lpc2129.s

# Optimization level, can be [0, 1, 2, 3, s]. 
# 0 = turn off optimization. s = optimize for size.
# (Note: 3 is not always the best optimization level. See avr-libc FAQ.)
OPT = s

# Debugging format.
# Native formats for AVR-GCC's -g are stabs [default], or dwarf-2.
# AVR (extended) COFF requires stabs, plus an avr-objcopy run.
#DEBUG = stabs
DEBUG = dwarf-2

# List any extra directories to look for include files here.
#     Each directory must be seperated by a space.

# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD = -std=gnu99

# Place -D or -U options for C here
CDEFS =  -D$(RUN_MODE)

# Place -I options here
CINCS =

# Place -D or -U options for ASM here
ADEFS =  -D$(RUN_MODE)


# Compiler flags.
#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
#
# Flags for C and C++ (arm-elf-gcc/arm-elf-g++)
CFLAGS = -g$(DEBUG) 
CFLAGS += $(CDEFS) $(CINCS)
CFLAGS += -O$(OPT)
CFLAGS += -D$(DEVICE)=$(HWVSN)
CFLAGS += -Wall -Wcast-align -Wcast-qual -Wimplicit
CFLAGS += -Wpointer-arith -Wswitch
CFLAGS += -Wredundant-decls -Wreturn-type -Wshadow -Wunused
CFLAGS += -Wa,-adhlns=$(<:%.c=$(LSTDIR)/%.lst)
CFLAGS += -I../include

# flags only for C
CONLYFLAGS  = -Wstrict-prototypes -Wmissing-declarations
CONLYFLAGS += -Wmissing-prototypes -Wnested-externs 
CONLYFLAGS += $(CSTANDARD)

# Assembler flags.
#  -Wa,...:   tell GCC to pass this to the assembler.
#  -ahlms:    create listing
#  -gstabs:   have the assembler create line number information; note that
#             for use in COFF files, additional information about filenames
#             and function names needs to be present in the assembler source
#             files -- see avr-libc docs [FIXME: not yet described there]
ASFLAGS = $(ADEFS) -Wa,-adhlns=$(<:.s=.lst),-g$(DEBUG)

#Additional libraries.
MATH_LIB = -lm

# Linker flags.
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
#LDFLAGS = -nostartfiles -Wl,"-Map $(TARGET).map"
#LDFLAGS = -nostartfiles -Wl,"-b"
LDFLAGS = -nostartfiles 
LDFLAGS += $(MATH_LIB)
LDFLAGS += -lc -lgcc

# Set Linker-Script Depending On Selected Memory
ifeq ($(RUN_MODE),RAM_RUN)
LDFLAGS +=-T $(SUBMDL)-RAM.ld
else 
LDFLAGS +=-T $(SUBMDL)-ROM.ld
endif

# Define directories, if needed.
## DIRARM = c:/WinARM/
## DIRARMBIN = $(DIRAVR)/bin/
## DIRAVRUTILS = $(DIRAVR)/utils/bin/

# Define programs and commands.
SHELL = sh
CC = arm-elf-gcc
CPP = arm-elf-g++
OBJCOPY = arm-elf-objcopy
OBJDUMP = arm-elf-objdump
SIZE = arm-elf-size
NM = arm-elf-nm
REMOVE = rm -f
COPY = cp

# Define all object files.
ADC_OBJ = $(ASRC:%.s=$(OBJDIR)/%.o) $(ADC_SRC:%.c=$(OBJDIR)/%.o) 

# Define all listing files.
ADC_LST = $(ADC_SRC:%.c=%.lst) $(ASRC:%.s=$(LSTDIR)/%.lst) 

# Compiler flags to generate dependency files.
### GENDEPFLAGS = -Wp,-M,-MP,-MT,$(*F).o,-MF,.dep/$(@F).d
GENDEPFLAGS = -MD -MP -MF .dep/$(@F).d

# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS = -mcpu=$(MCU) $(THUMB_IW) -I. $(CFLAGS) $(GENDEPFLAGS)
ALL_ASFLAGS = -mcpu=$(MCU) $(THUMB_IW) -I. -x assembler-with-cpp $(ASFLAGS)

# Default target.
all: adc_v1

build: elf ihex lss sym

adc_v1:
	$(MAKE) build DEVICE=ADC HWVSN=1 SWVSN=$(ADC_SWVSN) APP=adc


elf:  $(TARGET).elf
ihex: $(TARGET).ihex
lss:  $(TARGET).lss 
sym:  $(TARGET).sym

.PRECIOUS: $(ADC_OBJ)

# Create final output files (.ihex, .eep) from ELF output file.
# TODO: handling the .eeprom-section should be redundant
%.ihex: %.elf
	$(OBJCOPY) -O $(FORMAT) $< $@

# Create extended listing file from ELF output file.
# testing: option -C
%.lss: %.elf
	$(OBJDUMP) -h -S -C $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	$(NM) -n $< > $@

# Link: create ELF output file from object files.
$(OUTDIR)/adc_%.elf: $(ADC_OBJ)
	$(CC) $(THUMB) $(ALL_CFLAGS) $(ADC_OBJ) --output $@ $(LDFLAGS)


# Compile: create object files from C source files. ARM/Thumb
$(OBJDIR)/%.o: %.c
	$(CC) -c $(THUMB) $(ALL_CFLAGS) $(CONLYFLAGS) $< -o $@ 

# Assemble: create object files from assembler source files. ARM/Thumb
$(OBJDIR)/%.o : %.s
	$(CC) -c $(THUMB) $(ALL_ASFLAGS) $< -o $@

clean:
	rm -f $(ADC_OBJ)
	rm -f $(ADC_LST)


# Include the dependency files.
include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)


# Listing of phony targets.
.PHONY : all build elf ihex lss sym clean clean_list

