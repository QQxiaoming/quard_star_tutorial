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

# Test rebuild rules for Java class files.

required='javac'
. test-init.sh

cat >> configure.ac <<'END'
AC_OUTPUT
END

cat > Makefile.am <<'END'
AM_JAVACFLAGS = -verbose
foodir = $(datadir)/java
foo_JAVA = a.java
dist_foo_JAVA = d.java
nodist_foo_JAVA = n.java
nobase_foo_JAVA = Nobase.java
nobase_dist_foo_JAVA = NobaseDist.java
nobase_nodist_foo_JAVA = NobaseNoDist.java
END

echo 'class _x {}' > a.java
echo 'class x_ {}' > d.java
echo 'class a {} class d {}' > n.java
echo 'class Nobase_Foo {} class Nobase_Bar {}' > Nobase.java
echo 'class NobaseDist {}' > NobaseDist.java
echo 'class NobaseNoDist {}' > NobaseNoDist.java

$ACLOCAL
$AUTOCONF
$AUTOMAKE

all_classes='_x x_ a d Nobase_Foo Nobase_Bar NobaseDist NobaseNoDist'

for vpath in : false; do

  if $vpath; then
    srcdir=..
    mkdir build
    cd build
  else
    srcdir=.
  fi

  $srcdir/configure
  $MAKE
  ls -l # For debugging.

  # Sanity check.
  test -f classfoo.stamp
  for cls in $all_classes; do
    test -f $cls.class
  done

  # When the stampfile is removed, all the *.class files should
  # be considered out-of-date.
  echo timestamp > older
  $sleep
  rm -f classfoo.stamp
  $MAKE
  for cls in $all_classes; do
    is_newest $cls.class older
  done

  # When only a java file is modified, only the *.class files derived from
  # it should be updated.
  # The strings we loop on here have the following format:
  # "JAVA-FILES-TO-BE-TOUCHED -- CLASSES-THAT-SHOULD-BE-UPDATED"
  for args in \
    'a -- _x' \
    'd -- x_' \
    'n -- a d' \
    'a d Nobase -- _x x_ Nobase_Foo Nobase_Bar' \
    'n NobaseDist -- a d NobaseDist' \
    'd NobaseNoDist -- x_ NobaseNoDist' \
    "a d n Nobase NobaseDist NobaseNoDist -- $all_classes" \
  ; do
    set $args
    touched_javas=
    while test $# -gt 0; do
      if test x"$1" = x"--"; then
        shift
        break
      else
        touched_javas="$touched_javas $1"
        shift
      fi
    done
    updated_classes=$*
    echo timestamp > older
    $sleep
    for j in $touched_javas; do
      touch $srcdir/$j.java
    done
    $MAKE
    is_newest classfoo.stamp older
    for cls in $all_classes; do
      case " $updated_classes " in
        *" $cls "*) is_newest $cls.class older;;
        *) is_newest older $cls.class;;
      esac
    done
  done # $args ...

  cd $srcdir

done # $vpath ...

:
