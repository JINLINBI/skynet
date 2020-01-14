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


static MYSQL * pmysql = NULL;

enum pieces_type {
	user,
	player,
	role,
};

// 固定时间: 2019-12-29 18:48:05
#define build_time 1577616485


typedef struct pieces_op pieces_operations;
typedef struct pieces_link pieces_link;
typedef struct pieces_data pieces_data;


typedef struct pieces {
	uint32_t excel_id;
	union flags {
		struct {
			int64_t classify:6;			// 分类
			int64_t rand:12;			// 独立标志：每秒最多生成4096个uniq tag
			int64_t dirty:1;			// 是否是脏数据
			int64_t copy:1;				// 是否是copy数据,具体没想到怎么实现，主要用来省内存
			int64_t data:1;				// 数据扩展（重要）
			int64_t lock:1;				// 加锁(暂时没用，可能要用spinlock)
			int64_t mm:1;				// 存放内存的数据，不会持久化（如：玩家上线生成的属性数据)			
			int64_t timer:1;			// 是否具有定时任务
			int64_t life:1;				// 是否具有有效期
			int64_t online:1;			// 下线消失
			int64_t virtual:1;			// 虚拟物品
			int64_t effect:1;			// 影响父节点（需要更新）
			int64_t excel:1;			// excel模板已动态修改
			int64_t event:1;			// 事件监听中
			int64_t reverse4:1;			// 保存
			int64_t reverse5:1;			// 保存
			int64_t born_time:30;		// 可以存放30年不溢出
		} flag;
		int64_t onlyId;
	} flags;
	pieces_link * link;
}pieces;


typedef struct pieces_op
{
	uint32_t (*init)(pieces* pi);
	uint32_t (*attach)(pieces* pi, pieces * pi_attach);
	uint32_t (*attach_by_type)(pieces* pi, uint32_t classify, uint32_t type);
	uint32_t (*attach_by_onlyId)(pieces* pi, uint64_t onlyId);
	uint32_t (*detach)(pieces* pi, pieces* pi_detach);
	uint32_t (*detach_by_type)(pieces* pi, uint32_t classify, uint32_t type);
	uint32_t (*detach_by_onlyId)(pieces* pi, uint64_t onlyId);
	uint32_t (*detach_all)(pieces* pi);
	uint32_t (*add)(pieces* pi, uint32_t copy, uint32_t parent);
	uint32_t (*del)(pieces* pi, uint32_t copy, uint32_t parent);
	uint32_t (*mod)(pieces* pi, uint32_t flag, uint32_t value);
	uint32_t (*die)(pieces* pi);
	uint32_t (*save)(pieces* pi);
	uint32_t (*serialize)(pieces* pi, void *msg);
	uint32_t (*unserialize)(pieces* pi, void *msg, uint32_t len);
	uint32_t (*from)(pieces* pi, uint32_t excelId);
	uint32_t (*load)(pieces* pi, uint64_t onlyId);
	uint32_t (*update)(pieces* pi);
	uint32_t (*change)(pieces* pi, uint32_t classify);
	uint32_t (*tranlate)(pieces* pi, uint32_t type);
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


static pieces* root = NULL;
static struct spinlock lock;


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
	define_pieces_flag_get(mm)
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

	get_pieces_flag(L, index_name, pi);

	return 1;
}

static int pieces_save_func(lua_State * L) {
	printf("in pieces_save_function.\n");
	lua_getfield(L, 1, "pieces_userdata");
	pieces * pi = (pieces*) lua_touserdata(L, -1);
	if (pi == NULL) {
		printf("in pieces_save_function. pi == NULL \n");
		stack_dump(L);
		lua_pushboolean(L, 0);
		return 0;
	}

	printf("in pieces_save_function. pi != NULL \n");

	stack_dump(L);
	char sql_buffer[256];
	memset(sql_buffer, 0, sizeof(sql_buffer));
	sprintf(sql_buffer, "insert into pieces(id, excelId) values(%lu, %d)", pi->flags.onlyId, pi->excel_id);

	printf("query mysql: %s\n", sql_buffer);

	mysql_real_query(pmysql, sql_buffer, strlen(sql_buffer));

	luaL_dofile(L, "test_ccall.lua");
	lua_getglobal(L, "callfromc");
	lua_call(L, 0, 0);
	lua_pushboolean(L, 1);


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
	luaL_newlib(L, l);

	
	pmysql = mysql_init(NULL);
	MYSQL * ret = mysql_real_connect(pmysql, "localhost", "dev", "123456", "dev", 0, NULL, 0);

	if (ret == NULL) {
		fprintf(stderr, "connect mysql error:%s \n", /* mysql_errno(pmysql),*/ mysql_error(pmysql));
		return -1;
	}
	else {
		fprintf(stdout, "connected mysql server.\n");
	}


	return 1;
}
