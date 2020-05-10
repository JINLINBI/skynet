#ifndef __LUA_EXCEL_H__
#define __LUA_EXCEL_H__
#include "skynet.h"

#include <lua.h>
#include <lauxlib.h>
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
	rb_cjson_root * rbtree_root;
} excel_list;

typedef struct pieces pieces;
typedef struct excel_line {
	int id;
	int size;
	cJSON * line_data;
	cJSON * line_fields;
	pieces * pi;
} excel_line;

typedef struct skynet_context {
	void * instance;
} skynet_context;

int excel_list_index(lua_State* L);
int excel_list_pairs(lua_State* L);
int excel_list_len(lua_State* L);

int excel_line_index(lua_State* L);
int excel_line_newindex(lua_State* L);
int excel_line_pairs(lua_State* L);
int excel_line_len(lua_State* L);
int excel_line_tostring(lua_State* L);


int InitExcelListMetaTable(lua_State *L);
int InitExcelLineMetaTable(lua_State *L);
void CreateExcelLineUserData(lua_State *L, cJSON * data, cJSON * fields, int32_t index, int32_t id, pieces* pi);
void init_excel_root(lua_State* L, excel_service* inst);

void stack_dump(lua_State* L);

// pieces lib
int set_pieces_excel_data(lua_State* L, pieces* pi, const char* index_name, cJSON* line_fields);
int get_pieces_excel_data(lua_State* L, pieces* pi, const char* index_name, cJSON* line_fields);
#endif //__LUA_EXCEL_H__