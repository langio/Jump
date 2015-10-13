COMM_DIR := $(shell dirname `echo $(MAKEFILE_LIST) | awk '{for(i=1;i<=NF;++i){if(\$$i~"comm.mk") {print \$$i;exit}}}'`)

INCLUDE += -I$(COMM_DIR) -I$(COMM_DIR)/table -I$(COMM_DIR)/mem_pool

LIBS += -L$(COMM_DIR)/lib -lcomm