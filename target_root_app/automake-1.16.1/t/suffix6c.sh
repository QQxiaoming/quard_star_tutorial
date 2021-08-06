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

# Test to make sure that '.o' and '.obj' are handled like '.$(OBJEXT)'.
# See also related "grepping" test 'suffix6.sh'.

. test-init.sh

cat >> configure.ac << 'END'
# $(LINK) is not defined automatically by Automake, since the *_SOURCES
# variables don't contain any known extension (.c, .cc, .f ...),
# So we need this hack -- but since such an hack can also serve as a
# mild stress test, that's ok.
OBJEXT=${OBJEXT-oOo}
AC_SUBST([LINK], ['cat >$@'])
AC_SUBST([EXEEXT], [.XxX])
AC_SUBST([OBJEXT])
AC_OUTPUT
END

unset OBJEXT

cat > Makefile.am << 'END'
SUFFIXES = .zoo .o .obj .@OBJEXT@

bin_PROGRAMS = foo
foo_SOURCES = foo.zoo

.zoo.o:
	{ echo '=.zoo.o=' && cat $<; } >$@
.zoo.obj:
	{ echo '=.zoo.obj=' && cat $<; } >$@
.zoo.@OBJEXT@:
	{ echo '=.zoo.@OBJEXT@=' && cat $<; } >$@
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

cat > foo.exp <<'END'
=.zoo.oOo=
%ONE%
END
echo %ONE% > foo.zoo
$MAKE
cat foo.oOo
cat foo.XxX
diff foo.XxX foo.exp

rm -f foo.* exp

cat > foo.exp <<'END'
=.zoo.o=
%TWO%
END
echo %TWO% > foo.zoo
run_make OBJEXT=o
cat foo.o
cat foo.XxX
diff foo.XxX foo.exp

rm -f foo.* exp

cat > foo.exp <<'END'
=.zoo.obj=
%THREE%
END
echo %THREE% > foo.zoo
run_make OBJEXT=obj
cat foo.obj
cat foo.XxX
diff foo.XxX foo.exp

rm -f foo.* exp

:
