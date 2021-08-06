# Maintainer makefile for Automake.  Requires GNU make.

# Copyright (C) 2012-2018 Free Software Foundation, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

ifeq ($(filter bootstrap,$(MAKECMDGOALS)),)

ifeq ($(wildcard Makefile),)
  # Any target but 'bootstrap' specified in an unconfigured tree
  # is an error, even when the user is running GNU make.
  $(warning There seems to be no Makefile in this directory.)
  $(warning You must run ./configure before running 'make'.)
  $(error Fatal Error)
endif
include ./Makefile
include $(srcdir)/maintainer/maint.mk
include $(srcdir)/maintainer/syntax-checks.mk

else # ! bootstrap in $(MAKECMDGOALS)

other-targets := $(filter-out bootstrap,$(MAKECMDGOALS))
config-status := $(wildcard ./config.status)

BOOTSTRAP_SHELL ?= /bin/sh
export BOOTSTRAP_SHELL

# Allow the user (or more likely the developer) to ask for a bootstrap
# of the package.
#
# Two issues that must be kept in mind in the implementation below:
#
#  [1] "make bootstrap" can be invoked before 'configure' is run (and in
#      fact, even before it is created, if we are bootstrapping from a
#      freshly-cloned checkout).
#
#  [2] When re-bootstrapping an already configured tree, we must ensure
#      that the automatic remake rules for Makefile and company do not
#      kick in, because the tree might be in an inconsistent state (e.g.,
#      we have just switched from 'maint' to 'master', and have the built
#      'automake' script left from 'maint', but the files 'lib/am/*.am'
#      are from 'master': if 'automake' gets run and uses those files --
#      boom!).

ifdef config-status # Bootstrap from an already-configured tree.
  # We need the definition of $(srcdir) in the 'bootstrap' rule
  # below.
  srcdir := $(shell echo @srcdir@ | $(config-status) --file=-)
  ifndef srcdir
    $(error Could not obtain $$(srcdir) from $(config-status))
  endif
  # Also, if we are re-bootstrapping an already-configured tree, we
  # want to re-configure it with the same pre-existing configuration.
  old-configure-flags := $(shell $(config-status) --config)
else # Assume we are bootstrapping from an unconfigured srcdir.
  srcdir := .
  old-configure-flags :=
endif

configure-flags := $(old-configure-flags) $(BOOTSTRAP_CONFIGURE_FLAGS)

.PHONY: bootstrap
bootstrap:
	cd $(srcdir) && $(SHELL) ./bootstrap
	$(srcdir)/configure $(configure-flags)
	$(MAKE) clean
	$(MAKE) check TESTS=t/get-sysconf

# Ensure that all the specified targets but 'bootstrap' (if any) are
# run with a properly re-bootstrapped tree.
ifdef other-targets
$(other-targets): restart
.PHONY: $(other-targets) restart
restart: bootstrap; $(MAKE) $(AM_MAKEFLAGS) $(other-targets)
endif

endif # ! bootstrap in $(MAKECMDGOALS)
