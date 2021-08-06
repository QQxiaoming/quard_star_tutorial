#! /bin/sh
# Copyright (C) 2001-2018 Free Software Foundation, Inc.
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

# Test to make sure Automake supports implicit rules with dot-less
# extensions.  Also make sure that '.o' and '.obj' are handled like
# '.$(OBJEXT)'.  See also related "semantic" tests 'suffix6b.sh'
# and 'suffix6c.sh'.

. test-init.sh

cat > Makefile.am << 'END'
SUFFIXES = a b .$(OBJEXT) c .o .obj
bin_PROGRAMS = foo
foo_SOURCES = fooa barc bazc
ab:
	dummy action 1
b.$(OBJEXT):
	dummy action 2
c.o:
	dummy action C
c.obj:
	dummy action C
END

$ACLOCAL
$AUTOMAKE

sed -n -e '/foo_OBJECTS *=.*\\$/ {
  :loop
  p
  n
  t clear
  :clear
  s/\\$/\\/
  t loop
  p
  n
}' -e 's/$/ /' -e 's/^.*foo_OBJECTS *= */ /p' Makefile.in > foo-objects
cat foo-objects

# Automake must figure that fooa translates to foo.$(OBJEXT) and
# foo.$(OBJEXT) using the following rules:
#  fooa --[ab]--> foob --[b.$(OBJEXT)]--> foo.$(OBJEXT)
$FGREP ' foo.$(OBJEXT) ' foo-objects
#  barc --[c.o]--> bar.$(OBJEXT)    ## This is really meant!
$FGREP ' bar.$(OBJEXT) ' foo-objects
#  bazc --[c.obj]--> baz.$(OBJEXT)  ## This is really meant!
$FGREP ' baz.$(OBJEXT) ' foo-objects

:
