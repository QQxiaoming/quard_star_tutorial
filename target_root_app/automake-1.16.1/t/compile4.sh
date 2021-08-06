#! /bin/sh
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

# Make sure 'compile' wraps the Microsoft C/C++ compiler (cl) correctly
# with respect to absolute paths.

required='cl'
. test-init.sh

get_shell_script compile

mkdir sub

cat >sub/foo.c <<'EOF'
int foo (void)
{
  return 0;
}
EOF

cat >main.c <<'EOF'
extern int foo (void);
int main (void)
{
  return foo ();
}
EOF

cwd=$(pwd) || fatal_ "cannot get current directory"
absfoodir=$cwd/sub
absmainc=$cwd/main.c
absmainobj=$cwd/main.obj

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
END

cat > Makefile.am << 'END'
SUBDIRS = sub
END

cat > sub/Makefile.am << 'END'
lib_LIBRARIES = libfoo.a
libfoo_a_SOURCES = foo.c
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a
./configure
$MAKE

./compile cl $CPPFLAGS $CFLAGS -c -o "$absmainobj" "$absmainc"

# POSIX mandates that the compiler accepts a space between the -I,
# -l and -L options and their respective arguments.  Traditionally,
# this should work also without a space.  Try both usages.
for sp in '' ' '; do
  rm -f main
  ./compile cl $CFLAGS $LDFLAGS -L${sp}"$absfoodir" "$absmainobj" \
               -o main -l${sp}foo
  ./main
done

:
