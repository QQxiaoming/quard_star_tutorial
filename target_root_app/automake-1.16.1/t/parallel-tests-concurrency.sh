#! /bin/sh
# Copyright (C) 2009-2018 Free Software Foundation, Inc.
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

# Check parallel-tests features:
# - concurrent parallel execution

# FIXME: we should factor out (into am-test-lib.sh?) the code to determine
#        how to run make in parallel mode ...

. test-init.sh

case $MAKE in
  *\ -j*) skip_ "\$MAKE contains '-j'";;
esac

if using_gmake; then
  j=-j
else
  unindent > Makefile <<END
    all: one two
    one:
	$sleep && test -f two
    two:
	echo ok > \$@
END
  for j in "-j" "-j " NONE; do
    if test x"$j" = xNONE; then
      skip_ "can't run make in parallel mode"
    fi
    run_make -M -- ${j}2 all || continue
    $EGREP -i "(warning|error):|-j[\"\'\` ]" output && continue
    break
  done
  rm -f one output Makefile
fi

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
TESTS =
END

cat > x <<'END'
#! /bin/sh
echo "this is $0"
# Creative quoting below to please maintainer-check.
sleep '1'
exit 0
END
chmod a+x ./x

mkdir sub
for i in 1 2 3; do
  echo "TESTS += foo$i.test" >> Makefile.am
  cp x foo$i.test
  echo "TESTS += zap$i" >> Makefile.am
  cp x zap$i
  echo "TESTS += sub/bar$i.test" >> Makefile.am
  cp x sub/bar$i.test
  echo "TESTS += sub/mu$i" >> Makefile.am
  cp x sub/mu$i
done

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

for build in serial parallel; do
  mkdir $build
  cd $build
  ../configure
  $MAKE
  cd ..
done

cd serial
# Do *not* use "make -j1" here; apparently, some make implementations
# (e.g., AIX 7.1) interpret it as a synonym of "make -j" :-(
$MAKE check &
cd ..

cd parallel
$sleep
run_make -O -- ${j}4 check
cd ..

# Ensure the tests are really being run in parallel mode: if this is
# the case, the serial run of the dummy testsuite started above should
# still be ongoing when the parallel one has terminated.
kill -0 $!
cat parallel/stdout
test $(grep -c '^PASS:' parallel/stdout) -eq 12

# Wait long enough so that there are no open files any more when the
# post-test cleanup runs.  But exit after we've waited for two minutes
# or more, to avoid testsuite hangs in unusual situations (this has
# already happened).
i=1
while test ! -f serial/test-suite.log && test $i -le 120; do
  i=$(($i + 1))
  sleep '1' # Extra quoting to please maintainer-check.
done
$sleep

:
