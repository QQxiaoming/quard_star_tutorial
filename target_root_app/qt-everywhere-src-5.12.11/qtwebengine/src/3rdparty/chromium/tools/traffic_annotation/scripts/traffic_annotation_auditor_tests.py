#!/usr/bin/env python
# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Runs tests to ensure annotation tests are working as expected.
"""

import os
import argparse
import sys
import tempfile

from annotation_tools import NetworkTrafficAnnotationTools

# If this test starts failing, please set TEST_IS_ENABLED to "False" and file a
# bug to get this reenabled, and cc the people listed in
# //tools/traffic_annotation/OWNERS.
TEST_IS_ENABLED = True

MINIMUM_EXPECTED_NUMBER_OF_ANNOTATIONS = 260

class TrafficAnnotationTestsChecker():
  def __init__(self, build_path=None):
    """Initializes a TrafficAnnotationTestsChecker object.

    Args:
      build_path: str Absolute or relative path to a fully compiled build
          directory.
    """
    self.tools = NetworkTrafficAnnotationTools(build_path)
    self.last_result = None


  def RunAllTests(self):
    """Runs all tests and returns the result."""
    return self.CheckAuditorResults() and self.CheckOutputExpectations()


  def CheckAuditorResults(self):
    """Runs auditor using different configurations, expecting to run error free,
    and having equal results in the exported TSV file in all cases. The TSV file
    provides a summary of all annotations and their content.

    Returns:
      bool True if all results are as expected.
    """

    configs = [
      ["--test-only", "--error-resilient"],  # Similar to trybot.
      ["--test-only"],                       # Failing on any runtime error.
      ["--test-only", "--no-filtering"]      # Not using heuristic filtering.
    ]

    self.last_result = None
    for config in configs:
      result = self._RunTest(config)
      if not result:
        print("No output for config: %s" % config)
        return False
      if self.last_result and self.last_result != result:
        print("Unexpected different results for config: %s" % config)
        return False
      self.last_result = result
    return True


  def CheckOutputExpectations(self):
    # This test can be replaced by getting results from a diagnostic mode call
    # to traffic_annotation_auditor, and checking for an expected minimum number
    # of items for each type of pattern that it extracts. E.g., we should have
    # many annotations of each type (complete, partial, ...), functions that
    # need annotations, direct assignment to mutable annotations, etc.

    # |self.last_result| includes the content of the TSV file that the auditor
    # generates. Counting the number of end of lines in the text will give the
    # number of extracted annotations.
    annotations_count = self.last_result.count("\n")
    print("%i annotations found in auditor's output." % annotations_count)

    if annotations_count < MINIMUM_EXPECTED_NUMBER_OF_ANNOTATIONS:
      print("Annotations are expected to be at least %i." %
                MINIMUM_EXPECTED_NUMBER_OF_ANNOTATIONS)
      return False
    return True


  def _RunTest(self, args):
    """Runs the auditor test with given |args|, and returns the extracted
    annotations.

    Args:
      args: list of str Arguments to be passed to auditor.

    Returns:
      str Content of annotations.tsv file if successful, otherwise None.
    """

    print("Running auditor using config: %s" % args)
    temp_file = tempfile.NamedTemporaryFile()
    temp_filename = temp_file.name
    temp_file.close()

    _, stderr_text, return_code = self.tools.RunAuditor(
        args + ["--annotations-file=%s" % temp_filename])

    if os.path.exists(temp_filename):
      # When tests are run on all files (without filtering), there might be some
      # compile errors in irrelevant files on Windows that can be ignored.
      if (return_code and "--no-filtering" in args and
          sys.platform.startswith(('win', 'cygwin'))):
        print("Ignoring return code: %i" % return_code)
        return_code = 0
      annotations = None if return_code else open(temp_filename).read()
      os.remove(temp_filename)
    else:
      annotations = None

    if annotations:
      print("Test PASSED.")
    else:
      print("Test FAILED.\n%s" % stderr_text)

    return annotations


def main():
  if not TEST_IS_ENABLED:
    return 0

  parser = argparse.ArgumentParser(
      description="Traffic Annotation Tests checker.")
  parser.add_argument(
      '--build-path',
      help='Specifies a compiled build directory, e.g. out/Debug. If not '
           'specified, the script tries to guess it. Will not proceed if not '
           'found.')

  args = parser.parse_args()
  checker = TrafficAnnotationTestsChecker(args.build_path)
  return 0 if checker.RunAllTests() else 1


if '__main__' == __name__:
  sys.exit(main())