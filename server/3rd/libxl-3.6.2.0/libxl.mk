LIBXL_DIR := $(shell dirname `echo $(MAKEFILE_LIST) | awk '{for(i=1;i<=NF;++i){if(\$$i~"libxl.mk") {print \$$i;exit}}}'`)


ARCH = $(shell getconf LONG_BIT)
ifeq ($(ARCH), 32)
  LIBPATH = $(LIBXL_DIR)/lib
else
  LIBPATH = $(LIBXL_DIR)/lib64
endif

MYCFLAGS += -L $(LIBPATH) -lxl -Wl,-rpath,$(LIBPATH),-rpath,../$(LIBPATH)

INCLUDE += -I$(LIBXL_DIR)/include_cpp