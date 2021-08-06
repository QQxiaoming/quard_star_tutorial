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

# Check that remake rules from subdirectories do not hang or cycle
# endlessly, even with build systems that takes several seconds to
# rebuild the Makefiles.
# This test tries to ensure a long-enough rebuild time by introducing
# an explicit delay in the build process.
# Suggestion by Ralf Wildenhues.

. test-init.sh

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_FILES([Makefile sub/Makefile])
AC_SUBST([MAGIC], [magic])
AC_OUTPUT
END

echo SUBDIRS = sub > Makefile.am
mkdir sub
: > sub/Makefile.am

# Both aclocal and automake are expected to run one and just one time.
# Create and use wrappers that will verify that.

ocwd=$(pwd) || fatal_ "cannot get current working directory"

mkdir bin

cat > bin/automake <<END
#!/bin/sh
set -e
PATH='$PATH'; export PATH
sentinel='$ocwd/automake-has-run'
if test -f "\$sentinel"; then
  echo "Automake has been run more than one time" >&2
  exit 1
else
  echo automake has run > "\$sentinel"
fi
$sleep; $sleep;
exec $AUTOMAKE \${1+"\$@"}
END
chmod a+x bin/automake

cat > bin/aclocal <<END
#!/bin/sh
set -e
PATH='$PATH'; export PATH
sentinel='$ocwd/aclocal-has-run'
if test -f "\$sentinel"; then
  echo "Aclocal has been run more than one time" >&2
  exit 1
else
  echo aclocal has run > "\$sentinel"
fi
$sleep; $sleep;
exec $ACLOCAL \${1+"\$@"}
END
chmod a+x bin/aclocal

# Just to be sure.
cp bin/automake bin/automake-$APIVERSION
cp bin/aclocal bin/aclocal-$APIVERSION

PATH=$ocwd/bin$PATH_SEPARATOR$PATH; export PATH

AUTOMAKE=automake ACLOCAL=aclocal; export AUTOMAKE ACLOCAL

$ACLOCAL  # Should use or just-defined wrapper.
$AUTOMAKE # Likewise.
$AUTOCONF

# Sanity check: the wrappers have been used.
test -f automake-has-run
test -f aclocal-has-run
rm -f automake-has-run aclocal-has-run

./configure
# Sanity check: Makefile doesn't get updated uselessly.
run_make ACLOCAL=false AUTOMAKE=false AUTOCONF=false

$sleep
sed "s|magic|magic2|" configure.ac > t
mv -f t configure.ac

cd sub
run_make Makefile AUTOMAKE="$AUTOMAKE" ACLOCAL="$ACLOCAL"
cd ..

# For debugging.
ls -l . sub
grep -i magic configure Makefile.in Makefile sub/Makefile.in sub/Makefile
# Sanity checks.
$FGREP magic2 configure
$FGREP magic2 Makefile
$FGREP magic2 sub/Makefile

:
