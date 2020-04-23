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
#include "rbtree.h"

#define data_length 128

typedef struct excel {
	int filescount;
	cJSON* excel_root;
	const char* excel_path;
	struct rb_cjson_root** rb_cjson_files_root;
} excel_service;

typedef struct excel_list {
	int line_size;
	char * name;
	cJSON * data;
	cJSON * fields;
	cJSON * files;
	rb_cjson_root * rbtree_root;
} excel_list;

typedef struct excel_line {
	int id;
	int size;
	cJSON * line_data;
	cJSON * line_fields;
} excel_line;

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

static int InitExcelListMetaTable(lua_State *L){
	luaL_newmetatable(L, "excel_list_meta");
	luaL_setfuncs(L, excel_list_func, 0);
	return 1;
}

static int InitExcelLineMetaTable(lua_State *L){
	luaL_newmetatable(L, "excel_line_meta");
	luaL_setfuncs(L, excel_line_func, 0);
	return 1;
}

static void CreateExcelLineUserData(lua_State *L, cJSON * data, cJSON * fields, int32_t index, int32_t id) {
	InitExcelLineMetaTable(L);
	if (id >= 0) {
		lua_pushinteger(L, id);
	}
	excel_line * excel_line_data = (excel_line*)lua_newuserdata(L, sizeof(excel_line));
	excel_line_data->line_data = data;
	excel_line_data->line_fields = fields;
	excel_line_data->id = index;
	excel_line_data->size = cJSON_GetArraySize(data);

	luaL_getmetatable(L, "excel_line_meta");
	lua_setmetatable(L, -2);
}

static excel_service * inst = NULL;

/////////////////////////////////////////////////////////////
//  excel_list : start
/////////////////////////////////////////////////////////////
static int inner_excel_list_pairs(lua_State * L){
	excel_list * data = (excel_list *) lua_touserdata(L, lua_upvalueindex(1));

	rb_cjson_line * line = rb_first_cjson_line(&data->rbtree_root->rb_root);
	cJSON * item = cJSON_GetArrayItem(data->data, 0);
	if (!lua_isnil(L, 2)) {
		int32_t index = luaL_checkinteger(L, 2);
		line = rb_next_cjson_by_index(&data->rbtree_root->rb_root, index);
	}

	if (!line) {
		lua_pushnil(L);
		lua_pushnil(L);
		return 0;
	}

	item = line->cjson_line_item;
	cJSON * id = cJSON_GetArrayItem(item, 0);
	CreateExcelLineUserData(L, item, data->fields, id->valueint, id->valueint);
	return 2;
}

static int excel_list_pairs(lua_State* L){
	excel_list * data = (excel_list *) lua_touserdata(L, 1);
	luaL_argcheck(L, data != NULL, 1, "'excel_data' expected");

	lua_pushcclosure(L, inner_excel_list_pairs, 1);

	return 1;
}

static int excel_list_len(lua_State* L) {
	excel_list * data = (excel_list *) lua_touserdata(L, 1);
	luaL_argcheck(L, data != NULL, 1, "'excel_data' expected");
	lua_pushinteger(L, data->line_size);

	return 1;
}

static int excel_list_index(lua_State* L) {
	excel_list * excel_list_data = (excel_list *) lua_touserdata(L, 1);
	luaL_argcheck(L, excel_list_data != NULL, 1, "'excel_list_data' expected");

	int index = (int) luaL_checkinteger(L, 2);

	rb_cjson_line * line = rb_search_cjson_line(&excel_list_data->rbtree_root->rb_root, index);
	if (line) {
		CreateExcelLineUserData(L, line->cjson_line_item, excel_list_data->fields, index, -1);
	}
	else {
		lua_pushnil(L);
	}

	return 1;
}

/////////////////////////////////////////////////////////////
//  excel_line : start
/////////////////////////////////////////////////////////////
void parse_line_data(lua_State * L, cJSON * item, char * type) {
	if (!item || !type) {
		lua_pushnil(L);
	}
	else if (!strcmp(type, "number")){
		lua_pushinteger(L, item->valuedouble);
	}
	else if (!strcmp(type, "number[]")){
		int int_num = cJSON_GetArraySize(item);
		int k = 0;
		lua_newtable(L);
		while (k < int_num) {
			lua_pushinteger(L, k + 1);
			lua_pushinteger(L, cJSON_GetArrayItem(item, k++)->valuedouble);
			lua_settable(L, -3);
		}
	}
	else if (!strcmp(type, "string")){
		lua_pushstring(L, item->valuestring);
	}
	else if (!strcmp(type, "string[]")){
		int int_num = cJSON_GetArraySize(item);
		int k = 0;
		lua_newtable(L);
		while (k < int_num){
			lua_pushinteger(L, k + 1);
			lua_pushstring(L, cJSON_GetArrayItem(item, k++)->valuestring);
			lua_settable(L, -3);
		}
	}
	else {
		lua_pushnil(L);
	}
}

static int inner_excel_line_pairs(lua_State* L) {
	excel_line * data = (excel_line *) lua_touserdata(L, lua_upvalueindex(1));
	uint32_t data_size = data->size;
	const char* index_name = NULL;

	// 2.顺序遍历
	cJSON * col_data = cJSON_GetArrayItem(data->line_data, 0);
	cJSON * fields = cJSON_GetArrayItem(data->line_fields, 0);

	if (!lua_isnil(L, 2)) {
		index_name = luaL_checkstring(L, 2);


		int32_t index = 0;
		while (strcmp(cJSON_GetArrayItem(data->line_fields, index++)->string, index_name));
		col_data = cJSON_GetArrayItem(data->line_data, index);
		fields = cJSON_GetArrayItem(data->line_fields, index);
		if (!fields) {
			lua_pushnil(L);
			lua_pushnil(L);
			return 2;
		}
	}
	
	lua_pushstring(L, fields->string);
	parse_line_data(L, col_data, fields->valuestring);
	return 2;
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
	excel_line * data = (excel_line *) lua_touserdata(L, 1);
	luaL_argcheck(L, data != NULL, 1, "'data' expected");

	const char * index_name = luaL_checkstring(L, 2);
	cJSON* fields = cJSON_GetObjectItem(data->line_fields, index_name);
	if (fields) {
		int32_t index = 0;
		while (strcmp(cJSON_GetArrayItem(data->line_fields, index)->string, index_name)) index++;
		cJSON * item = cJSON_GetArrayItem(data->line_data, index);
		parse_line_data(L, item, fields->valuestring);
		goto out;
	}

	lua_pushnil(L);
out:
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

	inst = excel_service->instance;
	cJSON * cjson_root = inst->excel_root;

	InitExcelListMetaTable(L);
	lua_newtable(L);

	cJSON* cjson_excel_lists = cjson_root->child;
	int32_t rbtree_root_index = 0;
	while (cjson_excel_lists && !cJSON_IsNull(cjson_excel_lists)) {
		struct excel_list* excel_list_data = (struct excel_list* ) lua_newuserdata(L, sizeof(struct excel_list));
		char* excel_list_name = cjson_excel_lists->string;
		excel_list_data->name = excel_list_name;
		excel_list_data->fields = cJSON_GetObjectItem(cJSON_GetObjectItem(cjson_root, excel_list_name), "fields");
		excel_list_data->data = cJSON_GetObjectItem(cJSON_GetObjectItem(cjson_root, excel_list_name), "data");
		excel_list_data->line_size = cJSON_GetArraySize(excel_list_data->data);
		excel_list_data->rbtree_root = inst->rb_cjson_files_root[rbtree_root_index++];

		luaL_getmetatable(L, "excel_list_meta");
		lua_setmetatable(L, -2);

		lua_setfield(L, -2, cjson_excel_lists->string);
		cjson_excel_lists = cjson_excel_lists->next;
	}

	return 1;
}

