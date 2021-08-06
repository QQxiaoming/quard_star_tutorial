#! /bin/sh
# Copyright (C) 1998-2018 Free Software Foundation, Inc.
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

# Removing subdir objects does not cause too much 'rm' invocations.
# Also, if we rename a source file in a subdirectory, the stale
# compiled object corresponding to the old name still gets removed by
# "make mostlyclean".  See automake bug#10697.
# This is the non-libtool case.  Keep this test in sync with sister test
# 'subobj-clean-lt-pr10697.sh', which deals with the libtool case.

required=cc
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_CONFIG_FILES([get-objext.sh:get-objext.in])
AC_OUTPUT
END

echo "OBJEXT='@OBJEXT@'" > get-objext.in

oPATH=$PATH
ocwd=$(pwd) || fatal_ "getting current working directory"

# An rm(1) wrapper that fails when invoked too many times.
mkdir rm-wrap
max_rm_invocations=3
count_file=$ocwd/rm-wrap/count
cat > rm-wrap/rm <<END
#!$AM_TEST_RUNNER_SHELL -e
count=\$((\$(cat '$count_file') + 1))
test \$count -le $max_rm_invocations || {
  echo "rm invoked more than $max_rm_invocations times" >&2
  exit 1
}
echo "\$count" > '$count_file'
PATH='$oPATH'; export PATH
exec rm "\$@"
END
chmod a+x rm-wrap/rm
echo "0" > rm-wrap/count

cat > Makefile.am <<'END'
.PHONY: sanity-check-rm
sanity-check-rm:
	rm -f 1
	rm -f 2
	rm -f 3
	rm -f x && exit 1; :
	echo "0" > rm-wrap/count

AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = foo
foo_SOURCES = \
  sub1/a.c \
  sub1/b.c \
  sub1/c.c \
  sub1/d.c \
  sub1/e.c \
  sub1/f.c \
  sub2/a.c \
  sub2/b.c \
  sub2/c.c \
  sub2/d.c \
  sub2/e.c \
  sub2/f.c \
  main.c
END

mkdir sub1 sub2
echo 'int main (void)' > main.c
echo '{' >> main.c
for i in 1 2; do
  for j in a b c d e f; do
    echo "void $j$i (void) { }" > sub$i/$j.c
    echo "  $j$i ();" >> main.c
  done
done
echo '  return 0;' >> main.c
echo '}' >> main.c
cat main.c # For debugging.

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

test -f get-objext.sh
. ./get-objext.sh

$MAKE

# This must go after configure, since that will invoke rm many times.
PATH=$ocwd/rm-wrap$PATH_SEPARATOR$PATH; export PATH
$MAKE sanity-check-rm || fatal_ "rm wrapper doesn't work as expected"

$MAKE mostlyclean
ls -l . sub1 sub2
for i in 1 2; do
  for j in a b c d e f; do
    test ! -e sub$i/$j.o
    test ! -e sub$i/$j.obj
    test -f sub$i/$j.c || exit 99 # Sanity check
  done
done

PATH=$oPATH; export PATH
rm -rf rm-wrap

$MAKE clean
$MAKE
test -f sub1/a.$OBJEXT
test -f sub2/d.$OBJEXT

$sleep

mv -f sub2/d.c sub2/x.c
rm -f sub1/a.c

sed -e '/ a1 ()/d' main.c > t
mv -f t main.c

sed -e '/sub1\/a\.c/d' -e 's|sub2/d\.c|sub2/x.c|' Makefile.am > t
mv -f t Makefile.am

using_gmake || $MAKE Makefile
$MAKE
test -f sub2/x.$OBJEXT

# The stale objects are still there after a mere "make all" ...
test -f sub1/a.$OBJEXT
test -f sub2/a.$OBJEXT

# ... but they get removed by "make mostlyclean" ...
$MAKE mostlyclean
test ! -e sub1/a.$OBJEXT
test ! -e sub2/d.$OBJEXT

# ... and do not get rebuilt ...
$MAKE clean
$MAKE all
test ! -e sub1/a.$OBJEXT
test ! -e sub2/d.$OBJEXT

# ... while the non-stale files do.
test -f sub1/b.$OBJEXT
test -f sub2/x.$OBJEXT

:
