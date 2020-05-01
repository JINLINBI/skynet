#define LUA_LIB

#include "skynet.h"

#include <lua.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <time.h>
#include <mysql/mysql.h>

#include "skynet.h"
#include "skynet_handle.h"
#include "spinlock.h"
#include "lua-pieces.h"
#include "lua-excel.h"


// static MYSQL* pmysql = NULL;
static excel_service* inst = NULL;
static pieces* root = NULL;
static struct spinlock lock;
int excel_line_newindex(lua_State* L) {
	excel_line* data = (excel_line*) lua_touserdata(L, 1);
	luaL_argcheck(L, data != NULL, 1, "'excel_line userdata' expected");

	const char* index_name = lua_tostring(L, 2);
	const int64_t val = lua_tointeger(L, 3);
	pieces* pi = data->pi;
	if (pi == NULL) {
		return 0;
	}
	pi->excel_id = val;

	return 0;
}

#define define_pieces_flag_get(flag_arg) \
	else if (!strcmp(name, #flag_arg)) { \
		lua_pushinteger(L, pi->flag.pi_flag.flag_arg); \
	}

#define define_pieces_flag_set(flag_arg, val) \
	else if (!strcmp(name, #flag_arg)) { \
		pi->flag.pi_flag.flag_arg = val;\
	}

int get_pieces_flag(lua_State* L, const char* name, pieces* pi) {

	if (!strcmp(name, "excel_id")) {
		lua_pushinteger(L, pi->excel_id);
	}
	else if (!strcmp(name, "only_id")) {
		printf("only_id: %lu", pi->flag.only_id);
		lua_pushinteger(L, pi->flag.only_id);
	}
	else if (!strcmp(name, "born_time")) {
		lua_pushinteger(L, pi->flag.pi_flag.born_time + BUILD_TIME);
	}
	else if (!strcmp(name, "build_time")) {
		lua_pushinteger(L, BUILD_TIME);
	}
	define_pieces_flag_get(dirty)
	define_pieces_flag_get(data)
	define_pieces_flag_get(copy)
	define_pieces_flag_get(redmark)
	define_pieces_flag_get(dayreset)
	else {
		lua_pushnil(L);
	}

	return 1;
}

int set_pieces_flag(lua_State* L, const char* name, int64_t val, pieces* pi) {
	if (!strcmp(name, "only_id")) {
		pi->flag.only_id = val;
	}
	else if (!strcmp(name, "excel_id")) {
		pi->excel_id = val;
	}
	else if (!strcmp(name, "born_time")) {
		pi->flag.pi_flag.born_time = val - BUILD_TIME;
	}
	define_pieces_flag_set(dirty, val)
	define_pieces_flag_set(data, val)
	define_pieces_flag_set(copy, val)
	define_pieces_flag_set(redmark, val)
	define_pieces_flag_set(dayreset, val)


	return 0;
}

static int pieces_excel_index(lua_State* L, pieces* pi) {
	rb_cjson_line* line = rb_search_cjson_line(&((inst->rb_cjson_files_root)[1]->rb_root), pi->excel_id);
	if (line) {
		CreateExcelLineUserData(L, line->cjson_line_data, line->cjson_line_fields, pi->excel_id, -1, pi);
	}
	else {
		lua_pushnil(L);
	}

	return 1;
}

static int pieces_index_func(lua_State* L) {
	const char* index_name = lua_tostring(L, -1);
	lua_getfield(L, 1, "pieces_userdata");
	pieces * pi = (pieces*) lua_touserdata(L, -1);
	if (pi == NULL) {
		lua_pushnil(L);
		return 0;
	}
	else if (!strcmp(index_name, "excel")) {
		pieces_excel_index(L, pi);
	}
	else if (!strcmp(index_name, "__excel")) {
		pieces_excel_index(L, pi);
	}
	else {
		get_pieces_flag(L, index_name, pi);
	}

	return 1;
}

static int pieces_newindex_func(lua_State* L) {
	const char* index_name = lua_tostring(L, 2);
	lua_getfield(L, 1, "pieces_userdata");
	pieces * pi = (pieces*) lua_touserdata(L, -1);
	if (pi == NULL) {
		return 0;
	}
	else if (!strcmp(index_name, "excel")) {
		pieces_excel_index(L, pi);
	}
	else if (!strcmp(index_name, "__excel")) {
		pieces_excel_index(L, pi);
	}
	else {
		int64_t val = lua_tointeger(L, 3);
		set_pieces_flag(L, index_name, val, pi);
	}

	return 0;
}

static int pieces_save_func(lua_State* L) {
	lua_getfield(L, 1, "pieces_userdata");
	pieces * pi = (pieces*) lua_touserdata(L, -1);
	if (pi == NULL) {
		// lua_pushboolean(L, 0);
		return 0;
	}

	// char sql_buffer[256];
	// memset(sql_buffer, 0, sizeof(sql_buffer));
	// sprintf(sql_buffer, "insert into pieces(id, excelId) values(%lu, %d)", pi->flags.only_id, pi->excel_id);

	// printf("query mysql: %s\n", sql_buffer);

	// mysql_real_query(pmysql, sql_buffer, strlen(sql_buffer));

	// lua_pushboolean(L, 1);


	return 0;
}

static luaL_Reg arrayFunc [] = {
	{"__index", pieces_index_func},
	{"__newindex", pieces_newindex_func},
	// {"__len", excel_len},
	{"save", pieces_save_func},
	{NULL, NULL}
};

int InitMetaTable(lua_State* L){
	luaL_newmetatable(L, "pieces_meta");
	luaL_setfuncs(L, arrayFunc, 0);
	return 1;
}

static int new_pieces(lua_State* L) {
	pieces * pi = (pieces*) lua_newuserdata(L, sizeof(pieces));
	memset(pi, 0, sizeof(*pi));
	pi->flag.pi_flag.born_time = time(NULL) - BUILD_TIME;

	InitMetaTable(L);
	luaL_getmetatable(L, "pieces_meta");
	// TODO: waste of pushing pointer value.
	lua_pushlightuserdata(L, pi);
	lua_setfield(L, -2, "pieces_userdata");
	lua_setmetatable(L, -2);

	return 1;
}

static int root_pieces(lua_State* L) {
	spinlock_lock(&lock);
	pieces* pi = root;
	if (pi == NULL) {
		pi = (pieces*) skynet_malloc(sizeof(pieces));
		memset(pi, 0, sizeof(*pi));
		pi->flag.pi_flag.born_time = time(NULL) - BUILD_TIME;	
	}

	lua_pushlightuserdata(L, pi);
	InitMetaTable(L);
	luaL_getmetatable(L, "pieces_meta");
	// TODO: waste of pushing pointer value.
	lua_pushlightuserdata(L, pi);
	lua_setfield(L, -2, "pieces_userdata");
	lua_setmetatable(L, -2);

	root = pi;
	spinlock_unlock(&lock);
	return 1;
}

LUAMOD_API int
luaopen_pieces(lua_State* L) {
	luaL_checkversion(L);
	luaL_Reg l[] = {
		{ "new", new_pieces},
		{ "root", root_pieces},
		{ NULL, NULL }
	};

	int excel_handle = skynet_handle_findname("excel");
	struct skynet_context* excel_service = skynet_handle_grab(excel_handle);
	if (excel_service == NULL){
		return luaL_error(L, "pieces.so: Coundn't find excel service.");
	}

	inst = excel_service->instance;

	InitExcelListMetaTable(L);
	init_excel_root(L, inst);
	luaL_newlib(L, l);
	return 1;
}
