#!/bin/sh
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

# Check silent-rules mode, on 'make' implementations that do not
# support nested variables (Bug#9928, Bug#10237).

. test-init.sh

cat >>configure.ac <<'EOF'
AM_SILENT_RULES
AC_PROG_CC
AC_OUTPUT
EOF

cat > Makefile.am <<'EOF'
# Need generic and non-generic rules.
bin_PROGRAMS = foo bar
bar_CFLAGS = $(AM_CFLAGS)

# Check that AM_V and AM_DEFAULT_V work as advertised.
pkg_verbose = $(pkg_verbose_@AM_V@)
pkg_verbose_ = $(pkg_verbose_@AM_DEFAULT_V@)
pkg_verbose_0 = @echo PKG-GEN $@;

bin_SCRIPTS = oop
oop:
	$(pkg_verbose)echo $@ >$@

mostlyclean-local:
	rm -f oop
EOF

cat > foo.c <<'EOF'
int main ()
{
  return 0;
}
EOF
cp foo.c bar.c

cat >mymake <<'EOF'
#! /bin/sh
makerules=

case $1 in
  -f)
    makefile=$2
    case $2 in
      -) makerules=`cat` || exit ;;
    esac ;;
  *)
    for makefile in makefile Makefile; do
      test -f $makefile && break
    done ;;
esac

nested_var_pat='^[^#].*\$([^)]*\$'
if
  case $makefile in
    -) printf '%s\n' "$makerules" | grep "$nested_var_pat";;
    *) grep "$nested_var_pat" $makefile;;
  esac
then
  echo >&2 "mymake: $makefile contains nested variables"
  exit 1
fi

case $makefile in
  -) printf '%s\n' "$makerules" | $mymake_MAKE "$@";;
  *) exec $mymake_MAKE "$@";;
esac
EOF
chmod a+x mymake
mymake_MAKE=${MAKE-make}
MAKE=./mymake
export MAKE mymake_MAKE

# As a sanity check, verify that 'mymake' rejects Makefiles that
# use nested variables.
cat > Makefile <<'END'
a = $(b$(c))
all:
	touch bar
END
$MAKE && exit 99
mv -f Makefile foo.mk
$MAKE -f foo.mk && exit 99
cat foo.mk | $MAKE -f - && exit 99
test -f bar && exit 99
sed '/a =/d' foo.mk > Makefile
$MAKE && test -f bar || exit 99
rm -f bar Makefile foo.mk

$ACLOCAL
$AUTOMAKE --add-missing
$AUTOCONF

./configure --enable-silent-rules >stdout || { cat stdout; exit 1; }
cat stdout
grep '^checking whether \./mymake supports nested variables\.\.\. no *$' \
  stdout
$EGREP 'CC|AM_V|GEN' Makefile # For debugging.
grep '^AM_V_CC =  *\$(am__v_CC_0) *$' Makefile
grep '^AM_V_GEN =  *\$(am__v_GEN_0) *$' Makefile
run_make -O
$EGREP ' (-c|-o)' stdout && exit 1
grep 'mv ' stdout && exit 1
grep 'echo .*oop' stdout && exit 1
grep 'CC .*foo\.' stdout
grep 'CC .*bar\.' stdout
grep 'CCLD .*foo' stdout
grep 'CCLD .*bar' stdout
grep 'PKG-GEN .*oop' stdout
$MAKE distclean

./configure --disable-silent-rules > stdout || { cat stdout; exit 1; }
cat stdout
grep '^checking whether \./mymake supports nested variables\.\.\. no *$' \
  stdout
$EGREP 'CC|AM_V|GEN' Makefile # For debugging.
grep '^AM_V_CC =  *\$(am__v_CC_1) *$' Makefile
grep '^AM_V_GEN =  *\$(am__v_GEN_1) *$' Makefile

run_make -O
grep ' -c' stdout
grep ' -o foo' stdout
grep ' -o bar' stdout
grep 'echo .*>oop' stdout
$EGREP '(CC|LD) ' stdout && exit 1

:
