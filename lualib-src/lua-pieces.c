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

int get_pieces_excel_data(lua_State* L, pieces* pi, const char* index_name, cJSON* line_fields) {
	if (pi == NULL || !pi->excel_data) {
		return 0;
	}

	PieceExcelData* excel_data = (PieceExcelData*) pi->excel_data;
	for (int32_t i = 0; i < excel_data->n_item; i++) {
		if (!strcmp(excel_data->item[i]->key, index_name)) {
			PiecesExcelDataItem* excel_item = excel_data->item[i]->value;
			cJSON* fields = cJSON_GetObjectItem(line_fields, index_name);
			if (!strcmp(fields->valuestring, "number")) {
				lua_pushinteger(L, excel_item->number);
			}
			else if (!strcmp(fields->valuestring, "string")) {
				lua_pushstring(L, excel_item->str? excel_item->str: "");
			}
			else if (!strcmp(fields->valuestring, "number[]")) {
				lua_newtable(L);
				for (int32_t index = 1; index <= excel_item->n_number_list; index++) {
					lua_pushinteger(L, index);
					lua_pushinteger(L, excel_item->number_list[index - 1]);
					lua_settable(L, -3);
				}
			}
			else if (!strcmp(fields->valuestring, "string[]")) {
				lua_newtable(L);
				for (int32_t index = 1; index <= excel_item->n_str_list; index++) {
					lua_pushinteger(L, index);
					lua_pushstring(L, excel_item->str_list[index - 1]);
					lua_settable(L, -3);
				}
			}
			return 1;
		}
	}

	return 0;
}

void add_pieces_excel_data_number_list(lua_State* L, PiecesExcelDataItem* excel_item) {
	int32_t len = lua_rawlen(L, 3);
	if (len <= 0) {
		return;
	}

	excel_item->number_list = skynet_malloc(sizeof(void*) * len);
	int index = lua_gettop(L);
	int list_index = 0;
	lua_pushnil(L);
	while (lua_next(L, index)) {
		/* 此时栈上 -1 处为 value, -2 处为 key */
		excel_item->number_list[list_index++] = lua_tointeger(L, -1);
		excel_item->n_number_list++;
		lua_pop(L, 1);
	}
}


void add_pieces_excel_data_string_list(lua_State* L, PiecesExcelDataItem* excel_item) {
	int32_t len = lua_rawlen(L, 3);
	if (len <= 0) {
		return;
	}

	excel_item->str_list = skynet_malloc(sizeof(void*) * len);
	int index = lua_gettop(L);
	int list_index = 0;
	lua_pushnil(L);
	while (lua_next(L, index)) {
		/* 此时栈上 -1 处为 value, -2 处为 key */
		const char* value = lua_tostring(L, -1);
		excel_item->str_list[list_index++] = skynet_strdup(value? value: "");
		excel_item->n_str_list++;

		lua_pop(L, 1);
	}
}

int set_pieces_excel_data(lua_State* L, pieces* pi, const char* index_name, cJSON* line_fields) {
	if (pi == NULL || !pi->excel_data) {
		return 0;
	}

	PieceExcelData* excel_data = (PieceExcelData*) pi->excel_data;
	for (int32_t i = 0; i < excel_data->n_item; i++) {
		if (!strcmp(excel_data->item[i]->key, index_name)) {
			PiecesExcelDataItem* excel_item = excel_data->item[i]->value;
			if (lua_isnil(L, 3)) {
				skynet_free(excel_data->item[i]->value);

				while (i + 1 < excel_data->n_item) {
					excel_data->item[i] = excel_data->item[i + 1];
					i++;
				}
				excel_data->n_item--;
				return 0;
			}

			cJSON* fields = cJSON_GetObjectItem(line_fields, index_name);
			if (!strcmp(fields->valuestring, "number")) {
				excel_item->number = lua_tointeger(L, 3);
			}
			else if (!strcmp(fields->valuestring, "string")) {
				skynet_free(excel_item->str);
				excel_item->str = skynet_strdup(lua_tostring(L, 3));
			}
			else if (!strcmp(fields->valuestring, "number[]")) {
				if (lua_istable(L, 3)) {
					excel_item->n_number_list = 0;
					skynet_free(excel_item->number_list);
					add_pieces_excel_data_number_list(L, excel_item);
				}
			}
			else if (!strcmp(fields->valuestring, "string[]")) {
				if (lua_istable(L, 3)) {
					excel_item->n_str_list = 0;
					skynet_free(excel_item->str_list);
					add_pieces_excel_data_string_list(L, excel_item);
				}
			}
			return 0;
		}
	}

	return 0;
}

