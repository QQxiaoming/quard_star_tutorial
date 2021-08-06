#! /bin/sh
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

# Test that we can distribute the same file as many times as we want.
# The distdir target should take care of not copying it more than one
# time anyway.

. test-init.sh

echo AC_OUTPUT >> configure.ac

cat > Makefile.am <<'END'
bin_PROGRAMS = foo bar
foo_SOURCES = foo.c
bar_SOURCES = foo.c
python_PYTHON = bar.py
EXTRA_DIST = foo.c bar.py

.PHONY: sanity-check
sanity-check:
	for f in $(DISTFILES); do echo " $$f "; done > dist.txt
	cat dist.txt
	test `grep ' foo\.c '  dist.txt | wc -l` -eq 3
	test `grep ' bar\.py ' dist.txt | wc -l` -eq 2

# So that we don't have to require a C compiler.
AUTOMAKE_OPTIONS = no-dependencies
CC = false

# So that we don't have to require a Python interpreter.
pythondir = ${prefix}/py
PYTHON = false
END

ocwd=$(pwd) || fatal_ "cannot get current working directory"

# Help to ensure cp won't see the same file twice.
mkdir bin
cat > bin/cp <<END
#!/bin/sh
PATH='$PATH'; export PATH

case " \$* " in
  *foo.c\ *)
    if test -f '$ocwd'/foo-c-copied; then
      echo "\$0: we tried to copy foo.c twice" >&2
      exit 1
    else
      # For a sanity check later.
      echo ok > '$ocwd'/cp-wrapper-has-seen-foo-c
    fi
    ;;
esac

case " \$* " in
  *bar.py\ *)
    if test -f '$ocwd'/bar-py-copied; then
      echo "\$0: we tried to copy bar.py twice" >&2
      exit 1
    else
      # For a sanity check later.
      echo ok > '$ocwd'/cp-wrapper-has-seen-bar-py
    fi
    ;;
esac

exec cp "\$@"
END
chmod a+x bin/cp
PATH=$(pwd)/bin$PATH_SEPARATOR$PATH; export PATH;

: > foo.c
: > bar.py
: > py-compile

# Help to ensure cp won't try to copy the same file twice.
chmod a-w foo.c bar.py

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure
$MAKE sanity-check || fatal_ "expected invariants not verified"
$MAKE distdir
test -f cp-wrapper-has-seen-foo-c && test -f cp-wrapper-has-seen-bar-py \
  || fatal_ "our cp wrapper hasn't run correctly"

:
