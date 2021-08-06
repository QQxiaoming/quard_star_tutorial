#! /bin/sh
# Copyright (C) 2008-2018 Free Software Foundation, Inc.
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

# Installing many files should not exceed the command line length limit.

# This is the python sister test of 'instmany.sh', see there for details.

required='python'
. test-init.sh

limit=2500
subdir=long_subdir_name_with_many_characters
nfiles=81
list=$(seq_ 1 $nfiles)

oPATH=$PATH; export oPATH
nPATH=$(pwd)/x-bin$PATH_SEPARATOR$PATH; export nPATH

mkdir x-bin

sed "s|@limit@|$limit|g" >x-bin/my-install <<'END'
#! /bin/sh
limit=@limit@
PATH=$oPATH; export PATH
if test -z "$orig_INSTALL"; then
  echo "$0: \$orig_INSTALL variable not set" >&2
  exit 1
fi
len=`expr "$orig_INSTALL $*" : ".*" 2>/dev/null || echo $limit`
if test $len -ge $limit; then
  echo "$0: safe command line limit of $limit characters exceeded" >&2
  exit 1
fi
exec $orig_INSTALL "$@"
exit 1
END

# Creative quoting in the next line to please maintainer-check.
sed "s|@limit@|$limit|g" >x-bin/'rm' <<'END'
#! /bin/sh
limit=@limit@
PATH=$oPATH; export PATH
RM='rm -f'
len=`expr "$RM $*" : ".*" 2>/dev/null || echo $limit`
if test $len -ge $limit; then
  echo "$0: safe command line limit of $limit characters exceeded" >&2
  exit 1
fi
exec $RM "$@"
exit 1
END

# Creative quoting in the next line to please maintainer-check.
chmod +x x-bin/'rm' x-bin/my-install

cat >setenv.in <<'END'
orig_INSTALL='@INSTALL@'
# In case we've falled back on the install-sh script (seen e.g.,
# on AIX 7.1), we need to make sure we use its absolute path,
# as we don't know from which directory we'll be run.
case "$orig_INSTALL" in
   /*) ;;
  */*) orig_INSTALL=$(pwd)/$orig_INSTALL;;
esac
export orig_INSTALL
END

cat >>configure.ac <<END
AM_PATH_PYTHON
AC_CONFIG_FILES([setenv.sh:setenv.in])
AC_CONFIG_FILES([$subdir/Makefile])
AC_OUTPUT
END

cat >Makefile.am <<END
SUBDIRS = $subdir
END

mkdir $subdir
cd $subdir

cat >Makefile.am <<'END'
python_PYTHON =
nobase_python_PYTHON =
END

for n in $list; do
  unindent >>Makefile.am <<END
    python_PYTHON += python$n.py
    nobase_python_PYTHON += npython$n.py
END
  echo >python$n.py
  echo >npython$n.py
done

cd ..
$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

instdir=$(pwd)/inst
mkdir build
cd build
../configure --prefix="$instdir"
. ./setenv.sh
test -n "$orig_INSTALL"
$MAKE
# Try whether native install (or install-sh) works.
$MAKE install
test -n "$(find "$instdir" -name python1.py)"
# Multiple uninstall should work, too.
$MAKE uninstall
$MAKE uninstall
test $(find "$instdir" -type f -print | wc -l) -eq 0

# Try whether we don't exceed the low limit.
PATH=$nPATH; export PATH
run_make INSTALL=my-install install
test -n "$(find "$instdir" -name python1.py)"
run_make INSTALL=my-install uninstall
test $(find "$instdir" -type f -print | wc -l) -eq 0
PATH=$oPATH; export PATH

cd $subdir
srcdir=../../$subdir

# Ensure 'make install' fails when 'install' fails.

for file in python3.py python$nfiles.py
do
  chmod a-r $srcdir/$file
  test ! -r $srcdir/$file || skip_ "cannot drop file read permissions"
  $MAKE install && exit 1
  chmod u+r $srcdir/$file
done

for file in npython3.py npython$nfiles.py
do
  chmod a-r $srcdir/$file
  $MAKE install && exit 1
  chmod u+r $srcdir/$file
done

:
