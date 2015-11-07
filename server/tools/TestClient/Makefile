TARGET = TestClinet

SRC_DIR_RECURSION = Y

SRC_FILE_DIRS += . 
INCLUDE += -I.
LIBS += -lpthread

NEED_MAKE_DEP = Y

include ../../skynet/include.mk
include ../socket-server-master/socket.mk
include ../../protocol/proto.mk
include ../../3rd/yac/yac.mk
include ../../comm/comm.mk
include ../../mk/rules_inc.mk