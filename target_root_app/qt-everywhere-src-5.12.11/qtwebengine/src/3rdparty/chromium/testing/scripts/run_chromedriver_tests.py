#!/usr/bin/env python
# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Runs chrome driver tests.

This script attempts to emulate the contract of gtest-style tests
invoked via recipes. The main contract is that the caller passes the
argument:

  --isolated-script-test-output=[FILENAME]

json is written to that file in the format detailed here:
https://www.chromium.org/developers/the-json-test-results-format

Optional argument:

  --isolated-script-test-filter=[TEST_NAMES]

is a double-colon-separated ("::") list of test names, to run just that subset
of tests. This list is forwarded to the chrome driver test runner.
"""

import argparse
import json
import os
import shutil
import sys
import tempfile
import traceback

import common

def main():
  parser = argparse.ArgumentParser()

  # --isolated-script-test-output is passed through to the script.

  # This argument is ignored for now.
  parser.add_argument('--isolated-script-test-chartjson-output', type=str)
  # This argument is ignored for now.
  parser.add_argument('--isolated-script-test-perf-output', type=str)
  # This argument is translated below.
  parser.add_argument('--isolated-script-test-filter', type=str)

  args, rest_args = parser.parse_known_args()

  filtered_tests = args.isolated_script_test_filter
  if filtered_tests:
    if any('--filter' in arg for arg in rest_args):
      parser.error(
          'can\'t have the test call filter with the'
          '--isolated-script-test-filter argument to the wrapper script')

    # https://github.com/google/googletest/blob/master/googletest/docs/advanced.md#running-a-subset-of-the-tests
    # says that the gtest filter should accept single colons separating
    # individual tests. The input is double colon separated, so translate it.
    rest_args = rest_args + ['--filter', filtered_tests.replace('::', ':')]

  cmd = [sys.executable] + rest_args
  return common.run_command(cmd)


# This is not really a "script test" so does not need to manually add
# any additional compile targets.
def main_compile_targets(args):
  json.dump([], args.output)


if __name__ == '__main__':
  # Conform minimally to the protocol defined by ScriptTest.
  if 'compile_targets' in sys.argv:
    funcs = {
      'run': None,
      'compile_targets': main_compile_targets,
    }
    sys.exit(common.run_script(sys.argv[1:], funcs))
  sys.exit(main())
