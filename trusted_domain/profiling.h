#ifndef _PROFILING_H_
#define _PROFILING_H_


#include <stdint.h>
#include <string.h>

#define PROF_DECAY            1   // whether hitCnt should decay, faster decay makes most time consuming functions seems to have even more hit count
#define PROF_CNT              30
#define PROF_ERR              1
#define PROF_HITCNT_INC       10

//#define PROF_CUSTOMIZE_PROC

extern void print_profiling(void);

#endif /* _PROFILING_H_ */
