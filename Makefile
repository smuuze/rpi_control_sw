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

RM			:= rm
MK			:= mkdir
CP			:= cp
MAKE_EXE		:= chmod ug=+rwx
ECHO			:= echo

RM_FLAGS		:= -rf
VERBOSE 		:= @

# --------- Message Output

MSG_COMPILING		:= Compiling
MSG_LINKING		:= Linking to
MSG_PROG_LOCATION	:= Your programm can be found at
MSG_LISTING		:= Generating Disassembly
MSG_FINISH		:= --------------- Make done ---------------


# --------- Application Properties (Target / Working dir / ...)

OBJECT_DIRECTORY	:= obj
OUTPUT_DIRECTORY	:= program
CONFIG_DIRECTORY	:= cfg
APP_PATH		:= src
FORMAT			:= ihex
FREQUENCY		:= 10000000UL
VERSION			:= 1.0

TARGET			:= SmartHomeClient
TARGET_DAEMON		:= shcd
TARGET_SERVICE		:= shc_service


# --------- Include Path

INC_PATH +=
INC_PATH += -I $(APP_PATH)
INC_PATH += -I $(TOOLCHAIN_INC_PATH)


# --------- Library List

LIBS +=
LIBS += -l paho-mqtt3c
LIBS += -l wiringPi

# --------- Source File List

CSRC += 
CSRC += $(APP_PATH)/shc_timer.c
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

CFLAGS += -Wall
CFLAGS += -Os
CFLAGS += -funsigned-char
CFLAGS += -funsigned-bitfields
CFLAGS += -fpack-struct
CFLAGS += -fshort-enums
CFLAGS += -lm

LDFLAGS +=
#LDFLAGS += --section-start init=0
LDFLAGS += -Wl,-Map=$(OBJECT_DIRECTORY)/$(TARGET).map,--cref
LDFLAGS += -L$(TOOLCHAIN_LIB_PATH)


# --------- Make Targets 

C_DEPENDENCY_FILES 	:= $(CSRC:.c=.o)
C_ELF_FILES		:= $(CSRC:.c=.elf)
C_OBJ_FILES		:= $(CSRC:.c=.o)
OBJ_FILES		:= $(addprefix $(OBJECT_DIRECTORY)/,$(notdir $(C_OBJ_FILES)))
ELF_FILES		:= $(addprefix $(OBJECT_DIRECTORY)/,$(notdir $(C_ELF_FILES)))
OBJ  			:= $(C_ELF_FILES)

# --------- 

all: $(OBJECT_DIRECTORY) $(TARGET).hex $(TARGET).lss
	$(VERBOSE) $(CP) $(OBJECT_DIRECTORY)/$(TARGET).hex v$(VERSION)/$(TARGET)_v$(VERSION).hex
	$(VERBOSE) $(CP) $(OBJECT_DIRECTORY)/$(TARGET).lss v$(VERSION)/$(TARGET)_v$(VERSION).lss
	$(VERBOSE) $(CP) $(OBJECT_DIRECTORY)/$(TARGET).o v$(VERSION)/$(TARGET)_v$(VERSION)
	$(VERBOSE) $(MAKE_EXE) v$(VERSION)/$(TARGET)_v$(VERSION)
	$(VERBOSE) $(ECHO) $(MSG_PROG_LOCATION) v$(VERSION)/$(TARGET)_v$(VERSION).hex
	$(VERBOSE) $(ECHO) $(MSG_FINISH)
	
clean:
	$(VERBOSE) $(ECHO) Cleaning project
	$(VERBOSE) $(RM) $(RM_FLAGS) $(OBJECT_DIRECTORY)
	$(VERBOSE) $(RM) $(RM_FLAGS) v$(VERSION)

# --------- 

%.hex: %.o
	$(VERBOSE) $(ECHO) Generating $(OBJECT_DIRECTORY)/$(TARGET).hex
	$(OBJCOPY) -O $(FORMAT) $(OBJECT_DIRECTORY)/$< $(OBJECT_DIRECTORY)/$@
	$(VERBOSE) $(SIZE) $(OBJECT_DIRECTORY)/$(TARGET).o 
	
%.lss:
	$(VERBOSE) $(ECHO) $(MSG_LISTING)
	$(VERBOSE) $(OBJDUMP) -h -S $(OBJECT_DIRECTORY)/$(TARGET).o > $(OBJECT_DIRECTORY)/$(TARGET).lss

%.o:
	$(VERBOSE) $(ECHO) Generating Object from: $@
	$(VERBOSE) $(CC) $(CFLAGS) $(LIBS) $(INC_PATH) $(CSRC) $< -o $(OBJECT_DIRECTORY)/$(notdir $@)
	
$(OBJECT_DIRECTORY):
	$(VERBOSE) $(ECHO) Going to buiild release version $(VERSION)
	$(VERBOSE) $(ECHO) Creating Build directory: $(OBJECT_DIRECTORY)
	$(VERBOSE) $(MK) $@
	$(VERBOSE) $(ECHO) Creating Version directory: v$(VERSION)
	$(VERBOSE) $(MK) v$(VERSION)
	
install:
	$(VERBOSE) $(ECHO) Performing install
	$(VERBOSE) $(CP) service/shc_service /etc/init.d/$(TARGET_SERVICE)
	$(VERBOSE) $(MAKE_EXE) /etc/init.d/$(TARGET_SERVICE)
	$(VERBOSE) $(CP) v$(VERSION)/$(TARGET)_v$(VERSION) /usr/sbin/$(TARGET_DAEMON)
	$(VERBOSE) $(MAKE_EXE) /usr/sbin/$(TARGET_DAEMON)
	#$(VERBOSE) update-rc.d $(TARGET_SERVICE) enable
	$(VERBOSE) update-rc.d $(TARGET_SERVICE) defaults
	$(VERBOSE) /etc/init.d/$(TARGET_SERVICE) start
	
uninstall:
	$(VERBOSE) $(ECHO) Performing uninstall
	$(VERBOSE) /etc/init.d/$(TARGET_SERVICE) stop
	$(VERBOSE) update-rc.d $(TARGET_SERVICE) disable
	$(VERBOSE) update-rc.d $(TARGET_SERVICE) remove
	$(VERBOSE) $(RM) /etc/init.d/$(TARGET_SERVICE)
	$(VERBOSE) $(RM) /usr/sbin/$(TARGET_DAEMON)
	
update:
	$(VERBOSE) $(ECHO) Performing update
	$(VERBOSE) /etc/init.d/$(TARGET_SERVICE) stop
	$(VERBOSE) $(CP) v$(VERSION)/$(TARGET)_v$(VERSION) /usr/sbin/$(TARGET_DAEMON)
	$(VERBOSE) $(MAKE_EXE) /usr/sbin/$(TARGET_DAEMON)
	$(VERBOSE) /etc/init.d/$(TARGET_SERVICE) start
	
git_update:
	$(VERBOSE) git pull
