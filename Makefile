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

VERSION_MAJOR		:= 2
VERSION_MINOR		:= 0
VERSION			:= $(VERSION_MAJOR).$(VERSION_MINOR)

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


# --------- Defines

DEFS =
DEFS += -DVERSION_MAJOR=$(VERSION_MAJOR)
DEFS += -DVERSION_MINOR=$(VERSION_MINOR)

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
	$(VERBOSE) $(ECHO) Performing make - Version: $(VERSION)
	$(VERBOSE) $(CP) $(OBJECT_DIRECTORY)/$(TARGET).hex v$(VERSION)/$(TARGET)_v$(VERSION).hex
	$(VERBOSE) $(CP) $(OBJECT_DIRECTORY)/$(TARGET).lss v$(VERSION)/$(TARGET)_v$(VERSION).lss
	$(VERBOSE) $(CP) $(OBJECT_DIRECTORY)/$(TARGET).o v$(VERSION)/$(TARGET)_v$(VERSION)
	$(VERBOSE) $(MAKE_EXE) v$(VERSION)/$(TARGET)_v$(VERSION)
	$(VERBOSE) $(ECHO) $(MSG_PROG_LOCATION) v$(VERSION)/$(TARGET)_v$(VERSION).hex
	$(VERBOSE) $(ECHO) $(MSG_FINISH)
	
clean:
	$(VERBOSE) $(ECHO) Performing clean
	$(VERBOSE) $(ECHO) - Removing object directory from filesystem
	$(VERBOSE) $(RM) $(RM_FLAGS) $(OBJECT_DIRECTORY)
	$(VERBOSE) $(ECHO) - Removing release directory from filesystem
	$(VERBOSE) $(RM) $(RM_FLAGS) v$(VERSION)
	$(VERBOSE) $(ECHO) $(MSG_FINISH)

# --------- 

%.hex: %.o
	$(VERBOSE) $(ECHO) - Generating $(OBJECT_DIRECTORY)/$(TARGET).hex
	$(OBJCOPY) -O $(FORMAT) $(OBJECT_DIRECTORY)/$< $(OBJECT_DIRECTORY)/$@
	$(VERBOSE) $(SIZE) $(OBJECT_DIRECTORY)/$(TARGET).o 
	
%.lss:
	$(VERBOSE) $(ECHO) $(MSG_LISTING)
	$(VERBOSE) $(OBJDUMP) -h -S $(OBJECT_DIRECTORY)/$(TARGET).o > $(OBJECT_DIRECTORY)/$(TARGET).lss

%.o:
	$(VERBOSE) $(ECHO) - Generating Object from: $@
	$(CC) $(DEFS) $(CFLAGS) $(LIBS) $(INC_PATH) $(CSRC) $< -o $(OBJECT_DIRECTORY)/$(notdir $@)
	
$(OBJECT_DIRECTORY):
	$(VERBOSE) $(ECHO) - Creating Build directory: $(OBJECT_DIRECTORY)
	$(VERBOSE) $(MK) $@
	$(VERBOSE) $(ECHO) - Creating Version directory: v$(VERSION)
	$(VERBOSE) $(MK) v$(VERSION)
	
install:
	$(VERBOSE) $(ECHO) Performing install
	$(VERBOSE) $(ECHO) - Copy service to target: /etc/init.d/$(TARGET_SERVICE)
	$(VERBOSE) $(CP) service/shc_service /etc/init.d/$(TARGET_SERVICE)
	$(VERBOSE) $(MAKE_EXE) /etc/init.d/$(TARGET_SERVICE)
	$(VERBOSE) $(ECHO) - Copy daemon to target: /usr/sbin/$(TARGET_DAEMON)
	$(VERBOSE) $(CP) v$(VERSION)/$(TARGET)_v$(VERSION) /usr/sbin/$(TARGET_DAEMON)
	$(VERBOSE) $(MAKE_EXE) /usr/sbin/$(TARGET_DAEMON)
	$(VERBOSE) $(ECHO) - Register service with inid.d
	$(VERBOSE) update-rc.d $(TARGET_SERVICE) defaults
	$(VERBOSE) update-rc.d $(TARGET_SERVICE) enable
	$(VERBOSE) $(ECHO) - Starting service
	$(VERBOSE) /etc/init.d/$(TARGET_SERVICE) start
	$(VERBOSE) $(ECHO) $(MSG_FINISH)
	
uninstall:
	$(VERBOSE) $(ECHO) Performing uninstall
	$(VERBOSE) $(ECHO) - Sopping service
	$(VERBOSE) /etc/init.d/$(TARGET_SERVICE) stop
	$(VERBOSE) $(ECHO) - Disabling service
	$(VERBOSE) update-rc.d $(TARGET_SERVICE) disable
	$(VERBOSE) $(ECHO) - Removing service from init.d
	$(VERBOSE) update-rc.d $(TARGET_SERVICE) remove
	$(VERBOSE) $(ECHO) - Removing service from filesystem
	$(VERBOSE) $(RM) /etc/init.d/$(TARGET_SERVICE)
	$(VERBOSE) $(ECHO) - Removing Daemon from filesystem
	$(VERBOSE) $(RM) /usr/sbin/$(TARGET_DAEMON)
	$(VERBOSE) $(ECHO) $(MSG_FINISH)
	
update:
	$(VERBOSE) $(ECHO) Performing update
	$(VERBOSE) $(ECHO) - Sopping service
	$(VERBOSE) /etc/init.d/$(TARGET_SERVICE) stop
	$(VERBOSE) $(ECHO) - Updateing daemon
	$(VERBOSE) $(CP) v$(VERSION)/$(TARGET)_v$(VERSION) /usr/sbin/$(TARGET_DAEMON)
	$(VERBOSE) $(MAKE_EXE) /usr/sbin/$(TARGET_DAEMON)
	$(VERBOSE) $(ECHO) - Starting service
	$(VERBOSE) /etc/init.d/$(TARGET_SERVICE) start
	$(VERBOSE) $(ECHO) $(MSG_FINISH)
	
git_update:
	$(VERBOSE) git pull
