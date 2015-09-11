YAC_DIR := $(shell dirname `echo $(MAKEFILE_LIST) | awk '{for(i=1;i<=NF;++i){if(\$$i~"yac.mk") {print \$$i;exit}}}'`)

INCLUDE += -I$(YAC_DIR)/include
LIBS += -L$(YAC_DIR)/lib -lyacutil

DEP_MAKE_SUBDIRS += $(YAC_DIR)
TARGET_DEP_OBJS += $(YAC_DIR)/lib/libyacutil.a

