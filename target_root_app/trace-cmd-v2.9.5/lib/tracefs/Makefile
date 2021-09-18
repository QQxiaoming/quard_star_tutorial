# SPDX-License-Identifier: LGPL-2.1

include $(src)/scripts/utils.mk

bdir:=$(obj)/lib/tracefs

DEFAULT_TARGET = $(bdir)/libtracefs.a

LIBTRACEEVENT_CFLAGS = -I$(src)/include/traceevent -I$(src)/lib/traceevent/include
LIBTRACEFS_CFLAGS = -I$(src)/include/tracefs -I$(src)/lib/tracefs/include

CFLAGS += -I$(bdir)/include

CFLAGS := $(LIBTRACEEVENT_CFLAGS) $(LIBTRACEFS_CFLAGS) $(CFLAGS)

OBJS =
OBJS += tracefs-utils.o
OBJS += tracefs-instance.o
OBJS += tracefs-events.o

OBJS := $(OBJS:%.o=$(bdir)/%.o)
DEPS := $(OBJS:$(bdir)/%.o=$(bdir)/.%.d)

all: $(DEFAULT_TARGET)

$(bdir):
	@mkdir -p $(bdir)

$(OBJS): | $(bdir)
$(DEPS): | $(bdir)

LIBS = -L$(obj)/lib/traceevent -ltraceevent

.PHONY: warning
warning:
	@echo "********************************************"
	@echo "** NOTICE: libtracefs not found on system"
	@echo "**"
	@echo "** Building obsolete local version of libtracefs"
	@echo "** Consider installing the latest libtracefs"
	@echo "**"
	@echo "**  https://git.kernel.org/pub/scm/libs/libtrace/libtracefs.git/ "
	@echo "**"
	@echo "********************************************"

$(bdir)/libtracefs.a: $(OBJS)
	$(Q)$(call do_build_static_lib)

libtracefs: $(bdir)/libtracefs.a warning

libtracefs_nowarn: $(bdir)/libtracefs.a

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
	$(RM) $(bdir)/*.a $(bdir)/*.o $(bdir)/.*.d

.PHONY: clean
