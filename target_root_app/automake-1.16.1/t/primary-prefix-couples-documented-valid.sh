#! /bin/sh
# Copyright (C) 2011-2018 Free Software Foundation, Inc.
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

# Check that the "uncommon" prefix/primary combinations used in
# examples in the Automake manual do not cause obvious errors.
# Please keep this test in sync with the automake manual.

. test-init.sh

cat >> configure.ac <<'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
AC_PROG_LIBTOOL
AM_PROG_GCJ
AM_PATH_PYTHON
END

# Fake libtool availability.
: > ltmain.sh
cat > acinclude.m4 <<'END'
AC_DEFUN([AC_PROG_LIBTOOL],
         [AC_SUBST([LIBTOOL], [:])])
END

cat > Makefile.am <<'END'
xmldir = $(datadir)/xml
xml_DATA = file.xml

data_DATA = file1 file2 file3
data2dir = $(datadir)
data2_DATA = file4 file5 file6

aclocaldir = $(datadir)/aclocal
aclocal_DATA = mymacro.m4 myothermacro.m4

imagesdir = $(pkgdatadir)/images
soundsdir = $(pkgdatadir)/sounds
dist_images_DATA = images/vortex.pgm
dist_sounds_DATA = sounds/whirl.ogg

lisp_DATA = file1.el file2.el

javadir = $(datadir)/java
dist_java_JAVA = a.java b.java c.java

pkgpython_PYTHON = foo.py

pyexec_LTLIBRARIES = quaternion.la
quaternion_la_SOURCES = quaternion.c support.c support.h
quaternion_la_LDFLAGS = -avoid-version -module

myexecbindir = /exec
myexecbin_PROGRAMS = zardoz

foodir = $(prefix)/foo
barexecdir = $(prefix)/bar/binaries
foo_SCRIPTS = foo.sh
barexec_SCRIPTS = quux.pl

my_execbindir = $(pkglibdir)
my_doclibdir = $(docdir)
my_execbin_PROGRAMS = foo
my_doclib_LIBRARIES = libquux.a
END

$ACLOCAL

# Both these two invocations are meant.
# They exercise both code paths concerning auxiliary files.
$AUTOMAKE -a
$AUTOMAKE

:
