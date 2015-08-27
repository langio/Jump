BASE_DIR := $(shell dirname `echo $(MAKEFILE_LIST) | awk '{for(i=1;i<=NF;++i){if(\$$i~"include.mk") {print \$$i;exit}}}'`)

INCLUDE += -I$(BASE_DIR)/skynet-src -I$(BASE_DIR)/service-src 

