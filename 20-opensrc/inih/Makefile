
SRC_DIR := .

CC      = $(CROSS)gcc
CPP     = $(CROSS)g++
CXX     = $(CROSS)g++
LD      = $(CROSS)gcc
AR      = $(CROSS)ar
OBJDUMP = $(CROSS)objdump
INSTALL = install -D -m 644
RM      = -@rm -f


INCPATH			= -I./		\
				  -I./../../00-common/include

CFLAGS += -D_LINUX_ -g -Wall -fPIC -Wno-unused-function -rdynamic  -fvisibility=hidden -Wno-write-strings

#LIB_PATH	= ./../../00-common/lib/debug	\
 

#LIBS		= 


## Add library path and libraries to link options
LDFLAGS += $(foreach lib_path,$(LIB_PATH),-L$(lib_path))
LDFLAGS += $(foreach lib,$(LIBS),-l$(lib)$(LIB_SUFFIX))
LDFLAGS += -Wl,-rpath=./../../00-common/lib/debug:./ -shared


C_FILES := $(wildcard $(SRC_DIR)/*.c)
CPP_FILES := $(wildcard $(SRC_DIR)/*.cpp)


OBJS := $(C_FILES:%.c=%)
OBJS += $(CPP_FILES:%.cpp=%)


OBJS := ${foreach obj, ${OBJS}, ${obj}.o}

TARGET			= libini.so

.SUFFIXES: .o .c .cpp .cc .cxx .C

${SRC_DIR}/%.o:${SRC_DIR}/%.cpp
	${CXX} -c -o $@ $< ${CFLAGS} ${INCPATH}

${SRC_DIR}/%.o:${SRC_DIR}/%.c
	${CC} -c -o $@ $< ${CFLAGS} ${INCPATH}





all: $(TARGET)

clean:
	${RM} $(TARGET) $(OBJS)

$(TARGET):  $(OBJS)
	$(CC) -o $@ $^  $(LDFLAGS)
	cp $(TARGET) ../../00-common/lib/debug

