#define LUA_LIB

#include "skynet.h"

#include <lua.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

#include "skynet.h"
#include "skynet_handle.h"
#include "cJSON.h"

#define data_length 128
struct excel{
	cJSON * data;
};

struct excel_data{
	cJSON *data;
	char *name;
	int size;
	int index: 4;
};

struct excel_service {
	FILE * handle;
	char * filename;
	int close;
	cJSON* excel_root;
};

struct skynet_context {
	void * instance;
};

static int
lget(lua_State *L) {
	luaL_error(L, "get function has been called");
	struct skynet_context * context = lua_touserdata(L, lua_upvalueindex(1));
	const char * cmd = luaL_checkstring(L, 1);
	printf("excel.so getstring %s", cmd);
	const char * result;
	const char * parm = NULL;
	if (lua_gettop(L) == 2) {
		parm = luaL_checkstring(L,2);
	}

	result = skynet_command(context, cmd, parm);
	if (result) {
		lua_pushstring(L, result);
		return 1;
	}
	return 0;
}


static int index_data(lua_State* L){
	printf("index data");
	lua_pushnumber(L, 1);

	return 1;
}

// static const struct luaL_Reg funcs[] = {
// 	{"__"}
// 	{NULL, NULL}
// };

static void stack_dump(lua_State* L){
	int i;
	int top = lua_gettop(L);
	for (int i = 0; i < top; i++){
		int t = lua_type(L, i);
		switch (t){
			case LUA_TSTRING: {
				printf("'%s'", lua_typename(L, i));
				break;
			}
			case LUA_TBOOLEAN: {
				printf("'%s'", lua_typename(L, i));
				break;
			}
			case LUA_TNUMBER: {
				printf("'%s'", lua_typename(L, i));
				break;
			}
			default: {
				printf("'%s'", lua_typename(L, t));
				break;
			}
		}

		printf("\t");
	}

	printf(">>>>>>>\n");
};

LUAMOD_API int
luaopen_excel(lua_State *L) {
	luaL_checkversion(L);

	// luaL_Reg l[] = {
	// 	{ "" , lget },
	// 	{ "get" , lget },
	// 	{ NULL, NULL },
	// };

	stack_dump(L);
	lua_getfield(L, LUA_REGISTRYINDEX, "skynet_context");
	struct skynet_context *ctx = lua_touserdata(L, -1);
	if (ctx == NULL) {
		return luaL_error(L, "Init skynet context first");
	}
	stack_dump(L);
	lua_pop(L, 1);

	int excel_handle = skynet_handle_findname("excel");
	struct skynet_context * excel_service = skynet_handle_grab(excel_handle);
	if (excel_service == NULL){
		return luaL_error(L, "Coundn't find excel service");
	}

	struct excel_service * inst = excel_service->instance;
	cJSON * cjson = inst->excel_root;
	// printf("%s\n", cJSON_Print(cjson));

	// luaL_newmetatable(L, "meta_excel");
	// lua_pushvalue(L, -1);

	stack_dump(L);
	lua_newtable(L);
	stack_dump(L);
	lua_pushcfunction(L, index_data);
	stack_dump(L);
	lua_setfield(L, -2, "__index");
	stack_dump(L);
	lua_pop(L, 1);
	stack_dump(L);
	// lua_pushstring(L, "atb_data");
	struct excel_data* user_data = (struct excel_data* ) lua_newuserdata(L, sizeof(struct excel_data));

	stack_dump(L);
	user_data->name = skynet_strdup(cjson->child->string);
	user_data->data = cJSON_GetObjectItem(cJSON_GetObjectItem(cjson, "./excel/json/atb_data.json"), "data");
	user_data->size = cJSON_GetArraySize(user_data->data);
	// lua_settable(L, -3);
	lua_setfield(L, -2, "atb_data");

	stack_dump(L);
	
	return 1;
}

