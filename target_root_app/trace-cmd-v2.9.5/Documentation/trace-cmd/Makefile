# SPDX-License-Identifier: GPL-2.0

# Include the utils
include $(src)/scripts/utils.mk

bdir:=$(doc_dir)/trace-cmd

$(bdir)/%.xsl: $(bdir)/%.txt
	$(Q)mkdir -p $(bdir)
	$(Q)$(do_asciidoc_build)


$(bdir)/%.1: $(bdir)/%.1.xsl
	@$(call manpage.xsl)
	$(Q)$(do_xsltproc_build)

$(bdir)/%.5: $(bdir)/%.5.xsl
	@$(call manpage.xsl)
	$(Q)$(do_xsltproc_build)
	$(Q)$(call remove_synopsis, $@)

 TEXT1 = $(wildcard $(bdir)/*.1.txt)
 MAN1 = $(patsubst $(bdir)/%.1.txt,$(bdir)/%.1, ${TEXT1})
 TEXT5 = $(wildcard $(bdir)/*.5.txt)
 MAN5 = $(patsubst $(bdir)/%.5.txt,$(bdir)/%.5, ${TEXT5})

all: $(MAN1) $(MAN5)

MAN1_INSTALL = $(MAN1:%.1=%.1.install)
MAN5_INSTALL = $(MAN5:%.5=%.5.install)

$(MAN1_INSTALL): %.1.install : %.1 force
	$(Q)$(call do_install_docs,$<,$(man_dir_SQ)/man1)
$(MAN5_INSTALL): %.5.install : %.5 force
	$(Q)$(call do_install_docs,$<,$(man_dir_SQ)/man5)

install: $(MAN1_INSTALL) $(MAN5_INSTALL)

clean:
	$(Q)(cd $(bdir); \
		$(RM) *.xml *.xsl *.1 *.5)

.PHONE: force
force:
