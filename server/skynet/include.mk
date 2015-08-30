SKYNET_DIR := $(shell dirname `echo $(MAKEFILE_LIST) | awk '{for(i=1;i<=NF;++i){if(\$$i~"include.mk") {print \$$i;exit}}}'`)

INCLUDE += -I$(SKYNET_DIR)/skynet-src -I$(SKYNET_DIR)/service-src  

