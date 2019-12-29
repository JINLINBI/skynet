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

enum pieces_type {
	user,
	player,
	role,
};

#define build_time strtol(__TIMESTAMP__, NULL, 10)


typedef struct pieces_op pieces_operations;
typedef struct pieces_link pieces_link;
typedef struct pieces_data pieces_data;


typedef struct pieces {
	union flags {
		struct {
			int64_t excel_id:24;
			int64_t classify:4;
			int64_t dirty:1;
			int64_t copy:1;
			int64_t data:1;
			int64_t lock:1;
			int64_t born_time:30;
		} flag;
		int64_t onlyId;
	} flags;
	pieces_operations * op;
	pieces_link * link;

}pieces;

typedef struct pieces_op
{
	int (*add)(pieces* pi, int copy, int parent);
	int (*del)(pieces* pi, int copy, int parent);
} pieces_operations;

typedef struct pieces_link
{
	pieces_data* prev;
	pieces_data* next;
} pieces_link;

typedef struct pieces_data {
	uint32_t date_type;
	uint32_t data_len;
	void * data;
} pieces_data;




static void stack_dump(lua_State* L){
	int top = lua_gettop(L);
	printf("get stack len: %d\n", top);
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

	printf(">>>>>>> returning ...\n");
};



static int id_func(lua_State * L) { 
	stack_dump(L);
	struct pieces * pi = (struct  pieces*) lua_touserdata(L, -1);
	if (pi == NULL) {
		printf("pi == NULL, get userdata type error.");
	}

	lua_pushinteger(L, pi->flags.onlyId);

	//stack_dump(L);
	//luaL_error(L, ".................................");
	return 1;
}

static int pieces_func(lua_State * L) {
	printf("in pieces func start;\n");
	stack_dump(L);
	lua_newtable(L);
	lua_pushstring(L, "onlyId");
	lua_pushnumber(L, 12341234);
	lua_settable(L, 1);
	stack_dump(L);

	printf("in pieces func end;\n");

	return 0;
}

static luaL_Reg arrayFunc [] = {
	//{"__index", pieces_func},
	// {"__pairs", excel_pairs},
	// {"__len", excel_len},
	// {"onlyId", id_func},
	{NULL, NULL}
};

int InitMetaTable(lua_State *L){
	luaL_newmetatable(L, "pieces_meta");
	luaL_setfuncs(L, arrayFunc, 0);
	return 1;
}

#define pieces_setflag(index, name, value) \
	lua_pushnumber(L, value);\
	lua_setfield(L, index, name);

static int new_pieces(lua_State* L) {
	stack_dump(L);
	printf("pieces new:>>>>>>>>>>>>>>>\n");
	pieces * p = (pieces*)lua_newuserdata(L, sizeof(pieces));
	memset(p, 0, sizeof(*p));
	p->flags.flag.born_time = time(NULL) - build_time;

	InitMetaTable(L);
	luaL_getmetatable(L, "pieces_meta");
	lua_setmetatable(L, -2);

	pieces_setflag(-2, "onlyId", 123432423);

	return 1;
}


LUAMOD_API int
luaopen_pieces(lua_State *L) {
	luaL_checkversion(L);
	luaL_Reg l[] = {
		{ "new", new_pieces},
		{ NULL, NULL }
	};
	luaL_newlib(L, l);

	return 1;
}
