// String manipulation functions.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "stacks.h" // yield
#include "string.h" // memcpy
#include "farptr.h" // SET_SEG


/****************************************************************
 * String ops
 ****************************************************************/

// Sum the bytes in the specified area.
u8
checksum_far(u16 buf_seg, void *buf_far, u32 len)
{
    SET_SEG(ES, buf_seg);
    u32 i;
    u8 sum = 0;
    for (i=0; i<len; i++)
        sum += GET_VAR(ES, ((u8*)buf_far)[i]);
    return sum;
}

u8
checksum(void *buf, u32 len)
{
    return checksum_far(GET_SEG(SS), buf, len);
}

size_t
strlen(const char *s)
{
    if (__builtin_constant_p(s))
        return __builtin_strlen(s);
    const char *p = s;
    while (*p)
        p++;
    return p-s;
}

// Compare two areas of memory.
int
memcmp(const void *s1, const void *s2, size_t n)
{
    while (n) {
        if (*(u8*)s1 != *(u8*)s2)
            return *(u8*)s1 < *(u8*)s2 ? -1 : 1;
        s1++;
        s2++;
        n--;
    }
    return 0;
}

// Compare two strings.
int
strcmp(const char *s1, const char *s2)
{
    for (;;) {
        if (*s1 != *s2)
            return *s1 < *s2 ? -1 : 1;
        if (! *s1)
            return 0;
        s1++;
        s2++;
    }
}

inline void
memset_far(u16 d_seg, void *d_far, u8 c, size_t len)
{
	d_far = MAKE_FLATPTR(d_seg, (u32)d_far);
	memset(d_far, c, len);
}

inline void
memset16_far(u16 d_seg, void *s, u16 c, size_t n)
{
    s = MAKE_FLATPTR(d_seg, (u32)s);
    while (n)
        ((u16 *)s)[--n] = c;
}

void *
memset(void *s, int c, size_t n)
{
    while (n)
        ((char *)s)[--n] = c;
    return s;
}

void memset_fl(void *ptr, u8 val, size_t size)
{
        memset(ptr, val, size);
}

inline void
memcpy_far(u16 d_seg, void *d, u16 s_seg, const void *s, size_t n)
{
    d = MAKE_FLATPTR(d_seg, (u32)d);
    s = MAKE_FLATPTR(s_seg, (u32)s);
    while (n) {
	--n;
	((char *)d)[n] = ((char *)s)[n];
    }
}

inline void
memcpy_fl(void *d_fl, const void *s_fl, size_t len)
{
        memcpy(d_fl, s_fl, len);
}

void *
#undef memcpy
memcpy(void *d1, const void *s1, size_t len)
{
  memcpy_far(0, d1, 0, s1, len);
  return d1;
}

void
iomemcpy(void *d, const void *s, u32 len)
{
   memcpy(d, s, len);
}

void *
memmove(void *d, const void *s, size_t len)
{
    if (s >= d)
        return memcpy(d, s, len);

    d += len-1;
    s += len-1;
    while (len--) {
        *(char*)d = *(char*)s;
        d--;
        s--;
    }

    return d;
}

// Copy a string - truncating it if necessary.
char *
strtcpy(char *dest, const char *src, size_t len)
{
    char *d = dest;
    while (--len && *src != '\0')
        *d++ = *src++;
    *d = '\0';
    return dest;
}

// locate first occurrence of character c in the string s
char *
strchr(const char *s, int c)
{
    for (; *s; s++)
        if (*s == c)
            return (char*)s;
    return NULL;
}

// Remove any trailing blank characters (spaces, new lines, carriage returns)
char *
nullTrailingSpace(char *buf)
{
    int len = strlen(buf);
    char *end = &buf[len-1];
    while (end >= buf && *end <= ' ')
        *(end--) = '\0';
    while (*buf && *buf <= ' ')
        buf++;
    return buf;
}
