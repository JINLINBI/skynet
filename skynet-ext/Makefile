SKYNET_BUILD_PATH ?= ../skynet
SKYNET_INC ?= $(SKYNET_BUILD_PATH)/skynet-src
include $(SKYNET_BUILD_PATH)/platform.mk

LUA_CLIB_PATH ?= luaclib
CSERVICE_PATH ?= cservice
PROTO_INC ?= ../proto-src
PROTO_SRC ?= ../proto-src
UTILS_INC ?= ../3rd/utils

CFLAGS = -g -Wall -I$(LUA_INC) $(MYCFLAGS)
CPPFLAGS = -g -Wall -I$(LUA_INC) $(MYCFLAGS)
# CFLAGS += -DUSE_PTHREAD_LOCK

# lua

LUA_STATICLIB := $(SKYNET_BUILD_PATH)/3rd/lua/liblua.a
LUA_LIB ?= $(LUA_STATICLIB)
LUA_INC ?= $(SKYNET_BUILD_PATH)/3rd/lua

	
all : cjson
.PHONY : cjson

CJSON_STATICLIB := ../3rd/cJSON/libcjson.a
CJSON_LIB ?= $(CJSON_STATICLIB)
CJSON_INC ?= ../3rd/cJSON
CJSON := ../3rd/cJSON/cJSON.c

$(CJSON_STATICLIB):
	cd $(CJSON_INC) && make

LUA_CLIB = excel pieces 
CSERVICE = excel

SKYNET_SRC = skynet_main.c skynet_handle.c skynet_module.c skynet_mq.c \
  skynet_server.c skynet_start.c skynet_timer.c skynet_error.c \
  skynet_harbor.c skynet_env.c skynet_monitor.c skynet_socket.c socket_server.c \
  malloc_hook.c skynet_daemon.c skynet_log.c

all : $(foreach v, $(CSERVICE), $(CSERVICE_PATH)/$(v).so) \
	$(foreach v, $(LUA_CLIB), $(LUA_CLIB_PATH)/$(v).so)
 
$(LUA_CLIB_PATH) :
	mkdir $(LUA_CLIB_PATH)

$(CSERVICE_PATH) :
	mkdir $(CSERVICE_PATH)

define CSERVICE_TEMP
  $$(CSERVICE_PATH)/$(1).so : service-src/service_$(1).c | $$(CSERVICE_PATH)
	$$(CC) $$(CFLAGS) $$(SHARED) $(CJSON) $(UTILS_INC)/rbtree.c $$< -o $$@ -I$(SKYNET_INC) -I$(CJSON_INC) -I$(SKYNET_BUILD_PATH)/lualib-src -I$(UTILS_INC)
endef

$(foreach v, $(CSERVICE), $(eval $(call CSERVICE_TEMP,$(v))))

$(LUA_CLIB_PATH)/excel.so : lualib-src/lua-excel.c lualib-src/lua-pieces.c $(PROTO_SRC)/pieces.pb-c.c | $(LUA_CLIB_PATH)
	$(CC) $(CFLAGS) $(SHARED) -I$(SKYNET_INC) -I$(CJSON_INC) -I$(PROTO_INC) -I$(UTILS_INC) $^ -o $@ -lprotobuf-c

$(LUA_CLIB_PATH)/pieces.so : lualib-src/lua-pieces.c lualib-src/lua-excel.c $(PROTO_SRC)/pieces.pb-c.c | $(LUA_CLIB_PATH)
	$(CC) $(CFLAGS) $(SHARED) -I$(SKYNET_INC) $^ -I$(CJSON_INC) -I$(PROTO_INC) -I$(UTILS_INC) -o $@ -lprotobuf-c

.PHONY : clean
clean :
	rm -f $(CSERVICE_PATH)/*.so $(LUA_CLIB_PATH)/*.so 
