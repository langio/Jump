
PROTO_DIR := $(shell dirname `echo $(MAKEFILE_LIST) | awk '{for(i=1;i<=NF;++i){if(\$$i~"proto.mk") {print \$$i;exit}}}'`)

INCLUDE += -I$(PROTO_DIR)/pb -I$(PROTO_DIR)/proto

#PB_LIB_DIR = /usr/local/lib/
PB_LIB_DIR = /usr/lib/x86_64-linux-gnu/

#SO_LIBS += -L$(PROTO_DIR) -lproto -L$(PB_LIB_DIR) -lprotobuf -Wl,-rpath,$(PB_LIB_DIR)
LIBS += -L$(PROTO_DIR) -lproto -L$(PB_LIB_DIR) -lprotobuf -Wl,-rpath,$(PB_LIB_DIR)

#LIBS += -L $(LIBPATH) -lxl -Wl,-rpath,$(LIBPATH),-rpath,../$(LIBPATH)

DEP_MAKE_SUBDIRS += $(PROTO_DIR)
TARGET_DEP_OBJS += $(PROTO_DIR)/libproto.a
