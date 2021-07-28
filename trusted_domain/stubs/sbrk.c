/* See LICENSE of license details. */

#include <stddef.h>
#include <errno.h>
#include "FreeRTOS.h"
#include "task.h"

extern char heap_start, heap_end;
static size_t heapBytesRemaining = -1;

char *sbrk(int incr)
{
    static char *currentHeapEnd = &heap_start;

    vTaskSuspendAll();

    if(heapBytesRemaining == -1)
    {
        heapBytesRemaining = ((size_t)&heap_end)-((size_t)&heap_start);
    }

    char *previousHeapEnd = currentHeapEnd;
    if(currentHeapEnd + incr >= &heap_end)
    {
        _impure_ptr->_errno = ENOMEM;
        xTaskResumeAll();
        return (char *)-1;
    }
    currentHeapEnd += incr;
    heapBytesRemaining -= incr;
    xTaskResumeAll();
    return (char *)previousHeapEnd;
}

int get_heap_bytes_remaining_size(void)
{
    if(heapBytesRemaining == -1)
    {
        heapBytesRemaining = ((size_t)&heap_end)-((size_t)&heap_start);
    }
    return heapBytesRemaining;
}

char *_sbrk(int incr)
{
    return sbrk(incr);
}

void __malloc_lock(struct _reent *p)
{
    vTaskSuspendAll();
}

void __malloc_unlock(struct _reent *p)
{
    (void)xTaskResumeAll();
}

void __env_lock()
{
    vTaskSuspendAll();
}

void __env_unlock()
{
    (void)xTaskResumeAll();
}

void *__wrap_malloc(size_t nbytes)
{
    extern void *__real_malloc(size_t nbytes);
    void *p = __real_malloc(nbytes);
    return p;
}

void *__wrap__malloc_r(void *reent, size_t nbytes)
{
    extern void *__real__malloc_r(size_t nbytes);
    void *p = __real__malloc_r(nbytes);
    return p;
}
