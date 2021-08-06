## -*- makefile-automake -*-
## Copyright (C) 1995-2018 Free Software Foundation, Inc.
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <https://www.gnu.org/licenses/>.

## ---------------------------------------------------- ##
##  Private perl modules used by automake and aclocal.  ##
## ---------------------------------------------------- ##

perllibdir = $(pkgvdatadir)/Automake

dist_perllib_DATA = \
  %D%/ChannelDefs.pm \
  %D%/Channels.pm \
  %D%/Condition.pm \
  %D%/Configure_ac.pm \
  %D%/DisjConditions.pm \
  %D%/FileUtils.pm \
  %D%/General.pm \
  %D%/Getopt.pm \
  %D%/Item.pm \
  %D%/ItemDef.pm \
  %D%/Language.pm \
  %D%/Location.pm \
  %D%/Options.pm \
  %D%/Rule.pm \
  %D%/RuleDef.pm \
  %D%/Variable.pm \
  %D%/VarDef.pm \
  %D%/Version.pm \
  %D%/XFile.pm \
  %D%/Wrap.pm

nodist_perllib_DATA = %D%/Config.pm
CLEANFILES += $(nodist_perllib_DATA)

%D%/Config.pm: %D%/Config.in Makefile
	$(AM_V_at)rm -f $@ $@-t
	$(AM_V_at)$(MKDIR_P) %D%
	$(AM_V_GEN)in=Config.in \
	  && $(do_subst) <$(srcdir)/%D%/Config.in >$@-t
	$(generated_file_finalize)
EXTRA_DIST += %D%/Config.in

# vim: ft=automake noet
