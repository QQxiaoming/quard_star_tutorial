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

# Check that python support can work well with virtualenvs.
# This test also works as a mild stress-test on the python support.

required='cc python virtualenv'
. test-init.sh

# In case the user's config.site defines pythondir or pyexecdir.
CONFIG_SITE=/dev/null; export CONFIG_SITE

py_version_pre=$($PYTHON -V)

# Skip the test if a proper virtualenv cannot be created.
virtualenv -p"$PYTHON" --verbose virtenv && py_installed virtenv/bin/activate \
  || skip_ "couldn't create python virtual environment"

# Activate the virtualenv.
. ./virtenv/bin/activate
# Sanity check.
if test -z "$VIRTUAL_ENV"; then
  framework_failure_ "can't activate python virtual environment"
fi

py_version_post=$(python -V)

# Sanity check.
test "$py_version_pre" = "$py_version_post"

cwd=$(pwd) || fatal_ "getting current working directory"
py_version=$(python -c 'import sys; print("%u.%u" % tuple(sys.version_info[:2]))')
py_site=$VIRTUAL_ENV/lib/python$py_version/site-packages

# We need to do do this early, just to set some cache variables properly,
# since because we're going to unset $PYTHON next.
if python_has_pep3147; then
  : PEP 3147 will be used in installation of ".pyc" files
fi
# We don't want our original python to be picked up by configure
# invocations.
unset PYTHON

# We need control over the package name.
cat > configure.ac << END
AC_INIT([am_virtenv], [1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile])
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
AM_PATH_PYTHON
AC_OUTPUT
END

cat > Makefile.am << 'END'
python_PYTHON = am_foo.py
pkgpython_PYTHON = __init__.py
pyexec_LIBRARIES = libquux.a
libquux_a_SOURCES = foo.c
pkgpyexec_LIBRARIES = libzardoz.a
libzardoz_a_SOURCES = foo.c

.PYTHON: debug test-run
debug:
	@echo PYTHON: $(PYTHON)
	@echo PYTHON_VERSION: $(PYTHON_VERSION)
	@echo prefix: $(prefix)
	@echo pythondir: $(pythondir)
	@echo pkgpythondir: $(pkgpythondir)
	@echo pyexecdir: $(pyexecdir)
	@echo pkgpyexecdir: $(pkgpyexecdir)
test-run:
	## In a virtualenv, the default python must be the custom
	## virtualenv python.
	@: \
	  && py1=`python -c 'import sys; print(sys.executable)'` \
	  && py2=`$(PYTHON) -c 'import sys; print(sys.executable)'` \
	  && echo "py1: $$py1" \
	  && echo "py2: $$py2" \
	  && test -n "$$py1" \
	  && test -n "$$py2" \
	  && test x"$$py1" = x"$$py2"
	## Check that modules installed in the virtualenv are readily
	## available.
	python -c 'from am_foo import foo_func; assert (foo_func () == 12345)'
	python -c 'from am_virtenv import old_am; assert (old_am () == "AutoMake")'
all-local: debug
END

cat > am_foo.py << 'END'
def foo_func ():
    return 12345
END

cat > __init__.py << 'END'
def old_am ():
    return 'AutoMake'
END

cat > foo.c << 'END'
int foo (void)
{
  return 0;
}
END

check_install ()
{
  $MAKE install ${1+"$@"}

  test -f      "$py_site"/am_foo.py
  py_installed "$py_site"/am_foo.pyc
  py_installed "$py_site"/am_foo.pyo
  py_installed "$py_site"/am_virtenv/__init__.py
  py_installed "$py_site"/am_virtenv/__init__.pyc
  py_installed "$py_site"/am_virtenv/__init__.pyo
  test -f      "$py_site"/libquux.a
  test -f      "$py_site"/am_virtenv/libzardoz.a
}

check_uninstall ()
{
  $MAKE uninstall ${1+"$@"}

  test ! -e          "$py_site"/am_foo.py
  py_installed --not "$py_site"/am_foo.pyc
  py_installed --not "$py_site"/am_foo.pyo
  test ! -e          "$py_site"/am_virtenv/__init__.py
  py_installed --not "$py_site"/am_virtenv/__init__.pyc
  py_installed --not "$py_site"/am_virtenv/__init__.pyo
  test ! -e          "$py_site"/libquux.a
  test ! -e          "$py_site"/am_virtenv/libzardoz.a
}

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

# Try a VPATH build.
mkdir build
cd build
../configure --prefix="$VIRTUAL_ENV"
check_install
$MAKE test-run
check_uninstall
cd ..

# Try an in-tree build.
./configure --prefix="$VIRTUAL_ENV"
check_install
$MAKE test-run
check_uninstall

$MAKE distclean

# Overriding pythondir and pyexecdir with cache variables should work.
./configure am_cv_python_pythondir="$py_site" \
            am_cv_python_pyexecdir="$py_site"
check_install
$MAKE test-run
check_uninstall

$MAKE distclean

# Overriding pythondir and pyexecdir at make time should be enough.
./configure --prefix="$cwd/bad-prefix"
check_install pythondir="$py_site" pyexecdir="$py_site" \
              AM_MAKEFLAGS="pythondir='$py_site' pyexecdir='$py_site'"
test ! -e bad-prefix
$MAKE test-run
check_uninstall pythondir="$py_site" pyexecdir="$py_site" \
                AM_MAKEFLAGS="pythondir='$py_site' pyexecdir='$py_site'"

# Also check that the distribution is self-contained, for completeness.
$MAKE distcheck

# Finally, check that if we disable the virtualenv, we shouldn't be
# able to access to the installed modules anymore.
cd build
$MAKE install
python -c 'import am_foo; print(am_foo.__file__)'
python -c 'import am_virtenv; print(am_virtenv.__file__)'
deactivate "nondestructive"
python -c 'import am_foo' && exit 1
python -c 'import am_virtenv' && exit 1

:
