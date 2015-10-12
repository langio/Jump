REDOX_DIR := $(shell dirname `echo $(MAKEFILE_LIST) | awk '{for(i=1;i<=NF;++i){if(\$$i~"redox.mk") {print \$$i;exit}}}'`)

INCLUDE += -I$(REDOX_DIR)/include
#LIBS += -lredox -Wl,-rpath,/usr/local/lib64/
LIBS += -L$(REDOX_DIR)/lib64 -lredox_static -lev -lhiredis


