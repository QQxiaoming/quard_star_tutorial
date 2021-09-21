#include "profiling.h"
#include "debug_log.h"


#define PROF_MASK       (~(PROF_ERR - 1))
#define ARRAY_SIZE(x)   (sizeof(x) / sizeof((x)[0]))

typedef struct
{
	uint64_t base_addr; /*!< (aligned) base address range of PC sample            */
	uint64_t hit_cnt;   /*!< hit count (a decay mecahnism automatically drops it) */
	uint64_t hit_ratio; /*!< 10-bit resolution hit ratio,                         */
} prof_unit_t;

typedef struct
{
	uint32_t prof_cnt; /*!< totoal hit count of profiling  */
	uint32_t decay_ndx; /*!< which item to decay its hit_cnt */
	prof_unit_t items[PROF_CNT];
} prof_t;

prof_t call_prof;
uint64_t ignr_list[] =
{
	0xbf802a0a,
	// todo: add pc address ranges that you do not care, such as idle function.
    0,
    0,
};

/**
 * @brief 记录采样pc地址
 *
 * @param p_item 记录列表
 * @param pc 样本pc指针
 */
void __attribute__ ((no_instrument_function)) prof_on_hit(prof_unit_t *p_item, uint64_t pc)
{
	/* 记录地址，增加hitcnt，计算hitRatio */
	p_item->base_addr = pc & PROF_MASK;
	call_prof.prof_cnt += PROF_HITCNT_INC;
	p_item->hit_cnt += PROF_HITCNT_INC;
	p_item->hit_ratio =(uint32_t)(((uint64_t)(p_item->hit_cnt) << 10) / call_prof.prof_cnt);

	/* Item冒泡排序 */
	prof_unit_t tmp_item;
	for (; p_item != call_prof.items && p_item[0].hit_cnt > p_item[-1].hit_cnt; p_item--)
	{
		tmp_item = p_item[0];
		p_item[0] = p_item[-1];
		p_item[-1] = tmp_item;
	}
}

/**
 * @brief 采样PC指针
 *
 * @param pc 样本PC指针
 */
void __attribute__ ((no_instrument_function)) profiling(uint32_t pc)
{
	uint32_t i;
	prof_unit_t *p_item = &call_prof.items[0];

	/* 遍历忽略列表，忽略指定区域pc */
	for (i = 0; i < ARRAY_SIZE(ignr_list); i++)
	{
		if (pc - ignr_list[i] < PROF_ERR)
        {
			return;
        }
	}

#if PROF_DECAY
	/* 衰减处理，定时减少hitCnt数，以便忽略仅在初始化时密集度高的代码 */
	if (call_prof.items[call_prof.decay_ndx].hit_cnt > 1)
	{
		call_prof.items[call_prof.decay_ndx].hit_cnt--;
		call_prof.prof_cnt--;
	}
	if (++call_prof.decay_ndx == PROF_CNT)
    {
		call_prof.decay_ndx = 0;
    }
#endif

	uint32_t free_ndx = PROF_CNT;

	/* 搜索已经存在的pc样本 */
	for (i = 0, p_item = call_prof.items; i < PROF_CNT; i++, p_item++)
	{
		if (p_item->base_addr == (pc & PROF_MASK))
		{
			prof_on_hit(p_item, pc);
			break;
		}
		else if (free_ndx == PROF_CNT && p_item->hit_cnt == 0)
		{
			/* 记录空的items */
			free_ndx = i;
		}
	}

	if (i == PROF_CNT)
	{
		if (free_ndx < PROF_CNT)
		{
			/* 分配新items */
			prof_on_hit(call_prof.items + free_ndx, pc);
		}
		else
		{
			/* 替换最后一个items */
			free_ndx = PROF_CNT - 1;
			call_prof.prof_cnt -= call_prof.items[free_ndx].hit_cnt;
			call_prof.items[free_ndx].hit_cnt = 0;
			prof_on_hit(call_prof.items + free_ndx, pc);
		}
	}
}


void __attribute__ ((no_instrument_function)) _mcount(uint64_t caller_ra)
{
    uint64_t curr_ra;
	__asm__ __volatile__("mv %0, ra":"=r"(curr_ra));
#ifndef PROF_CUSTOMIZE_PROC 
    profiling(curr_ra-4);
#else 
    /* TODO: 
       1.通过自定义协议将curr_ra数据发出进行处理，例如通过串口发送到pc处理或
        写入flash/sd等存储设备。
       2.caller_ra是我们采集到的函数结束时的返回地址，因此有条件的情况下尝试
         对这个地址的代码段进行修改甚至可以动态hook这个函数的end时机，因此可
         以对函数进行trace，例如统计函数的执行时间，这里仅抛砖引玉。
     */
#endif
}

void print_profiling(void)
{
#ifndef PROF_CUSTOMIZE_PROC 
    for(uint64_t i=0;i<PROF_CNT;i++)
    {
        debug_log("No. %d\n",i);
        debug_log("--- base_addr\t 0x%lx\n",call_prof.items[i].base_addr);
        debug_log("--- hit_cnt\t %ld\n",call_prof.items[i].hit_cnt);
        debug_log("--- hit_ratio\t %ld\n",call_prof.items[i].hit_ratio);
    }
#endif
}