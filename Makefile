##########################################################################################################################
# Improved Makefile for STM32F334 CAN Project
# Target: simplecan
# Generated: January 17, 2026
##########################################################################################################################

######################################
# target
######################################
TARGET = simplecan

######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og

#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES =  \
Core/Src/main.c \
Core/Src/can.c \
Core/Src/gpio.c \
Core/Src/adc.c \
Core/Src/temperature.c \
Core/Src/stm32f3xx_it.c \
Core/Src/stm32f3xx_hal_msp.c \
Core/Src/system_stm32f3xx.c \
Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_can.c \
Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_adc.c \
Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_adc_ex.c \
Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal.c \
Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_rcc.c \
Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_rcc_ex.c \
Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_gpio.c \
Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_dma.c \
Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_cortex.c \
Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_pwr.c \
Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_pwr_ex.c \
Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_flash.c \
Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_flash_ex.c \
Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_i2c.c \
Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_i2c_ex.c \
Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_exti.c

# ASM sources
ASM_SOURCES =  \
STM32CubeIDE/Application/User/Startup/startup_stm32f334c8tx.s

#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
OBJDUMP = $(GCC_PATH)/$(PREFIX)objdump
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
OBJDUMP = $(PREFIX)objdump
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# fpu
FPU = -mfpu=fpv4-sp-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32F334x8

# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-ICore/Inc \
-IDrivers/STM32F3xx_HAL_Driver/Inc \
-IDrivers/STM32F3xx_HAL_Driver/Inc/Legacy \
-IDrivers/CMSIS/Device/ST/STM32F3xx/Include \
-IDrivers/CMSIS/Include

# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = STM32CubeIDE/STM32F334C8TX_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

#######################################
# Phony targets
#######################################
.PHONY: all clean flash flash-openocd erase size disasm help info

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	@echo "CC $<"
	@$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	@echo "AS $<"
	@$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	@echo "LD $@"
	@$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@echo ""
	@echo "Build complete!"
	@$(SZ) $@
	@echo ""

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@echo "HEX $@"
	@$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@echo "BIN $@"
	@$(BIN) $< $@	
	
$(BUILD_DIR):
	mkdir -p $@

#######################################
# Programming targets
#######################################

# Flash using st-flash (texane/stlink)
flash: $(BUILD_DIR)/$(TARGET).bin
	@echo "Flashing $(TARGET).bin to STM32..."
	st-flash write $(BUILD_DIR)/$(TARGET).bin 0x8000000
	@echo "Flash complete!"

# Flash using OpenOCD
flash-openocd: $(BUILD_DIR)/$(TARGET).elf
	@echo "Flashing $(TARGET).elf using OpenOCD..."
	openocd -f interface/stlink.cfg -f target/stm32f3x.cfg \
	        -c "program $(BUILD_DIR)/$(TARGET).elf verify reset exit"
	@echo "Flash complete!"

# Erase the flash
erase:
	@echo "Erasing STM32 flash..."
	st-flash erase
	@echo "Erase complete!"

#######################################
# Analysis targets
#######################################

# Show memory usage
size: $(BUILD_DIR)/$(TARGET).elf
	@echo ""
	@echo "===== Memory Usage (Berkeley format) ====="
	@$(SZ) --format=berkeley $<
	@echo ""
	@echo "===== Memory Usage (SysV format) ====="
	@$(SZ) --format=sysv $<
	@echo ""

# Generate disassembly
disasm: $(BUILD_DIR)/$(TARGET).elf
	@echo "Generating disassembly..."
	@$(OBJDUMP) -D -S $(BUILD_DIR)/$(TARGET).elf > $(BUILD_DIR)/$(TARGET).disasm
	@echo "Disassembly saved to $(BUILD_DIR)/$(TARGET).disasm"

# Show build info
info:
	@echo ""
	@echo "===== Project Information ====="
	@echo "Target:        $(TARGET)"
	@echo "MCU:           STM32F334C8T6"
	@echo "Optimization:  $(OPT)"
	@echo "Debug:         $(DEBUG)"
	@echo "Toolchain:     $(PREFIX)"
	@echo "Build dir:     $(BUILD_DIR)"
	@echo ""
	@echo "C Sources:     $(words $(C_SOURCES)) files"
	@echo "ASM Sources:   $(words $(ASM_SOURCES)) files"
	@echo ""

#######################################
# clean up
#######################################
clean:
	@echo "Cleaning build directory..."
	-rm -fR $(BUILD_DIR)
	@echo "Clean complete!"

#######################################
# Help
#######################################
help:
	@echo ""
	@echo "===== STM32 CAN Project Makefile ====="
	@echo ""
	@echo "Available targets:"
	@echo "  all              - Build firmware (default)"
	@echo "  clean            - Remove all build files"
	@echo ""
	@echo "Programming:"
	@echo "  flash            - Upload to board using st-flash"
	@echo "  flash-openocd    - Upload to board using OpenOCD"
	@echo "  erase            - Erase chip flash"
	@echo ""
	@echo "Analysis:"
	@echo "  size             - Show memory usage statistics"
	@echo "  disasm           - Generate disassembly file"
	@echo "  info             - Show project information"
	@echo ""
	@echo "Examples:"
	@echo "  make             - Build the project"
	@echo "  make flash       - Build and flash to board"
	@echo "  make clean all   - Clean rebuild"
	@echo "  make size        - Check memory usage"
	@echo ""
	@echo "Options:"
	@echo "  DEBUG=1          - Enable debug symbols (default)"
	@echo "  DEBUG=0          - Release build without debug"
	@echo "  OPT=-O2          - Change optimization level"
	@echo "  GCC_PATH=<path>  - Specify toolchain path"
	@echo ""

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***
