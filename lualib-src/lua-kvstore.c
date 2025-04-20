// kvstore.c

#include <lua.h>
#include <lauxlib.h>
#include <stdatomic.h>
#include <string.h>
#include "rwlock.h"
#include "skynet_malloc.h"

#define HASH_CAPACITY 2039 // 哈希桶数量（质数减少碰撞）
#define MAX_KEY_LEN 63     // 键最大长度

typedef struct HashNode
{
    char key[MAX_KEY_LEN + 1]; // 键（栈内存保证原子写）
    atomic_uintptr_t val_ptr;  // 原子化字符串指针
    atomic_uintptr_t next;     // 链表指针
} HashNode;

typedef struct
{
    atomic_uintptr_t buckets[HASH_CAPACITY];   // 原子桶数组
    struct rwlock bucket_locks[HASH_CAPACITY]; // 分段读写锁
} StringKVStore;

// Lua API
int luaopen_kvstore(lua_State *L);

static atomic_uintptr_t g_store = 0;

// ------------------------
// 核心哈希算法 (FNV-1a)
// ------------------------
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

static void atomic_str_store(atomic_uintptr_t *dest, const char *value)
{
    char *new_val = skynet_strdup(value);
    char *old_val = (char *)atomic_exchange_explicit(dest, (uintptr_t)new_val,
                                                     memory_order_release);
    if (old_val)
        skynet_free(old_val);
}

// ------------------------
// 哈希表操作（线程安全）
// ------------------------
static HashNode *hash_find(StringKVStore *store, const char *key)
{
    uint32_t idx = hash_func(key);
    rwlock_rlock(&store->bucket_locks[idx]);

    HashNode *node = (HashNode *)atomic_load_explicit(&store->buckets[idx],
                                                      memory_order_consume);
    while (node)
    {
        if (strcmp(node->key, key) == 0)
            break;
        node = (HashNode *)atomic_load_explicit(&node->next, memory_order_consume);
    }

    rwlock_runlock(&store->bucket_locks[idx]);
    return node;
}

static HashNode *hash_insert(StringKVStore *store, const char *key, const char *value)
{
    uint32_t idx = hash_func(key);
    rwlock_wlock(&store->bucket_locks[idx]);

    HashNode *new_node = skynet_malloc(sizeof(HashNode));
    strncpy(new_node->key, key, MAX_KEY_LEN);
    atomic_init(&new_node->val_ptr, (uintptr_t)skynet_strdup(value));
    atomic_init(&new_node->next, 0);

    HashNode *expected = (HashNode *)atomic_load(&store->buckets[idx]);
    while (1)
    {
        HashNode *current = expected;
        while (current)
        {
            if (strcmp(current->key, key) == 0)
            {
                atomic_str_store(&current->val_ptr, value);
                skynet_free(new_node);
                rwlock_wunlock(&store->bucket_locks[idx]);
                return current;
            }
            current = (HashNode *)atomic_load(&current->next);
        }

        atomic_store(&new_node->next, (uintptr_t)expected);
        if (atomic_compare_exchange_weak(&store->buckets[idx],
                                         (uintptr_t *)&expected, (uintptr_t)new_node))
        {
            break;
        }
    }

    rwlock_wunlock(&store->bucket_locks[idx]);
    return new_node;
}

// ------------------------
// Lua API实现
// ------------------------
static int kv_get(lua_State *L)
{
    const char *key = luaL_checkstring(L, 1);
    StringKVStore *store = (StringKVStore *)atomic_load(&g_store);
    HashNode *node = hash_find(store, key);

    if (node)
    {
        char *val = (char *)atomic_load_explicit(&node->val_ptr, memory_order_acquire);
        lua_pushstring(L, val);
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

static int kv_set(lua_State *L)
{
    const char *key = luaL_checkstring(L, 1);
    const char *value = luaL_checkstring(L, 2);
    StringKVStore *store = (StringKVStore *)atomic_load(&g_store);

    HashNode *node = hash_find(store, key);
    if (!node)
    {
        node = hash_insert(store, key, value);
    }
    else
    {
        atomic_str_store(&node->val_ptr, value);
    }

    lua_pushboolean(L, 1);
    return 1;
}

// ------------------------
// 初始化与销毁
// ------------------------
static void init_store()
{
    StringKVStore *store = (StringKVStore *)atomic_load(&g_store);
    if (store != NULL)
    {
        return;
    }

    store = skynet_malloc(sizeof(StringKVStore));
    memset(store, 0, sizeof(StringKVStore));

    for (int i = 0; i < HASH_CAPACITY; ++i)
    {
        rwlock_init(&store->bucket_locks[i]);
    }

    atomic_store(&g_store, (uintptr_t)store);
}

int luaopen_kvstore(lua_State *L)
{
    init_store();
    static const luaL_Reg lib[] = {
        {"get", kv_get},
        {"set", kv_set},
        {NULL, NULL}};
    luaL_newlib(L, lib);
    return 1;
}