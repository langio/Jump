
PROTO_DIR := $(shell dirname `echo $(MAKEFILE_LIST) | awk '{for(i=1;i<=NF;++i){if(\$$i~"proto.mk") {print \$$i;exit}}}'`)

INCLUDE += -I$(PROTO_DIR)/pb

LIBS += -L$(PROTO_DIR) -lproto -lprotobuf

DEP_MAKE_SUBDIRS += $(PROTO_DIR)
TARGET_DEP_OBJS += $(PROTO_DIR)/libproto.a

#MYCFLAGS += -L $(PROTO_DIR) -lproto