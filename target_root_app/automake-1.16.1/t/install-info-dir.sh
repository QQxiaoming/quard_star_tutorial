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

# Checks on the 'install-info' target.
# Details about the individual checks' purposes and motivations are
# inlined, below.

# FIXME: this test is a good candidate for a conversion to TAP,
# FIXME: and could be merged with 'txinfo27.sh'.

required=makeinfo
. test-init.sh

cwd=$(pwd) || fatal_ "cannot get current working directory"

mkdir bin
saved_PATH=$PATH; export saved_PATH
PATH=$cwd/bin$PATH_SEPARATOR$PATH; export PATH

cat >> configure.ac <<'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
info_TEXINFOS = foo.texi
END

cat > foo.texi << 'END'
\input texinfo
@setfilename foo.info
@c All the following directives might be required to allow older
@c versions of the 'install-info' program (e.g., version 4.8) to
@c create the 'dir' index file in ${infodir}.  Don't remove them.
@settitle foo
@dircategory Dummy utilities
@direntry
* Foo: (foo).  Does nothing at all, but has a nice name.
@end direntry
@node Top
Hello world!
@bye
END

$ACLOCAL
$AUTOMAKE -a
$AUTOCONF

instdir=_inst
destdir=_dest

./configure --prefix="$cwd/$instdir" --infodir="$cwd/$instdir/info"

$MAKE info
test -f foo.info

if install-info --version; then
  # Skip some checks even if 'install-info' is the one from dpkg, not
  # the one from GNU info, as the former might try to create files in
  # '/var/backups/', causing spurious failures like this for non-root
  # users.
  if install-info --version | $EGREP -i '(dpkg|debian) install-info'; then
    have_installinfo=no
  else
    have_installinfo=yes
  fi
else
  have_installinfo=no
fi

# The 'install-info' target updates the '${infodir}/dir' file
# by default (if the 'install-info' program is available).
# This should happen in a normal as well as in a DESTDIR installation.
if test $have_installinfo = yes; then

  $MAKE install-info
  test -f $instdir/info/foo.info
  cat $instdir/info/dir
  $FGREP 'Does nothing at all, but has a nice name' $instdir/info/dir

  $MAKE uninstall
  test ! -e $instdir/info/foo.info
  cat $instdir/info/dir
  $FGREP 'but has a nice name' $instdir/info/dir && exit 1

  dir="$destdir/$cwd/$instdir/info"

  $MAKE DESTDIR="$cwd/$destdir" install-info
  test -f "$dir"/foo.info
  cat "$dir"/dir
  $FGREP 'Does nothing at all, but has a nice name' "$dir"/dir
  $MAKE DESTDIR="$cwd/$destdir" uninstall
  test ! -e "$dir"/foo.info
  $FGREP 'but has a nice name' "$dir"/dir && exit 1

  unset dir

fi

rm -rf $instdir $destdir

# The 'install-info' target doesn't fail if the 'install-info'
# program is not available.
cat > bin/install-info <<'END'
#!/bin/sh
echo error from install-info >&2
exit 127
END
chmod a+x bin/install-info
run_make -M install-info
test -f $instdir/info/foo.info
test ! -e $instdir/info/dir
grep 'error from install-info' output && exit 1

rm -rf $instdir output

if test $have_installinfo = yes; then
  # The 'install-info' target doesn't try to guess whether the 'install-info'
  # is the GNU or debian version.
  unindent > bin/install-info <<'END'
    #!/bin/sh
    set -e; set -u;
    for fd in 1 2; do
      for str in dpkg debian Debian; do
        eval "echo This is $str install-info >&$fd"
      done
    done
    PATH=$saved_PATH; export PATH
    exec install-info ${1+"$@"}
END
  $MAKE install-info
  test -f $instdir/info/foo.info
  cat $instdir/info/dir
  $MAKE uninstall
  test ! -e $instdir/info/foo.info
  cat $instdir/info/dir
  $FGREP 'but has a nice name' $instdir/info/dir && exit 1
  : For shells with busted 'set -e'.
fi

rm -rf $instdir bin/install-info

# The 'AM_UPDATE_INFO_DIR' environment variable can be used to
# prevent the creation or update of the '${infodir}/dir' file,
# if set to a "no" value.
for val in no NO n; do
  rm -rf $instdir
  env AM_UPDATE_INFO_DIR="$val" $MAKE install-info
  test -f $instdir/info/foo.info
  test ! -e $instdir/info/dir
done

$MAKE install-info
if test $have_installinfo != yes; then
  echo 'Does nothing at all, but has a nice name' > $instdir/info/dir
fi

chmod a-w $instdir/info/dir
for val in no NO n; do
  env AM_UPDATE_INFO_DIR="$val" $MAKE uninstall
  cat $instdir/info/dir
  $FGREP 'Does nothing at all, but has a nice name' $instdir/info/dir
done

if test $have_installinfo = yes; then
  for val in 'yes' 'who cares!'; do
    rm -rf $instdir
    env AM_UPDATE_INFO_DIR="$val" $MAKE install-info
    test -f $instdir/info/foo.info
    cat $instdir/info/dir
    env AM_UPDATE_INFO_DIR="$val" $MAKE uninstall
    test ! -e $instdir/info/foo.info
    $FGREP 'but has a nice name' $instdir/info/dir && exit 1
    : For shells with busted 'set -e'.
  done
fi

:
