TARGET = lib/libyacutil.a

SRC_DIR_RECURSION = Y

SRC_FILE_DIRS += ./src 
INCLUDE += -I./include -I/usr/local/mysql/include -I/usr/include/mysql
LIBS += -L/usr/local/mysql/lib/mysql -L/usr/lib/x86_64-linux-gnu/ -lmysqlclient

include ../../mk/rules_inc.mk
