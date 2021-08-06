#! /bin/sh
# This file has been automatically generated.  DO NOT EDIT BY HAND!
. test-lib.sh

am_serial_tests=yes
# In the spirit of VPATH, we prefer a test in the build tree
# over one in the source tree.
for dir in . "$am_top_srcdir"; do
  if test -f "$dir/t/check11.sh"; then
    echo "$0: will source $dir/t/check11.sh"
    . "$dir/t/check11.sh"; exit $?
  fi
done
echo "$0: cannot find wrapped test 't/check11.sh'" >&2
exit 99
