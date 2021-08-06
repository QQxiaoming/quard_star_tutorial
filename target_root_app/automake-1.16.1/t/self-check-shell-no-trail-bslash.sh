#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# Check that our fake "shell" used to guard against use of trailing
# backslashes in recipes actually complains when those are used.

# Our hack doesn't work with some make implementations (see comments
# in 't/ax/shell-no-trail-bslash.in' for more details).
required=GNUmake
am_create_testdir=empty
. test-init.sh

cat >> Makefile <<'END'
am__backslash = \\ # foo
.PHONY: good bad
good:
	@printf '%s\n' OK
.PHONY: bad
bad:
	@echo $(am__backslash)
END

SHELL=$am_testaux_builddir/shell-no-trail-bslash

$SHELL -c 'exit 0'
test "$($SHELL -c 'echo is  o\k')" = "is ok"

echo 'echo is  ok\"' > ok.sh
$SHELL ./ok.sh
test "$($SHELL ./ok.sh)" = "is ok\""

tab='	'
nl='
'
for sfx in \
  '\' \
  '\\' \
  '\\\\\' \
  '\ ' \
  "\\$tab" \
  "\\ $tab$tab   " \
  "\\$nl" \
  "\\ $nl " \
  "\\$nl$nl$nl" \
; do
  for pfx in "" "echo bad" ": a${nl}# multine${nl}: text"; do
    cmd=${pfx}${sfx}
    printf '%s' "$cmd" > bad.sh
    for args in '-c "$cmd"' './bad.sh'; do
      eval "\$SHELL $args 2>stderr && { cat stderr >&2; exit 1; }; :"
      cat stderr >&2
      $FGREP "recipe/script ends with backslash character" stderr
      cmd="$cmd" $PERL -w -e '
        undef $/;
        $_ = <>;
        index($_, $ENV{cmd}) >= 0 or exit 1;
      ' <stderr
      $FGREP "$cmd" stderr
    done
  done
done

$MAKE good

run_make -E -e FAIL bad SHELL="$SHELL"
$FGREP "recipe/script ends with backslash character" stderr

:
