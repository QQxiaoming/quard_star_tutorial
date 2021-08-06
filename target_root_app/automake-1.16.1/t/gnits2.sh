#! /bin/sh
# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

# Test to ensure std-options checking is correct.

required='cc native'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = gnits
noinst_PROGRAMS = fubar2
bin_PROGRAMS = fubar sub/fine
nobase_bin_PROGRAMS = sub/fubar3
fubar_SOURCES = fubar.c
fubar2_SOURCES = fubar.c
sub_fubar3_SOURCES = fubar.c
sub_fine_SOURCES = fine.c
bin_SCRIPTS = sub/scriptok.sh sub/scriptnok.sh

grep-stderr:
	grep 'pfubar$(EXEEXT) does not support' stderr
	grep 'pfubar3$(EXEEXT) does not support' stderr
	grep 'pscriptnok\.sh does not support' stderr
## Only three failures please.
	test `grep 'does not support --help' stderr | wc -l` = 3
	test `grep 'does not support --version' stderr | wc -l` = 3

test-install: install
	test -f ../inst-dir/bin/pfine$(EXEEXT)
	test ! -f ../inst-dir/bin/fine$(EXEEXT)
END

cat > fubar.c <<'END'
int main (void)
{
  return 0;
}
END

cat > fine.c << 'END'
#include <stdio.h>
int main (void)
{
  puts ("Which version? Which usage?");
  return 0;
}
END

mkdir sub

cat >sub/scriptok.sh <<EOF
#!/bin/sh
echo "Which version? Which usage?"
EOF

# Not only does this script not support --help/--version, but
# it will also hang when run without input.
cat >sub/scriptnok.sh <<EOF
#!/bin/sh
cat
EOF

chmod +x sub/scriptok.sh
chmod +x sub/scriptnok.sh

# Files required by Gnits.
: > INSTALL
: > NEWS
: > README
: > COPYING
: > AUTHORS
: > ChangeLog
: > THANKS

# The following file should not be distributed.
# (alpha.sh checks the case where it must be distributed.)
: > README-alpha

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

grep README-alpha Makefile.in && exit 1

mkdir build
cd build

# Use --program-prefix to make sure the std-options check honors it.
../configure "--prefix=$(pwd)/../inst-dir" --program-prefix=p
$MAKE all
$MAKE test-install
# Don't trust th exit status of "make -k" for non-GNU makes.
if using_gmake; then status=FAIL; else status=IGNORE; fi
run_make -e $status -E -- -k installcheck
$MAKE grep-stderr

:
