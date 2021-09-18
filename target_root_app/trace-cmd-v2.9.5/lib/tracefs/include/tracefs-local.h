/* SPDX-License-Identifier: LGPL-2.1 */
/*
 * Copyright (C) 2019, VMware, Tzvetomir Stoyanov <tz.stoyanov@gmail.com>
 *
 */
#ifndef _TRACE_FS_LOCAL_H
#define _TRACE_FS_LOCAL_H

#define __hidden __attribute__((visibility ("hidden")))

/* Can be overridden */
void warning(const char *fmt, ...);
int str_read_file(const char *file, char **buffer);
char *trace_append_file(const char *dir, const char *name);

char *trace_find_tracing_dir(void);

#ifndef ACCESSPERMS
#define ACCESSPERMS (S_IRWXU|S_IRWXG|S_IRWXO) /* 0777 */
#endif

#ifndef ALLPERMS
#define ALLPERMS (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO) /* 07777 */
#endif

#ifndef DEFFILEMODE
#define DEFFILEMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH) /* 0666*/
#endif

#endif /* _TRACE_FS_LOCAL_H */
