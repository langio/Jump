include ../../skynet/platform.mk
include ../../skynet/include.mk

CSERVICE_PATH ?= ../GameServer/lib

SKYNET_BUILD_PATH ?= .

CFLAGS = -g -O2 -Wall -I$(LUA_INC) $(MYCFLAGS) 


# Server

CSERVICE = dispatcher

all : \
  $(foreach v, $(CSERVICE), $(CSERVICE_PATH)/$(v).so)
 

$(CSERVICE_PATH) :
	mkdir $(CSERVICE_PATH)

define CSERVICE_TEMP
  $$(CSERVICE_PATH)/$(1).so : service_$(1).cpp | $$(CSERVICE_PATH)
	$$(CPP) $$(CFLAGS) $$(SHARED) $$< -o $$@ -Iskynet-src
endef

$(foreach v, $(CSERVICE), $(eval $(call CSERVICE_TEMP,$(v))))


clean :
	rm -f $(CSERVICE_PATH)/*.so


test:
	echo $(THIS_MAKEFILE)
