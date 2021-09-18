# SPDX-License-Identifier: GPL-2.0

include $(src)/scripts/utils.mk

bdir:=$(obj)/lib/traceevent

DEFAULT_TARGET = $(bdir)/libtraceevent.a

LIBTRACEEVENT_CFLAGS = -I$(src)/include/traceevent -I$(src)/lib/traceevent/include

CFLAGS += -I$(bdir)/include

CFLAGS := $(LIBTRACEEVENT_CFLAGS) $(CFLAGS)

OBJS =
OBJS += event-parse.o
OBJS += event-plugin.o
OBJS += kbuffer-parse.o
OBJS += trace-seq.o
OBJS += parse-filter.o
OBJS += parse-utils.o
OBJS += event-parse-api.o
OBJS += tep_strerror.o

OBJS := $(OBJS:%.o=$(bdir)/%.o)
DEPS := $(OBJS:$(bdir)/%.o=$(bdir)/.%.d)

all: $(DEFAULT_TARGET)

$(bdir):
	@mkdir -p $(bdir)

$(OBJS): | $(bdir)
$(DEPS): | $(bdir)

.PHONY: warning
warning:
	@echo "********************************************"
	@echo "** NOTICE: libtraceevent not found on system"
	@echo "**"
	@echo "** Building obsolete local version of libtraceevent"
	@echo "** Consider installing the latest libtraceevent"
	@echo "**"
	@echo "**  https://git.kernel.org/pub/scm/libs/libtrace/libtraceevent.git/ "
	@echo "**"
	@echo "********************************************"

$(bdir)/libtraceevent.a: $(OBJS)
	$(Q)$(call do_build_static_lib)

libtraceevent: $(bdir)/libtraceevent.a warning

libtraceevent_nowarn: $(bdir)/libtraceevent.a

$(bdir)/%.o: %.c
	$(Q)$(call do_fpic_compile)

$(DEPS): $(bdir)/.%.d: %.c
	$(Q)$(CC) -M -MT $(bdir)/$*.o $(CPPFLAGS) $(CFLAGS) $< > $@

$(OBJS): $(bdir)/%.o : $(bdir)/.%.d

dep_includes := $(wildcard $(DEPS))

ifneq ($(dep_includes),)
  include $(dep_includes)
endif

clean:
	$(RM) -f $(bdir)/*.a $(bdir)/*.o $(bdir)/.*.d

.PHONY: clean
