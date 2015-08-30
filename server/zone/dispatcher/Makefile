include ../../skynet/platform.mk
include ../../skynet/include.mk
include ../../protocol/proto.mk

CSERVICE_PATH ?= ../GameServer/lib

SKYNET_BUILD_PATH ?= .

#CFLAGS = -g -O2 -Wall $(MYCFLAGS) 
CFLAGS = -g -O2 -Wall $(MYCFLAGS)


#JEMALLOC_STATICLIB := 3rd/jemalloc/lib/libjemalloc_pic.a
#JEMALLOC_INC := 3rd/jemalloc/include/jemalloc
#MALLOC_STATICLIB := $(JEMALLOC_STATICLIB)

# Server

CSERVICE = dispatcher

all : \
  $(foreach v, $(CSERVICE), $(CSERVICE_PATH)/$(v).so)
 

$(CSERVICE_PATH) :
	mkdir $(CSERVICE_PATH)

define CSERVICE_TEMP
  $$(CSERVICE_PATH)/$(1).so : service_$(1).cpp | $$(CSERVICE_PATH)
	$$(CPP) $$(CFLAGS) $$(SHARED) $$< -o $$@ $(INCLUDE)
endef

$(foreach v, $(CSERVICE), $(eval $(call CSERVICE_TEMP,$(v))))


clean :
	rm -f $(CSERVICE_PATH)/*.so && echo $(INCLUDE)

