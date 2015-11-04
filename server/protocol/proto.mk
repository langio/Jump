
PROTO_DIR := $(shell dirname `echo $(MAKEFILE_LIST) | awk '{for(i=1;i<=NF;++i){if(\$$i~"proto.mk") {print \$$i;exit}}}'`)

INCLUDE += -I$(PROTO_DIR)/pb -I$(PROTO_DIR)/proto

PB_LIB = libprotobuf.so

PB_LIB_DIR = /usr/local/lib/

FILE := $(PB_LIB_DIR)$(PB_LIB)

exist := $(shell if [ -f $(FILE) ]; then echo "exist"; else echo "notexist"; fi;)

ifneq ($(exist), "exist")
PB_LIB_DIR = /usr/lib/x86_64-linux-gnu/
endif

#PB_LIB_DIR = /usr/lib/x86_64-linux-gnu/

LIBS += -L$(PROTO_DIR) -lproto -L$(PB_LIB_DIR) -lprotobuf -Wl,-rpath,$(PB_LIB_DIR)

DEP_MAKE_SUBDIRS += $(PROTO_DIR)
TARGET_DEP_OBJS += $(PROTO_DIR)/libproto.a
