include $(src)/scripts/utils.mk

bdir:=$(obj)/lib/trace-cmd/plugins

PLUGIN_OBJS =

PLUGIN_OBJS := $(PLUGIN_OBJS:%.o=$(bdir)/%.o)
PLUGIN_BUILD := $(PLUGIN_OBJS:$(bdir)/%.o=$(bdir)/%.so)

PLUGINS := $(PLUGIN_BUILD)

DEPS := $(PLUGIN_OBJS:$(bdir)/%.o=$(bdir)/.%.d)

all: $(PLUGINS)

$(bdir):
	@mkdir -p $(bdir)

$(PLUGIN_OBJS): | $(bdir)
$(DEPS): | $(bdir)

$(PLUGIN_OBJS): $(bdir)/%.o : %.c
	$(Q)$(do_compile_plugin_obj)

$(PLUGIN_BUILD): $(bdir)/%.so: $(bdir)/%.o
	$(Q)$(do_plugin_build)

$(DEPS): $(bdir)/.%.d: %.c
	$(Q)$(CC) -M -MT $(bdir)/$*.o $(CPPFLAGS) $(CFLAGS) $< > $@

$(PLUGIN_OBJS): $(bdir)/%.o : $(bdir)/.%.d

PLUGINS_INSTALL = $(subst .so,.install,$(PLUGINS))

$(PLUGINS_INSTALL): $(bdir)/%.install : $(bdir)/%.so force
	$(Q)$(call do_install_data,$<,$(plugin_tracecmd_dir_SQ))

install_plugins: $(PLUGINS_INSTALL)

# The following targets are necessary to trigger a rebuild when
# $(PLUGIN_DIR_TRACECMD) change. Without them, a full clean build would
# necessary in order to get the binaries updated.

$(bdir)/tracecmd_plugin_dir: $(bdir) force
	$(Q)$(N)$(call update_dir, 'PLUGIN_DIR_TRACECMD=$(PLUGIN_DIR_TRACECMD)')

dep_includes := $(wildcard $(DEPS))

ifneq ($(dep_includes),)
  include $(dep_includes)
endif

clean:
	$(RM) -f $(bdir)/*.a $(bdir)/*.so $(bdir)/*.o $(bdir)/.*.d\
		$(bdir)/tracecmd_plugin_dir

force:
.PHONY: clean force
