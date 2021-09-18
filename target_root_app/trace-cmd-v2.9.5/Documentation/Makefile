# SPDX-License-Identifier: GPL-2.0

# Most likely a docbook.xsl is already installed on the users system
# instead of creating a new wheel, lets reuse what's already there.
#
FIND_MANPAGE_DOCBOOK_XSL :=  $(shell find /usr -name docbook.xsl 2>/dev/null | grep manpages | head -1)

MANPAGE_DOCBOOK_XSL ?= ${FIND_MANPAGE_DOCBOOK_XSL}

doc_dir:=$(src)/Documentation

export MANPAGE_DOCBOOK_XSL doc_dir

SUBDIR += trace-cmd
SUBDIR += libtracecmd

.PHONY: $(SUBDIR)

all: $(SUBDIR)
clean: $(SUBDIR)
install: $(SUBDIR)

$(SUBDIR):
	make -C $@ $(MAKECMDGOALS)

