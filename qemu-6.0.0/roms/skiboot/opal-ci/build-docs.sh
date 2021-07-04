#!/bin/bash

set -uo pipefail
set -e
set -vx

MAKE_J=$(grep -c processor /proc/cpuinfo)
export CROSS="ccache powerpc64-linux-gnu-"

# There's a bug in dtc v1.4.7 packaged on fedora 28 that makes our device tree
# tests fail, so for the moment, build a slightly older DTC
git clone --depth=1 -b v1.4.4 https://git.kernel.org/pub/scm/utils/dtc/dtc.git
(cd dtc; make -j${MAKE_J})
export PATH=`pwd`/dtc:$PATH

make -j${MAKE_J} SKIBOOT_GCOV=1 coverage-report

pip install -r doc/requirements.txt
(cd doc; make html)

cp -r doc/ghpages-skeleton doc/_build/ghpages
mv coverage-report doc/_build/ghpages/
mv doc/_build/html doc/_build/ghpages/doc
