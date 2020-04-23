#ifndef __LUA_PIECES_H__
#define __LUA_PIECES_H__
#include <inttypes.h>
// 固定时间: 2019-12-29 18:48:05
#define build_time 1577616485

enum pieces_type {
	unknown,
	user,
	player,
	role,
};

typedef struct pieces_op pieces_operations;
typedef struct pieces_link pieces_link;
typedef struct pieces_data pieces_data;


typedef struct pieces {
	uint32_t excel_id;
	union flags {
		struct {
			int64_t classify:6;			// 分类
			int64_t rand:12;			// 随机数：每秒最多生成4096个uniq tag
			int64_t dirty:1;			// 是否是脏数据
			int64_t copy:1;				// 是否是copy数据,具体没想到怎么实现，主要用来省内存
			int64_t data:1;				// 数据扩展（重要）
			int64_t dayreset:1;			// 重置
			int64_t timer:1;			// 定时任务
			int64_t life:1;				// 有效期
			int64_t online:1;			// 下线消失
			int64_t virtual:1;			// 虚拟物品
			int64_t effected:1;			// 影响父节点（需要更新）
			int64_t excel:1;			// excel模板已动态修改
			int64_t event:1;			// 事件监听中
			int64_t redmark:1;			// 红点
			int64_t root:1;				// 根
			int64_t policy:1;			// 策略
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

#endif // __LUA_PIECES_H__