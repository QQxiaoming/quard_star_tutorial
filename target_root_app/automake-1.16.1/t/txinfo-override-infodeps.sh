#! /bin/sh
# Copyright (C) 2004-2018 Free Software Foundation, Inc.
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

# Make sure that INFO_DEPS can be overridden.
# Report from Bruce Korb.

. test-init.sh

cat > Makefile.am << 'END'
INFO_DEPS = foo.info
info_TEXINFOS = bar.texi
END

echo '@setfilename baz.info' > bar.texi

$ACLOCAL
AUTOMAKE_fails --add-missing
grep '^Makefile\.am:1:.*user variable.*INFO_DEPS' stderr
grep 'overrides Automake variable.*INFO_DEPS' stderr

$AUTOMAKE -Wno-override

# There is only one definition of INFO_DEPS ...
test 1 -eq $(grep -c '^INFO_DEPS.*=' Makefile.in)
# ... and it is the right one.
grep '^INFO_DEPS *= *foo.info *$' Makefile.in

# Likewise with AC_SUBST.

cat > Makefile.am << 'END'
info_TEXINFOS = bar.texi
END
cat >> configure.ac << 'END'
AC_SUBST([INFO_DEPS])
END
rm -rf autom4te.cache # Make sure autoconf sees the configure.ac update.
AUTOMAKE_fails

grep '^configure\.ac:4:.*user variable.*INFO_DEPS' stderr
grep 'overrides Automake variable.*INFO_DEPS' stderr

$AUTOMAKE -Wno-override

# There is only one definition of INFO_DEPS ...
test 1 -eq $(grep -c '^INFO_DEPS.*=' Makefile.in)
# ... and it is the right one.
grep '^INFO_DEPS *= *@INFO_DEPS@ *$' Makefile.in

:
