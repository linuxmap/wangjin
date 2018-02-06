###
### Copyright (c) 2004 Keda Telecom, Inc.
###

#########################################################################
###
###  DESCRIPTION:
###    Common definitions for all Makefiles in TARGET linux project.
###
#########################################################################

TOP := .
COMM_DIR := ../../00-common
SRC_DIR := $(TOP)/source

## Name and type of the target for this Makefile
ifeq ($(TARGET),)
  TARGET = libdataswitch.so
endif

#APP_TARGET := $(TARGET)
#ARC_TARGET	:= $(TARGET)
SO_TARGET	:= $(TARGET)


## Define debugging symbols
DEBUG = 1


CC	= g++


## Add driver-specific include directory to the search path
INC_PATH += $(TOP)/include \
            $(COMM_DIR)/include/ \

CFLAGS += ${foreach dir,${INC_PATH},-I${dir}}
CFLAGS += -D_LINUX_  -fPIC -g

LDFLAGS	+= -shared -fPIC 
            
## Libraries to include in shared object file
#LIBS :=  

#########################################################################
###The following contents are usually not modified
#########################################################################

## Object files that compose the target(s)
C_FILES := $(wildcard $(SRC_DIR)/*.c)
CPP_FILES := $(wildcard $(SRC_DIR)/*.cpp)

OBJS := $(C_FILES:%.c=%)
OBJS += $(CPP_FILES:%.cpp=%)

OBJS :=  ${foreach obj,${OBJS},${obj}.o}


##The component vars (eg. PRJ_CONF PRJ_VER) for path are defined in common.mk

#INSTALL_APP_PATH = $(COMM_DIR)/version/$(PRJ_CONF)/$(PRJ_VER)/$(APP_TARGET)

${SRC_DIR}/%.o:${SRC_DIR}/%.c
	${CC} -c -o $@ ${CFLAGS} $<

CXXFLAGS += ${CFLAGS}
${SRC_DIR}%.o:${SRC_DIR}%.cpp
	${CC} -c ${CXXFLAGS} -o $@  $<


all: $(TARGET)

$(TARGET):  $(OBJS)
	$(CC) -o $@ $^  $(LDFLAGS)
	cp $(TARGET) ../../00-common/lib/debug/

clean:
	${RM} ${OBJS} libdataswitch.so
