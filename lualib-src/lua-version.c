#include <lua.h>
#include <lauxlib.h>
#include <stdatomic.h>
#include <stdint.h>
#include <string.h>
#include "skynet_malloc.h"

#define HASH_CAPACITY 2039 // 使用质数减少冲突，2039（大概支持一万数据量）
#define MAX_KEY_LEN 63
#define INIT_VALUE 1 // 初始化version的值

typedef struct HashNode
{
    char key[MAX_KEY_LEN + 1];
    atomic_int value;
    atomic_uintptr_t next; // 原子指针
} HashNode;

typedef struct
{
    atomic_uintptr_t buckets[HASH_CAPACITY]; // 原子化的桶数组
    atomic_size_t version_counter;           // 全局版本号
} VersionStore;

static atomic_uintptr_t g_store = 0;

// 内存屏障快捷方式
#define LOAD_ACQUIRE(ptr) atomic_load_explicit(ptr, memory_order_acquire)
#define STORE_RELEASE(ptr, val) atomic_store_explicit(ptr, val, memory_order_release)
#define CAS_WEAK(ptr, expected, desired)                          \
    atomic_compare_exchange_weak_explicit(ptr, expected, desired, \
                                          memory_order_acq_rel, memory_order_acquire)

// FNV-1a哈希算法
static uint32_t hash_func(const char *key)
{
    uint32_t hash = 2166136261U;
    for (int i = 0; i < MAX_KEY_LEN && *key; ++i)
    {
        hash ^= (uint8_t)*key++;
        hash *= 16777619U;
    }
    return hash % HASH_CAPACITY;
}

// 无锁查找
static HashNode *hash_find(VersionStore *store, const char *key)
{
    uint32_t idx = hash_func(key);
    HashNode *node = (HashNode *)LOAD_ACQUIRE(&store->buckets[idx]);

    while (node)
    {
        if (strcmp(node->key, key) == 0)
            break;
        node = (HashNode *)LOAD_ACQUIRE(&node->next);
    }
    return node;
}

// 无锁插入（带版本号的CAS）
static HashNode *hash_insert(VersionStore *store, const char *key)
{
    uint32_t idx = hash_func(key);
    HashNode *new_node = skynet_malloc(sizeof(HashNode));
    strncpy(new_node->key, key, MAX_KEY_LEN);
    atomic_init(&new_node->value, INIT_VALUE);
    atomic_init(&new_node->next, 0);

    atomic_uintptr_t *bucket = &store->buckets[idx];
    uintptr_t expected = LOAD_ACQUIRE(bucket);

    while (1)
    {
        // 检查是否已存在
        HashNode *current = (HashNode *)expected;
        while (current)
        {
            if (strcmp(current->key, key) == 0)
            {
                skynet_free(new_node);
                return current;
            }
            current = (HashNode *)LOAD_ACQUIRE(&current->next);
        }

        // 尝试插入新节点
        atomic_store(&new_node->next, expected);
        uintptr_t desired = (uintptr_t)new_node;

        if (CAS_WEAK(bucket, &expected, desired))
        {
            atomic_fetch_add_explicit(&store->version_counter, 1, memory_order_relaxed);
            return new_node;
        }
    }
}

static VersionStore *get_store();
// Lua API实现
static int version_get(lua_State *L)
{
    const char *key = luaL_checkstring(L, 1);
    VersionStore *store = get_store();
    HashNode *node = hash_find(store, key);
    lua_pushinteger(L, node ? atomic_load(&node->value) : 0);
    return 1;
}

static int version_update(lua_State *L)
{
    const char *key = luaL_checkstring(L, 1);
    VersionStore *store = get_store();
    HashNode *node = hash_find(store, key);

    if (!node)
    {
        node = hash_insert(store, key);
    }
    else
    {
        atomic_fetch_add(&node->value, 1);
    }

    int num = atomic_load(&node->value);
    lua_Integer to = luaL_optinteger(L, 2, num);
    if (num != to)
    {
        lua_pushinteger(L, atomic_compare_exchange_strong(&node->value, &num, to)? to: num);
        return 1;
    }
    lua_pushinteger(L, to);
    return 1;
}

// 初始化全局存储
static void init_store()
{
    uintptr_t expected = 0;
    VersionStore *new_store = skynet_malloc(sizeof(VersionStore));
    memset(new_store, 0, sizeof(VersionStore));

    // 原子CAS操作
    if (!atomic_compare_exchange_strong(&g_store, &expected, (uintptr_t)new_store))
    {
        skynet_free(new_store); // 失败时释放多余内存
    }
}

static VersionStore *get_store()
{
    VersionStore *store = (VersionStore *)atomic_load(&g_store);
    if (!store)
    {
        init_store(); // 可能被其他线程抢占
        store = (VersionStore *)atomic_load(&g_store);
    }
    return store;
}

// 注册Lua模块
static const luaL_Reg version_lib[] = {
    {"get", version_get},
    {"update", version_update},
    {NULL, NULL}};

int luaopen_version(lua_State *L)
{
    init_store();
    luaL_newlib(L, version_lib);
    return 1;
}