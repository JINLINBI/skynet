#ifndef __LUA_PIECES_H__
#define __LUA_PIECES_H__
#include <inttypes.h>
#include "pieces.pb-c.h"
// 固定时间: 2020-04-1 00:00:00
#define BUILD_TIME 1585670400

enum pieces_type {
	unknown,
	user,
	player,
	role,
};

typedef struct pieces_op pieces_operations;
typedef struct pieces {
	uint32_t excel_id;
	union {
		struct
		{
			uint64_t classify : 6;   // 分类
			uint64_t rand     : 12;  // 随机数：每秒最多生成4096个uniq tag
			uint64_t dirty    : 1;   // 是否是脏数据
			uint64_t copy     : 1;   // 是否是copy数据,具体没想到怎么实现，主要用来省内存
			uint64_t data     : 1;   // 数据扩展（重要）
			uint64_t dayreset : 1;   // 重置
			uint64_t timer    : 1;   // 定时任务
			uint64_t life     : 1;   // 有效期
			uint64_t online   : 1;   // 下线消失
			uint64_t virtual  : 1;   // 虚拟物品(临时创建的玩家、队伍信息、下线干掉)
			uint64_t effected : 1;   // 影响父节点（需要更新）
			uint64_t excel    : 1;   // excel模板已动态修改
			uint64_t event    : 1;   // 事件监听中
			uint64_t redmark  : 1;   // 红点
			uint64_t root     : 1;   // 根
			uint64_t policy   : 1;   // 策略
			uint64_t test     : 1;   // 根
			uint64_t debug    : 1;   // 策略
			uint64_t born_time: 30;  // 可以存放30年不溢出
		} pi_flag;
		uint64_t only_id;
	} flag;
	void* link_data;
	void* excel_data;
}pieces;

typedef union pieces_flag {
	uint64_t classify : 6;   // 分类
	uint64_t rand     : 12;  // 随机数：每秒最多生成4096个uniq tag
	uint64_t dirty    : 1;   // 是否是脏数据
	uint64_t copy     : 1;   // 是否是copy数据,具体没想到怎么实现，主要用来省内存
	uint64_t data     : 1;   // 数据扩展（重要）
	uint64_t dayreset : 1;   // 重置
	uint64_t timer    : 1;   // 定时任务
	uint64_t life     : 1;   // 有效期
	uint64_t online   : 1;   // 下线消失
	uint64_t virtual  : 1;   // 虚拟物品(临时创建的玩家、队伍信息、下线干掉)
	uint64_t effected : 1;   // 影响父节点（需要更新）
	uint64_t excel    : 1;   // excel模板已动态修改
	uint64_t event    : 1;   // 事件监听中
	uint64_t redmark  : 1;   // 红点
	uint64_t root     : 1;   // 根
	uint64_t policy   : 1;   // 策略
	uint64_t born_time: 30;  // 可以存放30年不溢出
	uint64_t only_id;
} pieces_flag;

typedef struct pieces_board {
	pieces* pi;
} pieces_board;

typedef struct pieces_op
{
	int32_t (*init)(pieces* pi);
	int32_t (*attach)(pieces* pi, pieces* pi_attach);
	int32_t (*attach_by_type)(pieces* pi, uint32_t classify, uint32_t type);
	int32_t (*attach_by_onlyId)(pieces* pi, uint64_t onlyId);
	int32_t (*detach)(pieces* pi, pieces* pi_detach);
	int32_t (*detach_by_type)(pieces* pi, uint32_t classify, uint32_t type);
	int32_t (*detach_by_onlyId)(pieces* pi, uint64_t onlyId);
	int32_t (*detach_all)(pieces* pi);
	int32_t (*add)(pieces* pi, uint32_t copy, uint32_t parent);
	int32_t (*del)(pieces* pi, uint32_t copy, uint32_t parent);
	int32_t (*mod)(pieces* pi, uint32_t flag, uint32_t value);
	int32_t (*die)(pieces* pi);
	int32_t (*save)(pieces* pi);
	int32_t (*serialize)(pieces* pi, void* msg);
	int32_t (*unserialize)(pieces* pi, void* msg, uint32_t len);
	int32_t (*from)(pieces* pi, uint32_t excelId);
	int32_t (*load)(pieces* pi, uint64_t onlyId);
	int32_t (*update)(pieces* pi);
	int32_t (*change)(pieces* pi, uint32_t classify);
	int32_t (*tranlate)(pieces* pi, uint32_t type);
} pieces_operations;


typedef struct pieces_board_op
{
	int32_t (*game_start)();
	int32_t (*game_end)();
	int32_t (*game_)();
} pieces_board_operations;

#endif // __LUA_PIECES_H__