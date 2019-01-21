# ----------------------------------------
#
# Chipdesign 2013 - C Firmware
#
# Author: Sebastian Lesse
#
# ----------------------------------------


# --------- Toolchain Path

CROSS_PREFIX		:= 
TOOLCHAIN_BIN_PATH	:= /usr/bin
TOOLCHAIN_INC_PATH	:= /usr/include
TOOLCHAIN_LIB_PATH	:=

CC			:= '$(TOOLCHAIN_BIN_PATH)/$(CROSS_PREFIX)gcc'
AS			:= '$(TOOLCHAIN_BIN_PATH)/$(CROSS_PREFIX)as'
AR			:= '$(TOOLCHAIN_BIN_PATH)/$(CROSS_PREFIX)ar' -r
LD			:= '$(TOOLCHAIN_BIN_PATH)/$(CROSS_PREFIX)ld'
NM          		:= '$(TOOLCHAIN_BIN_PATH)/$(CROSS_PREFIX)nm'
OBJDUMP			:= '$(TOOLCHAIN_BIN_PATH)/$(CROSS_PREFIX)objdump'
OBJCOPY			:= '$(TOOLCHAIN_BIN_PATH)/$(CROSS_PREFIX)objcopy'
SIZE			:= '$(TOOLCHAIN_BIN_PATH)/$(CROSS_PREFIX)size'

RM			:= rm -rf
MK			:= mkdir
CP			:= cp

ECHO 			:=

# --------- Message Output

MSG_COMPILING		:= Compiling
MSG_LINKING		:= Linking to
MSG_PROG_LOCATION	:= Your programm can be found at
MSG_LISTING		:= Generating Disassembly
MSG_FINISH		:= ----- Make done -----


# --------- Application Properties (Target / Working dir / ...)

OBJECT_DIRECTORY	:= obj
OUTPUT_DIRECTORY	:= program
APP_PATH		:= 
FORMAT			:= ihex
FREQUENCY		:= 10000000UL
VERSION			:= 1.0

TARGET			:= SmartHomeClient


# --------- Include Path

INC_PATH +=
INC_PATH += -I $(APP_PATH)
INC_PATH += -I $(TOOLCHAIN_INC_PATH)


# --------- Source File List

CSRC += 
CSRC +=	$(APP_PATH)/shc_command_interface.c
CSRC +=	$(APP_PATH)/shc_common_string.c
CSRC +=	$(APP_PATH)/shc_debug_interface.c
CSRC +=	$(APP_PATH)/shc_file_interface.c
CSRC +=	$(APP_PATH)/shc_gpio_interface.c
CSRC +=	$(APP_PATH)/shc_mqtt_interface.c
CSRC +=	$(APP_PATH)/shc_qeue_interface.c
CSRC +=	$(APP_PATH)/shc_spi_interface.c

CSRC +=	$(APP_PATH)/smart_home_client.c


# --------- Compiler Flags

CFLAGS +=

ifdef MCU_NAME
CFLAGS += -mmcu=$(MCU_NAME)
endif

ifdef FREQUENCY
CFLAGS += -DF_CPU=$(FREQUENCY)
endif

CFLAGS += -Wall
CFLAGS += -Os
CFLAGS += -funsigned-char
CFLAGS += -funsigned-bitfields
CFLAGS += -fpack-struct
CFLAGS += -fshort-enums
CFLAGS += -lm


LIBPATH +=
LIBPATH += -L.

LDFLAGS +=
#LDFLAGS += --section-start init=0
LDFLAGS += -Wl,-Map=$(OBJECT_DIRECTORY)/$(TARGET).map,--cref
LDFLAGS += -L$(TOOLCHAIN_LIB_PATH)

# --------- Loader FLags

LOADER_FLAGS		:=
LOADER_FLAGS		+= -com6
LOADER_FLAGS		+= -f10


# --------- Make Targets 

C_DEPENDENCY_FILES 	:= $(CSRC:.c=.o)
C_ELF_FILES		:= $(CSRC:.c=.elf)
C_OBJ_FILES		:= $(CSRC:.c=.o)
OBJ_FILES		:= $(addprefix $(OBJECT_DIRECTORY)/,$(notdir $(C_OBJ_FILES)))
ELF_FILES		:= $(addprefix $(OBJECT_DIRECTORY)/,$(notdir $(C_ELF_FILES)))
OBJ  			:= $(C_ELF_FILES)

# --------- 

all: $(TARGET).hex $(TARGET).lss
	$(ECHO) $(CP) $(OBJECT_DIRECTORY)/$(TARGET).hex v$(VERSION)/$(TARGET)_v$(VERSION).hex
	$(ECHO) $(CP) $(OBJECT_DIRECTORY)/$(TARGET).lss v$(VERSION)/$(TARGET)_v$(VERSION).lss
	@echo $(MSG_PROG_LOCATION) v$(VERSION)/$(TARGET)_v$(VERSION).hex
	@echo $(MSG_FINISH)
	
clean:
	@echo Cleaning project
	$(ECHO) $(RM) $(OBJECT_DIRECTORY)
	$(ECHO) $(RM) v$(VERSION)

# --------- 

%.hex: %.o
	@echo Generating $(OBJECT_DIRECTORY)/$(TARGET).hex
	$(OBJCOPY) -O $(FORMAT) $(OBJECT_DIRECTORY)/$< $(OBJECT_DIRECTORY)/$@
	$(ECHO) $(SIZE) --mcu=$(MCU_NAME) -C $(OBJECT_DIRECTORY)/$(TARGET).o 
	
%.lss:
	@echo $(MSG_LISTING)
	$(ECHO) $(OBJDUMP) -h -S $(OBJECT_DIRECTORY)/$(TARGET).o > $(OBJECT_DIRECTORY)/$(TARGET).lss

%.o:
	@echo Generating Object from: $<
	$(ECHO) $(CC) $(CSRC) $< -o $(OBJECT_DIRECTORY)/$(notdir $@)
	
$(OBJECT_DIRECTORY):
	@echo Creating Build directory: $(OBJECT_DIRECTORY)
	$(ECHO) $(MK) $@
	$(ECHO) $(MK) v$(VERSION)
	
	
	