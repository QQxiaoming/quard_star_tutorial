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

# Check silent-rules mode for C++, both with and without automatic
# dependency tracking.

required=c++
. test-init.sh

mkdir sub

cat >>configure.ac <<'EOF'
AC_PROG_CXX
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
EOF

cat > Makefile.am <<'EOF'
# Need generic and non-generic rules.
bin_PROGRAMS = foo1 foo2
foo1_SOURCES = foo.cpp baz.cxx quux.cc
foo2_SOURCES = $(foo1_SOURCES)
foo2_CXXFLAGS = $(AM_CXXFLAGS)
SUBDIRS = sub
EOF

cat > sub/Makefile.am <<'EOF'
AUTOMAKE_OPTIONS = subdir-objects
# Need generic and non-generic rules.
bin_PROGRAMS = bar1 bar2
bar1_SOURCES = bar.cpp
bar2_SOURCES = $(bar1_SOURCES)
bar2_CXXFLAGS = $(AM_CXXFLAGS)
EOF

cat > foo.cpp <<'EOF'
using namespace std; /* C compilers fail on this. */
int main (void) { return 0; }
EOF

# Let's try out other extensions too.
echo 'class Baz  { public: int i;  };' > baz.cxx
echo 'class Quux { public: bool b; };' > quux.cc

cp foo.cpp sub/bar.cpp

$ACLOCAL
$AUTOMAKE --add-missing
$AUTOCONF

# Sanity check: make sure the cache variable we force is really used
# by configure.
$FGREP am_cv_CXX_dependencies_compiler_type configure

# Force dependency tracking explicitly, so that slow dependency
# extractors are not rejected.  Try also with dependency tracking
# explicitly disabled.
for config_args in \
  --enable-dependency-tracking --disable-dependency-tracking
do

  ./configure $config_args --enable-silent-rules

  run_make -O

  $EGREP ' (-c|-o)' stdout && exit 1
  grep 'mv ' stdout && exit 1

  grep 'CXX .*foo\.'  stdout
  grep 'CXX .*baz\.'  stdout
  grep 'CXX .*quux\.' stdout
  grep 'CXX .*bar\.'  stdout
  grep 'CXXLD .*foo1' stdout
  grep 'CXXLD .*bar1' stdout
  grep 'CXXLD .*foo2' stdout
  grep 'CXXLD .*bar2' stdout

  # Ensure a clean rebuild.
  $MAKE clean

  run_make -O V=1

  grep ' -c ' stdout
  grep ' -o ' stdout

  $EGREP '(CXX|LD) ' stdout && exit 1

  # Ensure a clean reconfiguration/rebuild.
  $MAKE clean
  $MAKE maintainer-clean

done

:
