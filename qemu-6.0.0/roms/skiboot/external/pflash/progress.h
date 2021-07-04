#ifndef __PROGRESS_H
#define __PROGRESS_H

#include <inttypes.h>

void progress_init(uint64_t count);
void progress_tick(uint64_t cur);
void progress_end(void);

#endif /* __PROGRESS_H */
