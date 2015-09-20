#socket-server : socket_server.c test.c
#	gcc -g -Wall -o $@ $^ -lpthread
#
#clean:
#	rm socket-server

TARGET = libsocket.a

SRC_DIR_RECURSION = Y

SRC_FILE_DIRS += . 
INCLUDE += -I.
LIBS += 

NEED_MAKE_DEP = Y

include ../../mk/rules_inc.mk