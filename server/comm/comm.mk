COMM_DIR := $(shell dirname `echo $(MAKEFILE_LIST) | awk '{for(i=1;i<=NF;++i){if(\$$i~"comm.mk") {print \$$i;exit}}}'`)

INCLUDE += -I$(COMM_DIR)