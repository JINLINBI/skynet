#include <lua.h>
#include <lauxlib.h>
#include <stdatomic.h>
#include <string.h>
#include "spinlock.h"
#include "atomic.h"

#define HASH_CAPACITY 128 // 建议使用质数如127/521

typedef struct HashNode
{
    char key[64];
    ATOM_INT value;
    struct HashNode *next; // 链地址法解决冲突
} HashNode;

typedef struct
{
    HashNode *buckets[HASH_CAPACITY];
    struct spinlock lock;
} VersionStore;

static VersionStore *g_store = NULL;

// 哈希函数（FNV-1a算法）
static uint32_t hash_func(const char *key)
{
    
    uint32_t hash = 2166136261U;
    int max = 64;
    while (*key && max--)
    {
        hash ^= (uint8_t)(*key++);
        hash *= 16777619;
    }
    return hash % HASH_CAPACITY;
}

// 查找节点（线程安全）
static HashNode *hash_find(VersionStore *store, const char *key)
{
    uint32_t idx = hash_func(key);
    int max = 2048;
    SPIN_LOCK(store);
    HashNode *node = store->buckets[idx];
    while (node && max)
    {
        if (strcmp(node->key, key) == 0)
            break;
        node = node->next;
    }
    SPIN_UNLOCK(store);
    return node;
}

// 插入节点（线程安全）
static HashNode *hash_insert(VersionStore *store, const char *key)
{
    uint32_t idx = hash_func(key);
    SPIN_LOCK(store);
    HashNode **pp = &store->buckets[idx];
    while (*pp)
    {
        if (strcmp((*pp)->key, key) == 0)
        {
            SPIN_UNLOCK(store);
            return *pp;
        }
        pp = &(*pp)->next;
    }
    HashNode *new_node = malloc(sizeof(HashNode));
    strncpy(new_node->key, key, sizeof(new_node->key) - 1);
    ATOM_INIT(&new_node->value, 1);
    new_node->next = NULL;
    *pp = new_node;
    SPIN_UNLOCK(store);
    return new_node;
}

// 初始化全局存储（仅首次加载执行）
static void init_store(lua_State *L)
{
    if (!g_store)
    {
        g_store = (VersionStore *)lua_newuserdatauv(L, sizeof(VersionStore), 0);
        memset(g_store, 0, sizeof(VersionStore));
        SPIN_INIT(g_store)
    }
}

// 获取字段版本号（原子操作）
static int version_get(lua_State *L)
{
    const char *field = luaL_checkstring(L, 1);

    HashNode *hn = hash_find(g_store, field);

    lua_pushinteger(L, hn ? ATOM_LOAD(&hn->value) : 0);
    // lua_pushinteger(L, 0);
    return 1;
}

// 递增字段版本号（线程安全）
static int version_update(lua_State *L)
{
    const char *field = luaL_checkstring(L, 1);

    HashNode *hn = hash_find(g_store, field);
    if (hn)
    {
        ATOM_FINC(&hn->value);
    }
    else
    {
        hn = hash_insert(g_store, field);
    }

    lua_pushinteger(L, ATOM_LOAD(&hn->value));
    return 1;
}

// 注册Lua模块
static const luaL_Reg version_lib[] = {
    {"get", version_get},
    {"update", version_update},
    {NULL, NULL}};

int luaopen_version(lua_State *L)
{
    init_store(L);
    luaL_newlib(L, version_lib);
    return 1;
}