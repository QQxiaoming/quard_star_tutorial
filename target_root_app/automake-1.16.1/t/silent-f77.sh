#!/bin/sh
# Copyright (C) 2010-2018 Free Software Foundation, Inc.
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

# Check silent-rules mode for Fortran 77.
# Keep this ins sync with the sister test 'silent-f90.sh'.

required=fortran77
. test-init.sh

mkdir sub

cat >>configure.ac <<'EOF'
AC_PROG_F77
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
EOF

cat > Makefile.am <<'EOF'
# Need generic and non-generic rules.
bin_PROGRAMS = foo1 foo2
foo1_SOURCES = foo.f
foo2_SOURCES = $(foo1_SOURCES)
foo2_FFLAGS = $(AM_FFLAGS)
SUBDIRS = sub
EOF

cat > sub/Makefile.am <<'EOF'
AUTOMAKE_OPTIONS = subdir-objects
# Need generic and non-generic rules.
bin_PROGRAMS = bar1 bar2
bar1_SOURCES = bar.f
bar2_SOURCES = $(bar1_SOURCES)
bar2_FFLAGS = $(AM_FFLAGS)
EOF

cat > foo.f <<'EOF'
      program foo
      stop
      end
EOF
cp foo.f sub/bar.f

$ACLOCAL
$AUTOMAKE --add-missing
$AUTOCONF

./configure --enable-silent-rules
run_make -O
# Avoid spurious failures with SunStudio Fortran compilers.
sed '/^NOTICE:/d' stdout > t
mv -f t stdout
cat stdout

$EGREP ' (-c|-o)' stdout && exit 1
grep 'mv ' stdout && exit 1

grep 'F77 .*foo\.'  stdout
grep 'F77 .*bar\.'  stdout
grep 'F77LD .*foo1' stdout
grep 'F77LD .*bar1' stdout
grep 'F77LD .*foo2' stdout
grep 'F77LD .*bar2' stdout

$EGREP '(FC|FCLD) ' stdout && exit 1

# Ensure a clean rebuild.
$MAKE clean

run_make -O V=1

grep ' -c ' stdout
grep ' -o ' stdout

$EGREP '(F77|FC|LD) ' stdout && exit 1

:
