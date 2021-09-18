# SPDX-License-Identifier: GPL-2.0

include $(src)/scripts/utils.mk

bdir:=$(obj)/utest

TARGETS = $(bdir)/trace-utest

OBJS =
OBJS += trace-utest.o
OBJS += tracefs-utest.o

LIBS += -lcunit $(LIBTRACEEVENT_LDLAGS) $(LIBTRACEFS_LDLAGS)

OBJS := $(OBJS:%.o=$(bdir)/%.o)
DEPS := $(OBJS:$(bdir)/%.o=$(bdir)/.%.d)

$(bdir):
	@mkdir -p $(bdir)

$(OBJS): | $(bdir)
$(DEPS): | $(bdir)

$(bdir)/trace-utest: $(OBJS)
	$(Q)$(do_app_build)

$(bdir)/%.o: %.c
	$(Q)$(call do_fpic_compile)

$(DEPS): $(bdir)/.%.d: %.c
	$(Q)$(CC) -M $(CPPFLAGS) $(CFLAGS) $< > $@
	$(Q)$(CC) -M -MT $(bdir)/$*.o $(CPPFLAGS) $(CFLAGS) $< > $@

$(OBJS): $(bdir)/%.o : $(bdir)/.%.d

dep_includes := $(wildcard $(DEPS))

test: $(TARGETS)

clean:
	$(RM) $(TARGETS) $(bdir)/*.o $(bdir)/.*.d
