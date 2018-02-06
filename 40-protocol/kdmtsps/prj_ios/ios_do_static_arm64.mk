## android common set
#include $(BUILD_PATH)/android_cfg.mk

## compile
include $(BUILD_PATH)/ios_compile_arm64.mk

	
## ==================================================
## out

all: $(LIB_TARGET) install_rls

#$(LIB_TARGET): $(OBJS)
#	$(AR) crus $(LIB_TARGET) $(OBJS)
$(LIB_TARGET): $(OBJS)
	$(AR) -o $@ $(OBJS)	

clean:
	rm -f $(OBJS) *.pdb *.map $(LIB_TARGET)
		
install_all:install_dbg install_rls

install_dbg:
	$(INSTALL) $(LIB_TARGET) $(INSTALL_DBG_PATH)/$(LIB_TARGET)
	
install_rls:
	$(INSTALL) $(LIB_TARGET) $(INSTALL_RLS_PATH)/$(LIB_TARGET)
