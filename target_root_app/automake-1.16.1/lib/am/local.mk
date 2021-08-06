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

## --------------------- ##
##  Makefile fragments.  ##
## --------------------- ##

amdir = @amdir@

dist_am_DATA = \
  %D%/check.am \
  %D%/check2.am \
  %D%/clean-hdr.am \
  %D%/clean.am \
  %D%/compile.am \
  %D%/configure.am \
  %D%/data.am \
  %D%/dejagnu.am \
  %D%/depend.am \
  %D%/depend2.am \
  %D%/distdir.am \
  %D%/footer.am \
  %D%/header-vars.am \
  %D%/header.am \
  %D%/install.am \
  %D%/inst-vars.am \
  %D%/java.am \
  %D%/lang-compile.am \
  %D%/lex.am \
  %D%/library.am \
  %D%/libs.am \
  %D%/libtool.am \
  %D%/lisp.am \
  %D%/ltlib.am \
  %D%/ltlibrary.am \
  %D%/mans-vars.am \
  %D%/mans.am \
  %D%/program.am \
  %D%/progs.am \
  %D%/python.am \
  %D%/remake-hdr.am \
  %D%/scripts.am \
  %D%/subdirs.am \
  %D%/tags.am \
  %D%/texi-vers.am \
  %D%/texibuild.am \
  %D%/texinfos.am \
  %D%/vala.am \
  %D%/yacc.am

# vim: ft=automake noet
