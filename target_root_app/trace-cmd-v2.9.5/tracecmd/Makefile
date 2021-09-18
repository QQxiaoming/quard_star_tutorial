# SPDX-License-Identifier: GPL-2.0

VERSION := $(TC_VERSION)
PATCHLEVEL := $(TC_PATCHLEVEL)
EXTRAVERSION := $(TC_EXTRAVERSION)

bdir:=$(obj)/tracecmd

TC_VERSION := $(bdir)/include/tc_version.h
TARGETS = $(bdir)/trace-cmd $(TC_VERSION)

BUILDGUI := 0
include $(src)/scripts/utils.mk

CFLAGS += -I$(bdir)/include

TRACE_CMD_OBJS =
TRACE_CMD_OBJS += trace-cmd.o
TRACE_CMD_OBJS += trace-record.o
TRACE_CMD_OBJS += trace-read.o
TRACE_CMD_OBJS += trace-split.o
TRACE_CMD_OBJS += trace-listen.o
TRACE_CMD_OBJS += trace-stack.o
TRACE_CMD_OBJS += trace-hist.o
TRACE_CMD_OBJS += trace-mem.o
TRACE_CMD_OBJS += trace-snapshot.o
TRACE_CMD_OBJS += trace-stat.o
TRACE_CMD_OBJS += trace-profile.o
TRACE_CMD_OBJS += trace-stream.o
TRACE_CMD_OBJS += trace-record.o
TRACE_CMD_OBJS += trace-restore.o
TRACE_CMD_OBJS += trace-check-events.o
TRACE_CMD_OBJS += trace-show.o
TRACE_CMD_OBJS += trace-list.o
TRACE_CMD_OBJS += trace-usage.o
TRACE_CMD_OBJS += trace-dump.o
TRACE_CMD_OBJS += trace-clear.o
TRACE_CMD_OBJS += trace-vm.o

ifeq ($(VSOCK_DEFINED), 1)
TRACE_CMD_OBJS += trace-agent.o
TRACE_CMD_OBJS += trace-setup-guest.o
endif

ALL_OBJS := $(TRACE_CMD_OBJS:%.o=$(bdir)/%.o)

all_objs := $(sort $(ALL_OBJS))
all_deps := $(all_objs:$(bdir)/%.o=$(bdir)/.%.d)

CONFIG_INCLUDES =
CONFIG_LIBS	= -lrt -lpthread $(TRACE_LIBS)
CONFIG_FLAGS	=

all: $(TARGETS)

$(bdir):
	@mkdir -p $(bdir)

$(bdir)/include: | $(bdir)
	@mkdir -p $(bdir)/include

$(TC_VERSION): force | $(bdir)/include
	$(Q)$(call update_version.h)

$(all_deps): | $(bdir)
$(all_objs): | $(bdir)

$(bdir)/trace-cmd: $(ALL_OBJS)
	$(Q)$(do_app_build)

$(bdir)/trace-cmd: $(LIBTRACECMD_STATIC)

$(bdir)/%.o: %.c
	$(Q)$(call do_compile)

$(all_deps): $(bdir)/.%.d: %.c
	$(Q)$(CC) -M -MT $(bdir)/$*.o $(CPPFLAGS) $(CFLAGS) $< > $@

$(all_deps): $(TC_VERSION)

$(all_objs): $(bdir)/%.o : $(bdir)/.%.d

dep_includes := $(wildcard $(DEPS))

ifneq ($(dep_includes),)
  include $(dep_includes)
endif

clean:
	$(RM) $(bdir)/*.a $(bdir)/*.so $(bdir)/*.o $(bdir)/.*.d $(TARGETS)

force:
.PHONY: clean
