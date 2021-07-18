##########################################################################################################################
# trusted_domain GCC compiler Makefile
##########################################################################################################################

# ------------------------------------------------
# Generic Makefile (based on gcc)
# ------------------------------------------------

######################################
# target
######################################
TARGET = trusted_fw
######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og


PROJECTBASE = $(PWD)
override PROJECTBASE    := $(abspath $(PROJECTBASE))
TOP_DIR = $(PROJECTBASE)


#######################################
# binaries
#######################################
CROSS_COMPILE = riscv-
CC        = $(CROSS_COMPILE)gcc
AS        = $(CROSS_COMPILE)gcc -x assembler-with-cpp
OBJCOPY   = $(CROSS_COMPILE)objcopy
OBJDUMP   = $(CROSS_COMPILE)objdump
AR        = $(CROSS_COMPILE)ar
SZ        = $(CROSS_COMPILE)size
LD        = $(CROSS_COMPILE)ld
HEX       = $(OBJCOPY) -O ihex
BIN       = $(OBJCOPY) -O binary -S
GDB       = $(CROSS_COMPILE)gdb

#######################################
# paths
#######################################
# firmware library path
PERIFLIB_PATH =

# Build path
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

######################################
# source
######################################
# C sources
C_SOURCES =  \
		${wildcard $(TOP_DIR)/FreeRTOS-Kernel/*.c} \
		${wildcard $(TOP_DIR)/FreeRTOS-Kernel/portable/GCC/RISC-V/*.c} \
		${wildcard $(TOP_DIR)/FreeRTOS-Kernel/portable/MemMang/heap_4.c} \
		${wildcard $(TOP_DIR)/driver/*.c} \
		${wildcard $(TOP_DIR)/riscv/*.c} \
		${wildcard $(TOP_DIR)/*.c}

# ASM sources
ASM_SOURCES =  \
		${wildcard $(TOP_DIR)/*.S} \
		${wildcard $(TOP_DIR)/FreeRTOS-Kernel/portable/GCC/RISC-V/*.S}

######################################
# firmware library
######################################
PERIFLIB_SOURCES =


#######################################
# CFLAGS
#######################################
MCU = -march=rv64imafdc -mcmodel=medany -msmall-data-limit=8 -fmessage-length=0 -fsigned-char

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS = 

# AS includes
AS_INCLUDES = \
		-I $(TOP_DIR)/riscv \
		-I $(TOP_DIR)/driver

# C includes
C_INCLUDES = \
		-I $(TOP_DIR)/FreeRTOS-Kernel/include \
		-I $(TOP_DIR)/FreeRTOS-Kernel/portable/GCC/RISC-V \
		-I $(TOP_DIR)/riscv \
		-I $(TOP_DIR)/driver \
		-I $(TOP_DIR)


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)"

#######################################
# LDFLAGS
#######################################
# link script
LD_FILE = link.lds
LDSCRIPT = $(PROJECTBASE)/$(LD_FILE)

# libraries
LIBS = -lm 
LIBDIR =
LDFLAGS = $(MCU) -nostartfiles -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).lst


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(OBJ_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(OBJ_DIR)/,$(notdir $(ASM_SOURCES:.S=.o)))
vpath %.S $(sort $(dir $(ASM_SOURCES)))

$(OBJ_DIR)/%.o: %.c Makefile | $(OBJ_DIR)
	@echo CC $(notdir $@)
	@$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(OBJ_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(OBJ_DIR)/%.o: %.S Makefile | $(OBJ_DIR)
	@echo AS $(notdir $@)
	@$(AS) -c $(ASFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) $(LDSCRIPT) Makefile 
	@echo LD $(notdir $@)
	@$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@echo OBJCOPY $(notdir $@)
	@$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@echo OBJCOPY $(notdir $@)
	@$(BIN) $< $@

$(BUILD_DIR)/%.lst: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@echo OBJDUMP $(notdir $@)
	@$(OBJDUMP) --source --demangle --disassemble --reloc --wide $< > $@
	@$(SZ) --format=berkeley $<

$(BUILD_DIR):
	mkdir $@

ifeq ($(OBJ_DIR), $(wildcard $(OBJ_DIR)))
else
$(OBJ_DIR):$(BUILD_DIR)
	mkdir $@
endif

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)

#######################################
# use gdb debug
#######################################
debug:
	$(GDB) -x $(BUILD_DIR)/../gdb.sh

#######################################
# dependencies
#######################################
#-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

# *** EOF ***
