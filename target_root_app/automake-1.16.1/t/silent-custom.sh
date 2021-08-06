#!/bin/sh
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

# Check user extensibility of silent-rules mode.

. test-init.sh

cat >>configure.ac <<'EOF'
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
EOF

# We delegate all the work to the subdir makefile.  This is done
# to ensure any command-line setting of $(V) gets correctly passed
# down to recursive make invocations.
echo SUBDIRS = sub > Makefile.am

mkdir sub
cat > sub/Makefile.am <<'EOF'
AUTOMAKE_OPTIONS = -Wno-portability-recursive
my_verbose = $(my_verbose_$(V))
my_verbose_ = $(my_verbose_$(AM_DEFAULT_VERBOSITY))
my_verbose_0 = @echo " XGEN    $@";

all-local: foo gen-headers

list = 0 1 2
.PHONY: gen-headers
gen-headers:
	@headers=`for i in $(list); do echo sub/$$i.h; done`; \
	if $(AM_V_P); then set -x; else \
	  echo " GEN     [headers]"; \
	fi; \
	rm -f $$headers || exit 1; \
## Only fake header generation.
	: generate-header --flags $$headers

foo: foo.in
	$(my_verbose)cp $(srcdir)/foo.in $@
EXTRA_DIST = foo.in
CLEANFILES = foo
EOF

: > sub/foo.in

$ACLOCAL
$AUTOMAKE --add-missing
$AUTOCONF

do_check ()
{
  case ${1-} in
    --silent) silent=:;;
    --verbose) silent=false;;
    *) fatal_ "do_check(): incorrect usage";;
  esac
  shift
  $MAKE clean
  run_make -M -- ${1+"$@"}
  if $silent; then
    $FGREP 'cp ' output && exit 1
    $FGREP 'generate-header' output && exit 1
    $FGREP 'rm -f' output && exit 1
    grep '[012]\.h' output && exit 1
    grep '^ XGEN    foo$' output
    grep '^ GEN     \[headers\]$' output
  else
    $FGREP 'GEN ' output && exit 1
    $FGREP 'cp ./foo.in foo' output
    # Be prepared to handle "creative quoting" in the shell traces.
    # See automake bug#14760.
    ok=false
    for q in '' \' \"; do
      files="${q}sub/0.h${q} ${q}sub/1.h${q} ${q}sub/2.h${q}"
      $FGREP "rm -f $files" output || continue
      $FGREP "generate-header --flags $files" output || continue
      ok=:
      break
    done
    $ok || exit 1
    unset ok
  fi
}

./configure --enable-silent-rules
do_check --silent
do_check --verbose V=1

$MAKE distclean

./configure --disable-silent-rules
do_check --verbose
do_check --silent V=0

$MAKE distclean

:
