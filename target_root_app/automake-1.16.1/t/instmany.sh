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
# Here, the main issue is that we may prepend '$(srcdir)/' to each file,
# which may cause much longer command lines.  The list of files must
# anyway remain below the limit, otherwise 'make' won't be able to even
# fork the command.
#
# Further, the install rule should honor failures of the install program.

# Python is done in the sister test.
# For texinfos, we expand names using $(srcdir) in the first place.
# Let's hope nobody uses many texinfos.

. test-init.sh

# In order to have a useful test on modern systems (which have a high
# limit, if any), use a fake install program that errors out for more
# than 2K characters in a command line.  The POSIX limit is 4096, but
# that may include space taken up by the environment.

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
bin_SCRIPTS =
nobase_bin_SCRIPTS =
data_DATA =
nobase_data_DATA =
include_HEADERS =
nobase_include_HEADERS =
END

for n in $list; do
  unindent >>Makefile.am <<END
    bin_SCRIPTS += script$n
    nobase_bin_SCRIPTS += nscript$n
    data_DATA += data$n
    nobase_data_DATA += ndata$n
    include_HEADERS += header$n.h
    nobase_include_HEADERS += nheader$n.h
END
  echo >script$n
  echo >nscript$n
  echo >data$n
  echo >ndata$n
  echo >header$n.h
  echo >nheader$n.h
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
test -f "$instdir/bin/script1"
# Multiple uninstall should work, too.
$MAKE uninstall
$MAKE uninstall
test $(find "$instdir" -type f -print | wc -l) -eq 0

# Try whether we don't exceed the low limit.
PATH=$nPATH; export PATH
run_make INSTALL=my-install install
test -f "$instdir/bin/script1"
run_make INSTALL=my-install uninstall
test $(find "$instdir" -type f -print | wc -l) -eq 0
PATH=$oPATH; export PATH

cd $subdir
srcdir=../../$subdir

# Ensure 'make install' fails when 'install' fails.

# We cheat here, for efficiency, knowing the internal rule names.
# For correctness, one should '$MAKE install' here always, or at
# least use install-exec or install-data.

for file in script3 script$nfiles
do
  chmod a-r $srcdir/$file
  test ! -r $srcdir/$file || skip_ "cannot drop file read permissions"
  $MAKE install-binSCRIPTS && exit 1
  chmod u+r $srcdir/$file
done

for file in nscript3 nscript$nfiles
do
  chmod a-r $srcdir/$file
  $MAKE install-nobase_binSCRIPTS && exit 1
  chmod u+r $srcdir/$file
done

for file in data3 data$nfiles
do
  chmod a-r $srcdir/$file
  $MAKE install-dataDATA && exit 1
  chmod u+r $srcdir/$file
done

for file in ndata3 ndata$nfiles
do
  chmod a-r $srcdir/$file
  $MAKE install-nobase_dataDATA && exit 1
  chmod u+r $srcdir/$file
done

for file in header3.h header$nfiles.h
do
  chmod a-r $srcdir/$file
  $MAKE install-includeHEADERS && exit 1
  chmod u+r $srcdir/$file
done

for file in nheader3.h nheader$nfiles.h
do
  chmod a-r $srcdir/$file
  $MAKE install-nobase_includeHEADERS && exit 1
  chmod u+r $srcdir/$file
done

:
