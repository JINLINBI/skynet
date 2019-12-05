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
	cJSON *fields;
	cJSON *data;
	char *name;
	int size;
	int index;
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



static int excel_index(lua_State* L);
static void stack_dump(lua_State* L){
	int top = lua_gettop(L);
	for (int i = 1; i <= top; i++){
		int t = lua_type(L, i);
		switch (t){
			case LUA_TSTRING: {
				printf("'%s'", lua_tostring(L, i));
				break;
			}
			case LUA_TBOOLEAN: {
				printf("'%s'", lua_toboolean(L, i)? "true": "false");
				break;
			}
			case LUA_TNUMBER: {
				printf("'%g'", lua_tonumber(L, i));
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


static int excel_pairs_inner(lua_State * L){
	return 0;
}


static int _excel_pairs(lua_State * L){
	struct excel_data * data = (struct excel_data *) lua_touserdata(L, lua_upvalueindex(1));
	
	int index = 0;
	char* s_index;

	lua_pushnumber(L, 1);
	lua_pushnumber(L, 2);


	// if (lua_isnumber(L, -1)){
	// 	index = lua_tonumber(L, -1);
	// }
	// else if (lua_isstring(L, -1)){
	// 	s_index = lua_tostring(L, -1);
	// }
	// else if (lua_isnil(L, -1)){
	// 	lua_pushinteger(L, 1);
	// 	return 1;
	// }
	
	
	// cJSON* line = data->data;

	// if (cJSON_IsNull(line) || cJSON_IsInvalid(line) || data->size == 0){
	// 	lua_pushnil(L);
	// }
	// else {
	// 	for (int i = 0; i < data->size; i++){
	// 		cJSON * item = cJSON_GetArrayItem(line, i);
	// 		if (cJSON_GetArrayItem(item, data->index)->valueint == index){
	// 			item = cJSON_GetArrayItem(line, i + 1);
	// 			if (item == NULL || cJSON_IsNull(item)) {
	// 				lua_pushnil(L);
	// 			}
	// 			else {
	// 				lua_pushinteger(L, cJSON_GetArrayItem(item, data->index)->valueint);
	// 			}
	// 			break;
	// 		}
	// 	}
	// }

	return 0;
}

static int excel_pairs(lua_State* L){
	struct excel_data * data = (struct excel_data *) lua_touserdata(L, 1);
	//int data_index = (int) luaL_checkinteger(L, 2) - 1;
	luaL_argcheck(L, data != NULL, 1, "'excel_data' expected");
	stack_dump(L);
	
	// lua_pushnumber(L, 1);
	printf("excel_pairs function is called.\n");


	lua_pushcclosure(L, _excel_pairs, 1);

	return 1;
}

static int excel_len(lua_State* L){
	stack_dump(L);
	printf("len function called.\n");

	struct excel_data * data = (struct excel_data *) lua_touserdata(L, 1);
	luaL_argcheck(L, data != NULL, 1, "'excel_data' expected");
	lua_pushnumber(L, data->size);
	
	return 1;
}

static int excel_index(lua_State* L){
	struct excel_data * data = (struct excel_data *) lua_touserdata(L, 1);
	int data_index = (int) luaL_checkinteger(L, 2) - 1;
	luaL_argcheck(L, data != NULL, 1, "'excel_data' expected");
	lua_newtable(L);

	// 遍历数据
	int i = 0;
	while( i < data->size){		
		cJSON * item = cJSON_GetArrayItem(data->data, i++);
		cJSON * index = cJSON_GetArrayItem(item, data->index);
		if ((int)index->valuedouble == data_index + 1){
			
			int len = cJSON_GetArraySize(data->fields);
			int j = 0;
			
			while (j < len){
				cJSON* fields_item = cJSON_GetArrayItem(data->fields, j);
				cJSON* name = cJSON_GetObjectItem(fields_item, "name");
				cJSON* type = cJSON_GetObjectItem(fields_item, "type");
				if (!strcmp(type->valuestring, "float")){
					lua_pushnumber(L, cJSON_GetArrayItem(item, j)->valuedouble);
					stack_dump(L);
				}
				else if (!strcmp(type->valuestring, "number")){
					lua_pushnumber(L, cJSON_GetArrayItem(item, j)->valueint);
					stack_dump(L);
				}
				else if (!strcmp(type->valuestring, "int[]")){
					// FIXME: 
					lua_pushnumber(L, 1);
					// int int_num = cJSON_GetArraySize(item);
					// int k = 0;
					// lua_newtable(L);
					// while (k < int_num){
					// 	lua_pushinteger(L, cJSON_GetArrayItem(item, k++)->valuedouble);
					// 	lua_settable(L, -2);
					// }
				}
				else if (!strcmp(type->valuestring, "string")){
					lua_pushstring(L, cJSON_GetArrayItem(item, j)->valuestring);
					stack_dump(L);
				}
				j++;
				lua_setfield(L, -2, name->valuestring);
				stack_dump(L);
			}
			break;
		}
	}
	return 1;
}


static luaL_Reg arrayFunc [] = {
	{"__index", excel_index},
	{"__pairs", excel_pairs},
	{"__len", excel_len},
	{NULL, NULL}
};

int InitMetaTable(lua_State *L){
	luaL_newmetatable(L, "excel_meta");
	luaL_setfuncs(L, arrayFunc, 0);
	return 1;
}

LUAMOD_API int
luaopen_excel(lua_State *L) {
	luaL_checkversion(L);

	lua_getfield(L, LUA_REGISTRYINDEX, "skynet_context");
	struct skynet_context *ctx = lua_touserdata(L, -1);
	if (ctx == NULL) {
		return luaL_error(L, "Init skynet context first");
	}

	int excel_handle = skynet_handle_findname("excel");
	struct skynet_context * excel_service = skynet_handle_grab(excel_handle);
	if (excel_service == NULL){
		return luaL_error(L, "Coundn't find excel service");
	}

	struct excel_service * inst = excel_service->instance;
	cJSON * cjson = inst->excel_root;
	printf("printing excel_services->excel_root %s\n", cJSON_Print(cjson));

	InitMetaTable(L);
	lua_newtable(L);

	cJSON* file_item = inst->excel_root->child;
	while (!cJSON_IsNull(file_item) || !cJSON_IsInvalid(file_item)){
		struct excel_data* user_data = (struct excel_data* ) lua_newuserdata(L, sizeof(struct excel_data));
		user_data->name = skynet_strdup(file_item->string);
		user_data->fields = cJSON_GetObjectItem(cJSON_GetObjectItem(cjson, file_item->string), "fields");
		user_data->data = cJSON_GetObjectItem(cJSON_GetObjectItem(cjson, file_item->string), "data");
		user_data->size = cJSON_GetArraySize(user_data->data);
		user_data->index = 0;

		luaL_getmetatable(L, "excel_meta");
		lua_setmetatable(L, -2);
		
		lua_setfield(L, -2, file_item->string);
		file_item = file_item->next;
		if (file_item == NULL){
			printf("excel_root has no more childs.\n");
			break;
		}
	}

	printf("excel finished.");
	
	return 1;
}

