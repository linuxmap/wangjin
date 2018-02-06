
CC      = $(CROSS)g++
CXX     = $(CROSS)g++
LD      = $(CROSS)gcc
AR      = $(CROSS)ar
OBJDUMP = $(CROSS)objdump
INSTALL = install -D -m 644
RM      = -@rm -f


INCPATH			= -I./include/ -I./../../00-common/include/


CFLAGS += -D_LINUX_ -g -Wall -fPIC

## Add library path and libraries to link options
LDFLAGS += $(foreach lib_path,$(LIB_PATH),-L$(lib_path))
LDFLAGS += $(foreach lib,$(LIBS),-l$(lib)$(LIB_SUFFIX))
LDFLAGS += --shared


OBJS 		:= ./xmlparser.o					\
			   ./tinyxml2.o

./%.o:./%.c
	${CC} -c -o $@ ${CFLAGS} ${INCPATH} $<

./%.o:./%.cpp
	${CXX} -c -o $@ ${CFLAGS} ${INCPATH} $<

TARGET			= libxml.so

.SUFFIXES: .o .c .cpp .cc .cxx .C


all: $(TARGET)

test: main.o
	gcc main.o libxml.so -o test -Wl,-rpath=./

main.o:
	gcc -c -g ${CFLAGS} ${INCPATH} main.c

clean:
	${RM} $(TARGET) $(OBJS)
	${RM} main.o 

$(TARGET):  $(OBJS)
	$(CC) -o $@ $^  $(LDFLAGS)
	cp libxml.so ../../00-common/lib/debug/

