
PROTO_DIR := $(shell dirname `echo $(MAKEFILE_LIST) | awk '{for(i=1;i<=NF;++i){if(\$$i~"proto.mk") {print \$$i;exit}}}'`)

INCLUDE += -I$(PROTO_DIR)/pb

MYCFLAGS += -L $(PROTO_DIR) -lproto