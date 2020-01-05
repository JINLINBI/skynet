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
typedef struct excel{
	cJSON * data;
} excel;

typedef struct excel_list {
	int size;
	char * name;
	cJSON * data;
	cJSON * fields;
} excel_list;

typedef struct excel_line {
	int id;
	int size;
	cJSON * line_data;
	cJSON * line_fields;
} excel_line;

typedef struct excel_service {
	FILE * handle;
	char * filename;
	int close;
	cJSON* excel_root;
} excel_service;

typedef struct skynet_context {
	void * instance;
} skynet_context;


static int excel_list_index(lua_State* L);
static int excel_list_pairs(lua_State* L);
static int excel_list_len(lua_State* L);

static int excel_line_index(lua_State* L);
static int excel_line_pairs(lua_State* L);
static int excel_line_len(lua_State* L);
static int excel_line_tostring(lua_State* L);

static luaL_Reg excel_list_func [] = {
	{"__index", excel_list_index},
	{"__pairs", excel_list_pairs},
	{"__len", excel_list_len},
	{NULL, NULL}
};

static luaL_Reg excel_line_func [] = {
	{"__index", excel_line_index},
	{"__pairs", excel_line_pairs},
	{"__len", excel_line_len},
	{"__tostring", excel_line_tostring},
	{NULL, NULL}
};

int InitExcelListMetaTable(lua_State *L){
	luaL_newmetatable(L, "excel_list_meta");
	luaL_setfuncs(L, excel_list_func, 0);
	return 1;
}

int InitExcelLineMetaTable(lua_State *L){
	luaL_newmetatable(L, "excel_line_meta");
	luaL_setfuncs(L, excel_line_func, 0);
	return 1;
}



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

/////////////////////////////////////////////////////////////
//  excel_list : start
/////////////////////////////////////////////////////////////

static int inner_excel_list_pairs(lua_State * L){
	excel_list * data = (excel_list *) lua_touserdata(L, lua_upvalueindex(1));
	uint32_t data_size = data->size;

	// char* data = lua_tostring(L, L);

	stack_dump(L);
	int index = 0; 
	if (!lua_isnil(L, 2)) {
		index = luaL_checkinteger(L, 2);
	}

	if (index >= data_size) {
		lua_pushnil(L);
		lua_pushnil(L);
		return 2;
	}

	cJSON * item = cJSON_GetArrayItem(data->data, index);
	cJSON * id = cJSON_GetArrayItem(item, 0);

	InitExcelLineMetaTable(L);
	lua_pushinteger(L, index + 1);
	excel_line * excel_line_data = (excel_line*)lua_newuserdata(L, sizeof(excel_line));
	excel_line_data->line_data = item;
	excel_line_data->line_fields = data->fields;
	excel_line_data->id = id->valueint;
	excel_line_data->size = cJSON_GetArraySize(item);

	luaL_getmetatable(L, "excel_line_meta");
	lua_setmetatable(L, -2);


	return 2;
}

static int excel_list_pairs(lua_State* L){
	excel_list * data = (excel_list *) lua_touserdata(L, 1);
	luaL_argcheck(L, data != NULL, 1, "'excel_data' expected");

	lua_pushcclosure(L, inner_excel_list_pairs, 1);

	return 1;
}

static int excel_list_len(lua_State* L) {
	stack_dump(L);
	printf("len function called.\n");

	excel_list * data = (excel_list *) lua_touserdata(L, 1);
	luaL_argcheck(L, data != NULL, 1, "'excel_data' expected");
	lua_pushinteger(L, data->size);
	
	return 1;
}

static int excel_list_index(lua_State* L) {
	excel_list * excel_list_data = (excel_list *) lua_touserdata(L, 1);
	luaL_argcheck(L, excel_list_data != NULL, 1, "'excel_list_data' expected");

	int data_index = (int) luaL_checkinteger(L, 2);
	int list_size = excel_list_data->size;

	int hit = 0;
	for (int i = 0; i < list_size; i++) {
		cJSON * cjson_excel_line_data = cJSON_GetArrayItem(excel_list_data->data, i);
		cJSON * cjson_excel_line_id = cJSON_GetArrayItem(cjson_excel_line_data, 0);
		if (cjson_excel_line_id->valueint == data_index) {
			hit = 1;
			InitExcelLineMetaTable(L);
			excel_line * excel_line_data = (excel_line*)lua_newuserdata(L, sizeof(excel_line));
			excel_line_data->line_data = cjson_excel_line_data;
			excel_line_data->line_fields = excel_list_data->fields;
			excel_line_data->id = data_index;
			excel_line_data->size = cJSON_GetArraySize(cjson_excel_line_data);

			printf("hit: id = %d\n", excel_line_data->id);
			printf("hit: size = %d\n", excel_line_data->size);
			luaL_getmetatable(L, "excel_line_meta");
			lua_setmetatable(L, -2);
			break;
		}
	}

	if (!hit) {
		lua_pushnil(L);
	}

	return 1;
}

