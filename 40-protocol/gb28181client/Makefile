
SRC_DIR := ./source

CC      = $(CROSS)gcc
CPP     = $(CROSS)g++
CXX     = $(CROSS)g++
LD      = $(CROSS)gcc
AR      = $(CROSS)ar
OBJDUMP = $(CROSS)objdump
INSTALL = install -D -m 644
RM      = -@rm -f


#FFMPEG_PATH		= /home/wangkuan/work/code/opensrc/ffmpeg/ffmpeg-3.3/
INCPATH			= -I./include/ \
				  -I./../../00-common/include/ \
				  -I./ \
				  -I../../20-opensrc/osip/include \
				  -I../../20-opensrc/exosip/include \


CFLAGS += -D_LINUX_ -g -Wall -fPIC -Wno-unused-function -rdynamic -fexceptions   -fvisibility=hidden -Wno-write-strings	-std=c++11

LIB_PATH	= ./../../00-common/lib/debug	\
 

#LIBS		= osip2 osipparser2 


## Add library path and libraries to link options
LDFLAGS += $(foreach lib_path,$(LIB_PATH),-L$(lib_path))
LDFLAGS += $(foreach lib,$(LIBS),-l$(lib)$(LIB_SUFFIX))
LDFLAGS += -Wl,-rpath=./../../00-common/lib/debug:./ -shared


C_FILES := $(wildcard $(SRC_DIR)/*.c)
CPP_FILES := $(wildcard $(SRC_DIR)/*.cpp)


OBJS := $(C_FILES:%.c=%)
OBJS += $(CPP_FILES:%.cpp=%)


OBJS := ${foreach obj, ${OBJS}, ${obj}.o}

TARGET			= libgb28181client.so

.SUFFIXES: .o .c .cpp .cc .cxx .C

${SRC_DIR}/%.o:${SRC_DIR}/%.cpp
	${CXX} -c -o $@ $< ${CFLAGS} ${INCPATH}

${SRC_DIR}/%.o:${SRC_DIR}/%.c
	${CC} -c -o $@ $< ${CFLAGS} ${INCPATH}





all: $(TARGET)

clean:
	${RM} $(TARGET) $(OBJS)

$(TARGET):  $(OBJS)
	$(CXX) -o $@ $^ ../../00-common/lib/debug/libosip2.a ../../00-common/lib/debug/libosipparser2.a $(LDFLAGS)
	cp $(TARGET) ../../00-common/lib/debug

