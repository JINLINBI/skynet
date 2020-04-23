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


// static MYSQL * pmysql = NULL;
// lua-excel
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

static excel_service * inst = NULL;
static pieces* root = NULL;
static struct spinlock lock;

#define define_pieces_flag_get(flag_arg) \
	else if (strcmp(name, #flag_arg) == 0) { \
		lua_pushinteger(L, pi->flags.flag.flag_arg); \
	}

int get_pieces_flag(lua_State * L, const char* name, pieces * pi) {
	if (strcmp(name, "onlyId") == 0) {
		lua_pushinteger(L, pi->flags.onlyId);
	}
	else if (strcmp(name, "born_time") == 0) {
		lua_pushinteger(L, pi->flags.flag.born_time + build_time);
	}
	else if (strcmp(name, "build_time") == 0) {
		lua_pushinteger(L, build_time);
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

static int pieces_excel_index(lua_State * L, pieces * pi) {
	rb_cjson_line * line = rb_search_cjson_line(&((inst->rb_cjson_files_root)[1]->rb_root), /* pi->excel_id*/ 2);
	if (line) {
		CreateExcelLineUserData(L, line->cjson_line_data, line->cjson_line_fields, /*pi->excel_id*/ 2, -1);
	}
	else {
		lua_pushnil(L);
	}

	return 1;
}

static int pieces_func(lua_State * L) {
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

static int pieces_save_func(lua_State * L) {
	lua_getfield(L, 1, "pieces_userdata");
	pieces * pi = (pieces*) lua_touserdata(L, -1);
	if (pi == NULL) {
		// lua_pushboolean(L, 0);
		return 0;
	}

	// char sql_buffer[256];
	// memset(sql_buffer, 0, sizeof(sql_buffer));
	// sprintf(sql_buffer, "insert into pieces(id, excelId) values(%lu, %d)", pi->flags.onlyId, pi->excel_id);

	// printf("query mysql: %s\n", sql_buffer);

	// mysql_real_query(pmysql, sql_buffer, strlen(sql_buffer));

	// lua_pushboolean(L, 1);


	return 0;
}

static luaL_Reg arrayFunc [] = {
	{"__index", pieces_func},
	// {"__pairs", excel_pairs},
	// {"__len", excel_len},
	{"save", pieces_save_func},
	{NULL, NULL}
};

int InitMetaTable(lua_State *L){
	luaL_newmetatable(L, "pieces_meta");
	luaL_setfuncs(L, arrayFunc, 0);
	return 1;
}

static int new_pieces(lua_State* L) {
	pieces * pi = (pieces*) lua_newuserdata(L, sizeof(pieces));
	memset(pi, 0, sizeof(*pi));
	pi->flags.flag.born_time = time(NULL) - build_time;

	InitMetaTable(L);
	luaL_getmetatable(L, "pieces_meta");
	lua_pushlightuserdata(L, pi);
	lua_setfield(L, -2, "pieces_userdata");
	lua_setmetatable(L, -2);

	return 1;
}

static int root_pieces(lua_State * L) {
	spinlock_lock(&lock);
	pieces *pi = root;
	if (pi == NULL) {
		pi = (pieces*) skynet_malloc(sizeof(pieces));
		memset(pi, 0, sizeof(*pi));
		pi->flags.flag.born_time = time(NULL) - build_time;	
	}

	lua_pushlightuserdata(L, pi);
	InitMetaTable(L);
	luaL_getmetatable(L, "pieces_meta");
	lua_pushlightuserdata(L, pi);
	lua_setfield(L, -2, "pieces_userdata");
	lua_setmetatable(L, -2);

	root = pi;
	spinlock_unlock(&lock);
	return 1;
}

LUAMOD_API int
luaopen_pieces(lua_State *L) {
	luaL_checkversion(L);
	luaL_Reg l[] = {
		{ "new", new_pieces},
		{ "root", root_pieces},
		{ NULL, NULL }
	};

	int excel_handle = skynet_handle_findname("excel");
	struct skynet_context * excel_service = skynet_handle_grab(excel_handle);
	if (excel_service == NULL){
		return luaL_error(L, "Coundn't find excel service");
	}

	inst = excel_service->instance;

	InitExcelListMetaTable(L);
	init_excel_root(L, inst);

	luaL_newlib(L, l);

	return 1;
}
