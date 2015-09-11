TC_DIR := $(shell dirname `echo $(MAKEFILE_LIST) | awk '{for(i=1;i<=NF;++i){if(\$$i~"tc.mk") {print \$$i;exit}}}'`)

INCLUDE += -I$(TC_DIR)/include
LIBS += -L$(TC_DIR)/lib -ltcutil

DEP_MAKE_SUBDIRS += $(TC_DIR)
TARGET_DEP_OBJS += $(TC_DIR)/lib/libtcutil.a

