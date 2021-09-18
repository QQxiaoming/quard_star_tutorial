/* SPDX-License-Identifier: LGPL-2.1 */
#ifndef __TRACE_CMD_BUG
#define __TRACE_CMD_BUG

#define unlikely(cond)	__builtin_expect(!!(cond), 0)

#define WARN_ONCE(cond, fmt, ...)			\
	({						\
		int __c__ = cond;			\
		if (unlikely(__c__)) {			\
			warning(fmt, ##__VA_ARGS__);	\
		}					\
		__c__;					\
	})
#endif /* __TRACE_CMD_BUG */
