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


typedef struct pieces {
	int64_t excel_id:24;
	int64_t classify:4;
	int64_t dirty:1;
	int64_t copy:1;
	int64_t data:1;
	int64_t lock:1;
	int64_t born_time:30;
	pieces_operations * op;
	pieces_link * link;

}pieces;

typedef struct pieces_ops
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


#define piece_data_func(type, len_index) \
u##type##_t pieces_##type##_data(struct pieces* data, int idx) {\
	if (data->data_len[len_index] > idx)\
		return data->type##_data[idx];\
	return 0;\
}


piece_data_func(int8, int8_index);
piece_data_func(int16, int16_index);
piece_data_func(int32, int32_index);
piece_data_func(int64, int64_index);


static int id_func(lua_State * L) { 
	lua_pushinteger(L, 234234);

	//stack_dump(L);
	//luaL_error(L, ".................................");
	return 1;
}

static luaL_Reg arrayFunc [] = {
	// {"__index", excel_index},
	// {"__pairs", excel_pairs},
	// {"__len", excel_len},
	{"onlyId", id_func},
	{NULL, NULL}
};

int InitMetaTable(lua_State *L){
	luaL_newmetatable(L, "pieces_meta");
	luaL_setfuncs(L, arrayFunc, 0);
	return 1;
}


static int new_pieces(lua_State* L) {
	//stack_dump(L);
	pieces * p = (pieces*)lua_newuserdata(L, sizeof(pieces));
	memset(p, 0, sizeof(*p));

	InitMetaTable(L);
	luaL_getmetatable(L, "pieces_meta");
	lua_setmetatable(L, -2);
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
