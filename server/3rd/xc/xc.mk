XC_DIR := $(shell dirname `echo $(MAKEFILE_LIST) | awk '{for(i=1;i<=NF;++i){if(\$$i~"xc.mk") {print \$$i;exit}}}'`)

INCLUDE += -I$(XC_DIR)/include
LIBS += -L$(XC_DIR)/lib -lxcutil

DEP_MAKE_SUBDIRS += $(XC_DIR)
TARGET_DEP_OBJS += $(XC_DIR)/lib/libxcutil.a