/////////////////////////////////////////////////////////////
//  excel_line : start
/////////////////////////////////////////////////////////////
static int inner_excel_line_pairs(lua_State* L) {
	return 0;
}

static int excel_line_pairs(lua_State* L){
	excel_line * data = (excel_line *) lua_touserdata(L, 1);
	luaL_argcheck(L, data != NULL, 1, "'excel_data' expected");

	lua_pushcclosure(L, inner_excel_line_pairs, 1);

	return 1;
}

static int excel_line_len(lua_State* L) {
	excel_line * data = (excel_line *) lua_touserdata(L, 1);
	luaL_argcheck(L, data != NULL, 1, "'excel_line_data' expected");
	lua_pushinteger(L, data->size);
	
	return 1;
}

static int excel_line_tostring(lua_State* L) {
	excel_line * data = (excel_line *) lua_touserdata(L, 1);
	luaL_argcheck(L, data != NULL, 1, "'excel_line_data' expected");

	static char tostring[128];
	snprintf(tostring, sizeof(tostring), "excel_line(%d)", data->id);
	lua_pushstring(L, tostring);
	
	return 1;
}

static int excel_line_index(lua_State* L) {
	excel_line * excel_line_data = (excel_line *) lua_touserdata(L, 1);
	luaL_argcheck(L, excel_line_data != NULL, 1, "'excel_line_data' expected");

	const char * line_name = luaL_checkstring(L, 2);

	lua_newtable(L);

	if (cJSON_IsNull(excel_line_data->line_data) || cJSON_IsInvalid(excel_line_data->line_data)) {
		lua_pushnil(L);
		return 1;
	}
	
	int hit = 0;
	for (int32_t i = 0; i < excel_line_data->size; i++) {
		cJSON * fields = cJSON_GetArrayItem(excel_line_data->line_fields, i);
		cJSON* name = cJSON_GetObjectItem(fields, "name");
		if (strcmp(name->valuestring, line_name) == 0) {
			hit = 1;
			cJSON * item = cJSON_GetArrayItem(excel_line_data->line_data, i);
			cJSON* type = cJSON_GetObjectItem(fields, "type");
			if (!strcmp(type->valuestring, "float")){
				lua_pushinteger(L, item->valuedouble);
			}
			else if (!strcmp(type->valuestring, "number")){
				lua_pushinteger(L, item->valueint);
			}
			else if (!strcmp(type->valuestring, "int[]")){
				// FIXME: 
				lua_pushinteger(L, 1);
				// int int_num = cJSON_GetArraySize(item);
				// int k = 0;
				// lua_newtable(L);
				// while (k < int_num){
				// 	lua_pushinteger(L, cJSON_GetArrayItem(item, k++)->valuedouble);
				// 	lua_settable(L, -2);
				// }
			}
			else if (!strcmp(type->valuestring, "string")){
				lua_pushstring(L, item->valuestring);
			}
			else {
				lua_pushnil(L);
			}

			break;
		}
	}

	if (!hit) {
		lua_pushnil(L);
	}

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
	cJSON * cjson_root = inst->excel_root;

	InitExcelListMetaTable(L);
	lua_newtable(L);

	cJSON* cjson_excel_lists = cjson_root->child;
	while (!cJSON_IsNull(cjson_excel_lists) || !cJSON_IsInvalid(cjson_excel_lists)){
		struct excel_list* excel_list_data = (struct excel_list* ) lua_newuserdata(L, sizeof(struct excel_list));
		excel_list_data->name = skynet_strdup(cjson_excel_lists->string);
		excel_list_data->fields = cJSON_GetObjectItem(cJSON_GetObjectItem(cjson_root, cjson_excel_lists->string), "fields");
		excel_list_data->data = cJSON_GetObjectItem(cJSON_GetObjectItem(cjson_root, cjson_excel_lists->string), "data");
		excel_list_data->size = cJSON_GetArraySize(excel_list_data->data);

		luaL_getmetatable(L, "excel_list_meta");
		lua_setmetatable(L, -2);
		
		lua_setfield(L, -2, cjson_excel_lists->string);
		cjson_excel_lists = cjson_excel_lists->next;
		if (cjson_excel_lists == NULL)
			break;
	}
	
	return 1;
}

