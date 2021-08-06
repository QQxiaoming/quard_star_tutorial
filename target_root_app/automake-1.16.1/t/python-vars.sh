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

# Check that AM_PATH_PYTHON correctly sets all the output variables
# advertised in the manual.

required=python
. test-init.sh

# In case the user's config.site defines pythondir or pyexecdir.
CONFIG_SITE=/dev/null; export CONFIG_SITE

# Update the definition below if the documentation changes.
# Note that the value of the 'pythondir' and 'pyexecdir' variables can
# vary among different python installations, so we need more relaxed
# and ad-hoc checks for them.  Also, more proper "functional" checks
# on them should be done in the 'python-virtualenv.sh' test.
PYTHON_VERSION=$($PYTHON -c 'import sys; print(sys.version[:3])') || exit 1
PYTHON_PLATFORM=$($PYTHON -c 'import sys; print(sys.platform)') || exit 1
PYTHON_EXEC_PREFIX='${exec_prefix}'
PYTHON_PREFIX='${prefix}'
pkgpythondir="\${pythondir}/$me"
pkgpyexecdir="\${pyexecdir}/$me"

pyvars='PYTHON_VERSION PYTHON_PLATFORM PYTHON_PREFIX PYTHON_EXEC_PREFIX
        pkgpythondir pkgpyexecdir'

cat >> configure.ac << 'END'
AC_CONFIG_FILES([vars-got pythondir pyexecdir])
AM_PATH_PYTHON
AC_OUTPUT
END

cat > my.py << 'END'
def my():
    return 1
END

cat > Makefile.am << 'END'

python_PYTHON = my.py

EXTRA_DIST = vars-exp

check-local: test-in test-am
.PHONY: test-in test-am

test-in:
	cat pythondir
	case `cat pythondir` in '$${prefix}'/*);; *) exit 1;; esac
	cat pyexecdir
	case `cat pyexecdir` in '$${exec_prefix}'/*);; *) exit 1;; esac
	cat $(srcdir)/vars-exp
	cat $(builddir)/vars-got
	diff $(srcdir)/vars-exp $(builddir)/vars-got

## Note: this target's rules will be extended in the "for" loop below.
test-am:
	case '$(pythondir)' in '$(prefix)'/*);; *) exit 1;; esac
	case '$(pyexecdir)' in '$(exec_prefix)'/*);; *) exit 1;; esac
END

echo @pythondir@ > pythondir.in
echo @pyexecdir@ > pyexecdir.in

: > vars-exp
: > vars-got.in

for var in $pyvars; do
  eval val=\$$var
  echo "var=$val" >> vars-exp
  echo "var=@$var@" >> vars-got.in
  echo "${tab}test x'\$($var)' = x'$val'" >> Makefile.am
done

cat Makefile.am
cat vars-got.in

$ACLOCAL
$AUTOMAKE --add-missing

for var in pythondir pyexecdir $pyvars; do
  grep "^$var *=" Makefile.in
done

$AUTOCONF
./configure PYTHON="$PYTHON"

$MAKE test-in test-am
$MAKE distcheck

:
