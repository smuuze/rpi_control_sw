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
MK			:= mkdir -p
CP			:= cp
MAKE_EXE		:= chmod ug=+rwx
ECHO			:= echo

RM_FLAGS		:= -rf
VERBOSE 		:= @

# --------- Message Output

MSG_COMPILING		:= Compiling
MSG_LINKING		:= Linking to
MSG_PROG_LOCATION	:= Your programm can be found at
MSG_LISTING		:= - Generating Disassembly
MSG_FINISH		:= --------------- Make done ---------------


# --------- Application Properties (Target / Working dir / ...)

VERSION_MAJOR		:= 2
VERSION_MINOR		:= 0
VERSION			:= $(VERSION_MAJOR).$(VERSION_MINOR)

OBJECT_DIRECTORY	:= obj
OUTPUT_DIRECTORY	:= program
CONFIG_DIRECTORY	:= cfg
RELEASE_DIRECTORY	:= release/v$(VERSION)
APP_PATH		:= src
FORMAT			:= ihex

TARGET			:= SmartHomeClient
TARGET_DAEMON		:= shcd
TARGET_SERVICE		:= shc_service


# --------- Include Path

INC_PATH =
INC_PATH += -I $(APP_PATH)
INC_PATH += -I $(TOOLCHAIN_INC_PATH)


# --------- Library List

LIBS =
LIBS += -l paho-mqtt3c
LIBS += -l wiringPi

# --------- Source File List

CSRC = 
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

DEBUG_ENABLED = -DDEBUG_ENABLED

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

all: obj_dir release_obj lss_file hex_file
	$(VERBOSE) $(CP) $(OBJECT_DIRECTORY)/$(TARGET).hex $(TARGET).hex
	$(VERBOSE) $(CP) $(OBJECT_DIRECTORY)/$(TARGET).lss $(TARGET).lss
	$(VERBOSE) $(CP) $(OBJECT_DIRECTORY)/$(TARGET).o $(TARGET)
	$(VERBOSE) $(MAKE_EXE) $(TARGET)
	$(VERBOSE) $(ECHO) $(MSG_PROG_LOCATION) $(TARGET)
	$(VERBOSE) $(ECHO) $(MSG_FINISH)

release: obj_dir release_obj release_dir lss_file hex_file
	$(VERBOSE) $(CP) $(OBJECT_DIRECTORY)/$(TARGET).hex $(RELEASE_DIRECTORY)/$(TARGET).hex
	$(VERBOSE) $(CP) $(OBJECT_DIRECTORY)/$(TARGET).lss $(RELEASE_DIRECTORY)/$(TARGET).lss
	$(VERBOSE) $(CP) $(OBJECT_DIRECTORY)/$(TARGET).o $(RELEASE_DIRECTORY)/$(TARGET)
	$(VERBOSE) $(MAKE_EXE) $(RELEASE_DIRECTORY)/$(TARGET)
	$(VERBOSE) $(ECHO) $(MSG_PROG_LOCATION) $(RELEASE_DIRECTORY)/$(TARGET)
	$(VERBOSE) $(ECHO) $(MSG_FINISH)

debug: obj_dir debug_obj lss_file hex_file
	$(VERBOSE) $(CP) $(OBJECT_DIRECTORY)/$(TARGET).hex $(TARGET).hex
	$(VERBOSE) $(CP) $(OBJECT_DIRECTORY)/$(TARGET).lss $(TARGET).lss
	$(VERBOSE) $(CP) $(OBJECT_DIRECTORY)/$(TARGET).o $(TARGET)
	$(VERBOSE) $(MAKE_EXE) $(TARGET)
	$(VERBOSE) $(ECHO) $(MSG_PROG_LOCATION) $(TARGET)
	$(VERBOSE) $(ECHO) $(MSG_FINISH)

clean:
	$(VERBOSE) $(ECHO) - Removing object directory from filesystem
	$(VERBOSE) $(RM) $(RM_FLAGS) $(OBJECT_DIRECTORY)
	$(VERBOSE) $(ECHO) - Removing generated program-files
	$(VERBOSE) $(RM) $(RM_FLAGS) $(TARGET).hex
	$(VERBOSE) $(RM) $(RM_FLAGS) $(TARGET).lss
	$(VERBOSE) $(RM) $(RM_FLAGS) $(TARGET)
	$(VERBOSE) $(ECHO) $(MSG_FINISH)

# --------- 

release_obj:
	$(VERBOSE) $(ECHO) - Generating Relase-Object - Version: $(VERSION)
	$(VERBOSE) $(CC) $(DEFS) $(CFLAGS) $(LIBS) $(INC_PATH) $(CSRC) -o $(OBJECT_DIRECTORY)/$(TARGET).o

debug_obj:
	$(VERBOSE) $(ECHO) - Generating Debug-Object - Version: $(VERSION)
	$(VERBOSE) $(CC) $(DEFS) $(DEBUG_ENABLED) $(CFLAGS) $(LIBS) $(INC_PATH) $(CSRC) -o $(OBJECT_DIRECTORY)/$(TARGET).o

hex_file:
	$(VERBOSE) $(ECHO) - Generating $(OBJECT_DIRECTORY)/$(TARGET).hex
	$(VERBOSE) $(OBJCOPY) -O $(FORMAT) $(OBJECT_DIRECTORY)/$(TARGET).o $(OBJECT_DIRECTORY)/$(TARGET).hex
	$(VERBOSE) $(SIZE) $(OBJECT_DIRECTORY)/$(TARGET).o 

lss_file:
	$(VERBOSE) $(ECHO) $(MSG_LISTING)
	$(VERBOSE) $(OBJDUMP) -h -S $(OBJECT_DIRECTORY)/$(TARGET).o > $(OBJECT_DIRECTORY)/$(TARGET).lss

obj_dir:
	$(VERBOSE) $(ECHO) - Creating Object directory: $(OBJECT_DIRECTORY)
	$(VERBOSE) $(MK) $(OBJECT_DIRECTORY)

release_dir:
	$(VERBOSE) $(ECHO) - Creating Release directory: $(RELEASE_DIRECTORY)
	$(VERBOSE) $(MK) $(RELEASE_DIRECTORY)

# --------- 

install:
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
	$(VERBOSE) $(ECHO) - Stopping service
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
	$(VERBOSE) $(ECHO) - Stopping service
	$(VERBOSE) /etc/init.d/$(TARGET_SERVICE) stop
	$(VERBOSE) $(ECHO) - Updateing daemon
	$(VERBOSE) $(CP) v$(VERSION)/$(TARGET)_v$(VERSION) /usr/sbin/$(TARGET_DAEMON)
	$(VERBOSE) $(MAKE_EXE) /usr/sbin/$(TARGET_DAEMON)
	$(VERBOSE) $(ECHO) - Starting service
	$(VERBOSE) /etc/init.d/$(TARGET_SERVICE) start
	$(VERBOSE) $(ECHO) $(MSG_FINISH)

create_user:
	$(VERBOSE) $(ECHO) - Creating SHC user
	$(VERBOSE) useradd -M -s /bin/false -G gpio,audio,spi shc

git_update:
	$(VERBOSE) git pull
