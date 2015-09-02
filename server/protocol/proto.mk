
PROTO_DIR := $(shell dirname `echo $(MAKEFILE_LIST) | awk '{for(i=1;i<=NF;++i){if(\$$i~"proto.mk") {print \$$i;exit}}}'`)

#SUBDIRS += $(foreach D,$(PROTO_DIR),$(shell find $(D) -mindepth 1 -maxdepth 1 -type d | grep -v $(PBOUTPUTDIR) ))

INCLUDE += -I$(PROTO_DIR)/pb

MYCFLAGS += -L $(PROTO_DIR) -lproto