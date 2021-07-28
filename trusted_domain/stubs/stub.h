/* See LICENSE of license details. */
#ifndef _SYS_STUB_H
#define _SYS_STUB_H

#include <stdint.h>
#include <unistd.h>

static inline int _stub(int err)
{
    return -1;
}


#endif /* _SYS_STUB_H */
