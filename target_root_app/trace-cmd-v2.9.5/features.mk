# SPDX-License-Identifier: GPL-2.0

# taken from perf which was based on Linux Kbuild
# try-cc
# Usage: option = $(call try-cc, source-to-build, cc-options)
try-cc = $(shell sh -c							\
	'TMP="$(BUILD_OUTPUT)$(TMPOUT).$$$$";						\
	echo "$(1)" |							\
	$(CC) -x c - $(2) -o "$$TMP" > /dev/null 2>&1 && echo y;	\
	rm -f "$$TMP"')

define SOURCE_PTRACE
#include <stdio.h>
#include <sys/ptrace.h>

int main (void)
{
	int ret;
	ret = ptrace(PTRACE_ATTACH, 0, NULL, 0);
	ptrace(PTRACE_TRACEME, 0, NULL, 0);
	ptrace(PTRACE_GETSIGINFO, 0, NULL, NULL);
	ptrace(PTRACE_GETEVENTMSG, 0, NULL, NULL);
	ptrace(PTRACE_SETOPTIONS, NULL, NULL,
		       PTRACE_O_TRACEFORK |
		       PTRACE_O_TRACEVFORK |
		       PTRACE_O_TRACECLONE |
		       PTRACE_O_TRACEEXIT);
	ptrace(PTRACE_CONT, NULL, NULL, 0);
	ptrace(PTRACE_DETACH, 0, NULL, NULL);
	ptrace(PTRACE_SETOPTIONS, 0, NULL,
	       PTRACE_O_TRACEFORK |
	       PTRACE_O_TRACEVFORK |
	       PTRACE_O_TRACECLONE |
	       PTRACE_O_TRACEEXIT);
	return ret;
}
endef

define SOURCE_AUDIT
#include <stdio.h>
#include <libaudit.h>

int main (void)
{
	char *name;
	int ret;
	ret = audit_detect_machine();
	if (ret < 0)
		return ret;
	name = audit_syscall_to_name(1, ret);
	if (!name)
		return -1;
	return ret;
}
endef
