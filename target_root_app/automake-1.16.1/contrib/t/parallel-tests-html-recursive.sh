#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# Recursive use of 'check-html'.  See Automake bug#11287.

. test-init.sh

# Try the variants that are tried in check-html.am.
while :; do
  for r2h in $RST2HTML rst2html rst2html.py; do
    echo "$me: running $r2h --version"
    $r2h --version && break 2
    : For shells with busted 'set -e'.
  done
  skip_all_ "no proper rst2html program found"
done
unset r2h

cp "$am_top_srcdir"/contrib/check-html.am . \
  || fatal_ "cannot fetch 'check-html.am' from contrib"

cat >> configure.ac << 'END'
AM_EXTRA_RECURSIVE_TARGETS([check-html])
AC_CONFIG_FILES([sub/Makefile sub/more/Makefile])
AC_OUTPUT
END

cat > Makefile.am << 'END'
SUBDIRS = sub
EXTRA_DIST = $(TESTS)
TEST_SUITE_LOG = mylog.log
TESTS = foo.test bar.sh mu
XFAIL_TESTS = bar.sh
check_SCRIPTS = bla
bla:
	echo '#!/bin/sh' > $@-t
	echo 'echo Blah Blah Blah' >> $@-t
	chmod a+x,a-w $@-t
	mv -f $@-t $@
CLEANFILES = bla
include $(srcdir)/check-html.am
END

mkdir sub
echo SUBDIRS = more > sub/Makefile.am

mkdir sub/more
cat > sub/more/Makefile.am << 'END'
include $(top_srcdir)/check-html.am
TEST_EXTENSIONS = .test .sh
TESTS = sh.sh test.test
LOG_COMPILER = true
test.log: sh.log
nodist_check_DATA = x.txt
$(nodist_check_DATA):
	echo $@ > $@
CLEANFILES = $(nodist_check_DATA)
EXTRA_DIST = $(TESTS)
END

cat > foo.test <<'END'
#! /bin/sh
./bla
exit 77
END

cat > bar.sh <<'END'
#! /bin/sh
echo "this is $0"
exit 1
END

cat > mu <<'END'
#! /bin/sh
set -x
test -f sub/more/test.log
test -f sub/more/sh.log
END

cat > sub/more/test.test << 'END'
#!/bin/sh
echo "this is $0"
set -x
test -f sh.log
test -f x.txt
exit 77
END

cat > sub/more/sh.sh << 'END'
#!/bin/sh
set -x
test ! -f test.log
test -f x.txt
END


cat > sub/more/mu << 'END'
#!/bin/sh
exit 99
END

chmod a+x foo.test bar.sh mu sub/more/test.test sub/more/sh.sh

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

$MAKE check-html
grep 'Blah Blah Blah' mylog.html
grep 'this is .*bar\.sh' mylog.html
grep 'this is .*test\.test' sub/more/test-suite.html
# check-html should cause check_SCRIPTS and check_DATA to be created.
test -f bla
test -f sub/more/x.txt

# "make clean" should remove HTML files.
$MAKE clean
test ! -e mylog.html
test ! -e sub/more/test-suite.html
test ! -e bla
test ! -e sub/more/x.txt

# Create HTML output for individual tests.

$MAKE bla
$MAKE foo.html bar.sh.html
grep 'Blah Blah Blah' foo.html
grep 'this is .*bar\.sh' bar.sh.html
test ! -e mu.hml

ocwd=$(pwd) || fatal_ "getting current workind directory"
( cd sub/more \
    && $MAKE sh.html \
    && test -f sh.html \
    && test ! -e test.html \
    && $MAKE test.html \
    && grep 'this is .*test\.test' test.html) || exit 1

# HTML output removed by mostlyclean.
$MAKE check-html
test -f mylog.html
test -f sub/more/test-suite.html
$MAKE mostlyclean
find . -name '*.html' | grep . && exit 1

$MAKE distcheck

:
