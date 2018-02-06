## ==================================================
## rules for android arm

#Mdfy: suker 20130312 use new compiler r8d
#CROSS_PATH := $(android-cross-path)/bin/arm-linux-androideabi-
#CC  := $(CROSS_PATH)gcc
#CPP := $(CROSS_PATH)g++
CC  := $(CROSS_PATH)clang
CPP := $(CROSS_PATH)clang
LD  := $(CROSS_PATH)ld
AR  := $(CROSS_PATH)libtool

#INSTALL = install -D -m 644
INSTALL = cp 

## for symbols
STRIP := $(CROSS_PATH)strip

CFLAGS+=-arch arm64
#CFLAGS+=-arch i386 

CFLAGS+=-fmessage-length=0 -fdiagnostics-show-note-include-stack -fmacro-backtrace-limit=0 -fpascal-strings  -O0 
CFLAGS+=-Wall
#CFLAGS+=-fstrict-aliasing 
#CFLAGS+=-miphoneos-version-min=8.1 
#CFLAGS+=--serialize-diagnostics

CFLAGS+=-isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS7.1.sdk 
#CFLAGS+=-isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator7.1.sdk 

#CFLAGS+=-I/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include 
CFLAGS+= $(foreach dir,$(INC_PATH),-I$(dir))
CFLAGS+=-D_LINUX_

AR += -syslibroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS7.1.sdk
#AR += -syslibroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator7.1.sdk
AR += -framework Foundation 


OBJS := $(foreach obj,$(OBJS),$(obj).o)
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -x c -std=c99 -c $< -o $@  
$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CPP) $(CFLAGS) -x c++ -c $< -o $@
	
	
