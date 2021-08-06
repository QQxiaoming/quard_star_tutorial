#! /bin/sh
# Copyright (C) 2014-2018 Free Software Foundation, Inc.
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

# AM_AUX_DIR_EXPAND should ensure $ac_aux_dir is properly initialized.
# Issue revealed by related automake bug#15981.

. test-init.sh

cat > configure.ac <<'END'
AC_INIT([test], [0.0])
AM_AUX_DIR_EXPAND
printf '%s\n' "ac_aux_dir: '$ac_aux_dir'"
printf '%s\n' "am_aux_dir: '$am_aux_dir'"
test "$ac_aux_dir" = . || AS_EXIT([1])
test "$am_aux_dir" = "`pwd`" || AS_EXIT([1])
AS_EXIT([0])
END

$ACLOCAL
$AUTOCONF

test -f install-sh  # sanity check

./configure

:
