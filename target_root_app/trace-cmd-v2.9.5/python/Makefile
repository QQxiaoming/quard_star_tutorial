# SPDX-License-Identifier: GPL-2.0

include $(src)/scripts/utils.mk

ifdef BUILD_PYTHON_WORKS
PYTHON_SO_INSTALL := ctracecmd.install
PYTHON_PY_PROGS := event-viewer.install
PYTHON_PY_LIBS := tracecmd.install
endif

ctracecmd.so: ctracecmd.i $(LIBTRACECMD_STATIC)
	swig -Wall -python -noproxy -I$(src)/include/traceevent -I$(src)/include/trace-cmd ctracecmd.i
	$(CC) -fpic -c $(CPPFLAGS) $(CFLAGS) $(PYTHON_INCLUDES)  ctracecmd_wrap.c
	$(CC) --shared $(LIBTRACECMD_STATIC) $(LDFLAGS) ctracecmd_wrap.o -o ctracecmd.so $(TRACE_LIBS)

$(PYTHON_SO_INSTALL): %.install : %.so force
	$(Q)$(call do_install_data,$<,$(python_dir_SQ))

$(PYTHON_PY_PROGS): %.install : %.py force
	$(Q)$(call do_install,$<,$(python_dir_SQ))

$(PYTHON_PY_LIBS): %.install : %.py force
	$(Q)$(call do_install_data,$<,$(python_dir_SQ))

install_python: $(PYTHON_SO_INSTALL) $(PYTHON_PY_PROGS) $(PYTHON_PY_LIBS)


clean:
	$(RM) *.a *.so *.o .*.d ctracecmd_wrap.*

force:
.PHONY: clean force