int add_pieces_excel_data(lua_State* L, pieces* pi, const char* index_name, cJSON* line_fields) {
	if (lua_isnil(L, 3)) {
		return 0;
	}
	
	cJSON* fields = cJSON_GetObjectItem(line_fields, index_name);
	if (fields == NULL) {
		return 0;
	}

	if (pi->excel_data == NULL) {
		pi->excel_data = skynet_malloc(sizeof(PieceExcelData));
		piece_excel_data__init(pi->excel_data);
		if (pi->excel_data == NULL) {
			return 0;
		}
		pi->flag.pi_flag.is_excel = 1;
	}

	pi->excel_data->n_item++;
	pi->excel_data->item = skynet_realloc(pi->excel_data->item, pi->excel_data->n_item * sizeof(void*));
	// TODO: NULL 

	pi->excel_data->item[pi->excel_data->n_item - 1] = skynet_malloc(sizeof(PieceExcelData__ItemEntry));
	PieceExcelData__ItemEntry* excel_data_item_en = pi->excel_data->item[pi->excel_data->n_item - 1];
	piece_excel_data__item_entry__init(excel_data_item_en);
	excel_data_item_en->key = skynet_strdup(index_name);
	excel_data_item_en->value = skynet_malloc(sizeof(PiecesExcelDataItem));

	PiecesExcelDataItem* excel_data_item = excel_data_item_en->value;
	pieces_excel_data_item__init(excel_data_item);
	if (!strcmp(fields->valuestring, "number")) {
		excel_data_item->number = lua_tointeger(L, 3);
	}
	else if (!strcmp(fields->valuestring, "string")) {
		excel_data_item->str = skynet_strdup(lua_tostring(L, 3));
	}
	else if (!strcmp(fields->valuestring, "number[]")) {
		add_pieces_excel_data_number_list(L, excel_data_item);
	}
	else if (!strcmp(fields->valuestring, "string[]")) {
		add_pieces_excel_data_string_list(L, excel_data_item);
	}

	return 0;
}


int excel_line_newindex(lua_State* L) {
	excel_line* data = (excel_line*) lua_touserdata(L, 1);
	luaL_argcheck(L, data != NULL, 1, "'excel_line userdata' expected");

	const char* index_name = lua_tostring(L, 2);
	pieces* pi = data->pi;
	if (pi == NULL || index_name == NULL) {
		return 0;
	}

	if (set_pieces_excel_data(L, pi, index_name, data->line_fields)) {
		return 0;
	}

	add_pieces_excel_data(L, pi, index_name, data->line_fields);
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
		lua_pushinteger(L, pi->flag.only_id);
	}
	else if (!strcmp(name, "born_time")) {
		lua_pushinteger(L, pi->flag.pi_flag.born_time + BUILD_TIME);
	}
	else if (!strcmp(name, "build_time")) {
		lua_pushinteger(L, BUILD_TIME);
	}
	define_pieces_flag_get(is_dirty)
	define_pieces_flag_get(is_data)
	define_pieces_flag_get(is_copy)
	define_pieces_flag_get(is_redmark)
	define_pieces_flag_get(is_dayreset)
	define_pieces_flag_get(is_excel)
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
	define_pieces_flag_set(is_dirty, val)
	define_pieces_flag_set(is_data, val)
	define_pieces_flag_set(is_copy, val)
	define_pieces_flag_set(is_redmark, val)
	define_pieces_flag_set(is_dayreset, val)
	define_pieces_flag_set(is_excel, val)

	return 0;
}

static int pieces_excel_index(lua_State* L, pieces* pi, int32_t excel_id) {
	rb_cjson_line* line = rb_search_cjson_line(&((inst->rb_cjson_files_root)[1]->rb_root), excel_id);
	if (line) {
		CreateExcelLineUserData(L, line->cjson_line_data, line->cjson_line_fields, excel_id, -1, pi);
	}
	else {
		lua_pushnil(L);
	}

	return 1;
}

static int pieces_index_func(lua_State* L) {
	pieces * pi = (pieces*) lua_touserdata(L, 1);
	const char* index_name = lua_tostring(L, 2);
	if (pi == NULL) {
		lua_pushnil(L);
		return 0;
	}
	else if (!strcmp(index_name, "excel")) {
		pieces_excel_index(L, pi, pi->excel_id);
	}
	else if (!strcmp(index_name, "__excel")) {
		pieces_excel_index(L, NULL, pi->excel_id);
	}
	else {
		get_pieces_flag(L, index_name, pi);
	}

	return 1;
}

static int pieces_newindex_func(lua_State* L) {
	pieces * pi = (pieces*) lua_touserdata(L, 1);
	const char* index_name = lua_tostring(L, 2);
	if (pi == NULL) {
		return 0;
	}
	else if (!strcmp(index_name, "excel")) {
		pieces_excel_index(L, pi, pi->excel_id);
	}
	else if (!strcmp(index_name, "__excel")) {
		pieces_excel_index(L, NULL, pi->excel_id);
	}
	else {
		int64_t val = lua_tointeger(L, 3);
		set_pieces_flag(L, index_name, val, pi);
	}

	return 0;
}

static int pieces_save_func(lua_State* L) {
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

int InitPiecesMetaTable(lua_State* L){
	luaL_newmetatable(L, "pieces_meta");
	luaL_setfuncs(L, arrayFunc, 0);
	return 1;
}

static int new_pieces(lua_State* L) {
	pieces * pi = (pieces*) lua_newuserdata(L, sizeof(pieces));
	if (pi == NULL) {
		lua_pushnil(L);
		return 1;
	}
	memset(pi, 0, sizeof(*pi));
	pi->flag.pi_flag.born_time = time(NULL) - BUILD_TIME;

	luaL_getmetatable(L, "pieces_meta");
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

	luaL_getmetatable(L, "pieces_meta");
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

	InitPiecesMetaTable(L);
	InitExcelLineMetaTable(L);
	InitExcelListMetaTable(L);
	init_excel_root(L, inst);
	luaL_newlib(L, l);
	return 1;
}
