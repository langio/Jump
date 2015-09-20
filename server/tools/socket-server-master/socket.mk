SOCKET_DIR := $(shell dirname `echo $(MAKEFILE_LIST) | awk '{for(i=1;i<=NF;++i){if(\$$i~"socket.mk") {print \$$i;exit}}}'`)

INCLUDE += -I$(SOCKET_DIR)

LIBS += -L$(SOCKET_DIR) -lsocket