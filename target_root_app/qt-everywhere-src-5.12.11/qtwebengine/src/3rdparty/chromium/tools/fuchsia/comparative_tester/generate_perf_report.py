#!/usr/bin/env python3
# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""generate_perf_report.py is to be used after comparative_tester.py has been
executed and written some test data into the location specified by
target_spec.py. It writes to results_dir and reads all present test info from
raw_data_dir. Using this script should just be a matter of invoking it from
chromium/src while raw test data exists in raw_data_dir."""

import json
import logging
import math
import os
import sys
from typing import List, Dict, Set, Tuple, Optional, Any, TypeVar, Callable

import target_spec
from test_results import (TargetResult, ReadTargetFromJson, TestResult,
                          ResultLine)


class LineStats(object):

  def __init__(self, desc: str, unit: str, time_avg: float, time_dev: float,
               cv: float, samples: int) -> None:
    self.desc = desc
    self.time_avg = time_avg
    self.time_dev = time_dev
    self.cv = cv
    self.unit = unit
    self.sample_num = samples

  def ToString(self) -> str:
    if self.sample_num > 1:
      return "{}: {:.5f} σ={:.5f} {} with n={} cv={}".format(
          self.desc, self.time_avg, self.time_dev, self.unit, self.sample_num,
          self.cv)
    else:
      return "{}: {:.5f} with only one sample".format(self.desc, self.time_avg)


def LineFromList(lines: List[ResultLine]) -> Optional[LineStats]:
  desc = lines[0].desc
  unit = lines[0].unit
  times = [line.meas for line in lines]
  avg, dev, cv = GenStats(times)
  return LineStats(desc, unit, avg, dev, cv, len(lines))


class TestStats(object):

  def __init__(self, name: str, time_avg: float, time_dev: float, cv: float,
               samples: int, lines: List[LineStats]) -> None:
    self.name = name
    self.time_avg = time_avg
    self.time_dev = time_dev
    self.cv = cv
    self.sample_num = samples
    self.lines = lines

  def ToLines(self) -> List[str]:
    lines = []
    if self.sample_num > 1:
      lines.append("{}: {:.5f} σ={:.5f}ms with n={} cv={}".format(
          self.name, self.time_avg, self.time_dev, self.sample_num, self.cv))
    else:
      lines.append("{}: {:.5f} with only one sample".format(
          self.name, self.time_avg))
    for line in self.lines:
      lines.append("  {}".format(line.ToString()))
    return lines


def TestFromList(tests: List[TestResult]) -> TestStats:
  name = tests[0].name
  avg, dev, cv = GenStats([test.time for test in tests])
  lines = {}  # type: Dict[str, List[ResultLine]]
  for test in tests:
    assert test.name == name
    for line in test.lines:
      if not line.desc in lines:
        lines[line.desc] = [line]
      else:
        lines[line.desc].append(line)
  test_lines = []
  for _, line_list in lines.items():
    stat_line = LineFromList(line_list)
    if stat_line:
      test_lines.append(stat_line)
  return TestStats(name, avg, dev, cv, len(tests), test_lines)


class TargetStats(object):

  def __init__(self, name: str, samples: int, tests: List[TestStats]) -> None:
    self.name = name
    self.sample_num = samples
    self.tests = tests

  def ToLines(self) -> List[str]:
    lines = []
    if self.sample_num > 1:
      lines.append("{}: ".format(self.name))
    else:
      lines.append("{}: with only one sample".format(self.name))
    for test in self.tests:
      for line in test.ToLines():
        lines.append("  {}".format(line))
    return lines

  def __format__(self, format_spec):
    return "\n".join(self.ToLines())


def TargetFromList(results: List[TargetResult]) -> TargetStats:
  name = results[0].name
  sample_num = len(results)
  tests = {}  # type: Dict[str, List[TestResult]]
  for result in results:
    assert result.name == name
    # This groups tests by name so that they can be considered independently,
    # so that in the event tests flake out, their average times can
    # still be accurately calculated
    for test in result.tests:
      if not test.name in tests.keys():
        tests[test.name] = [test]
      tests[test.name].append(test)
  test_stats = [TestFromList(test_list) for _, test_list in tests.items()]
  return TargetStats(name, sample_num, test_stats)


def GenStats(corpus: List[float]) -> Tuple[float, float, float]:
  avg = sum(corpus) / len(corpus)
  adjusted_sum = 0.0
  for item in corpus:
    adjusted = item - avg
    adjusted_sum += adjusted * adjusted

  dev = math.sqrt(adjusted_sum / len(corpus))
  cv = dev / avg
  return avg, dev, cv


def DirectoryStats(directory: str) -> List[TargetStats]:
  resultMap = {}  # type: Dict[str, List[TargetResult]]
  for file in os.listdir(directory):
    results = ReadTargetFromJson("{}/{}".format(directory, file))
    if not results.name in resultMap.keys():
      resultMap[results.name] = [results]
    else:
      resultMap[results.name].append(results)

  targets = []
  for _, resultList in resultMap.items():
    targets.append(TargetFromList(resultList))
  return targets


def CompareTargets(linux: TargetStats, fuchsia: TargetStats) -> Dict[str, Any]:
  """Compare takes a corpus of statistics from both Fuchsia and Linux, and then
  lines up the values, compares them to each other, and writes them into a
  dictionary that can be JSONified.
  """
  assert linux.name == fuchsia.name
  paired_tests = ZipListsByPredicate(linux.tests, fuchsia.tests,
                                     lambda test: test.name)

  paired_tests = MapDictValues(paired_tests, CompareTests)
  return {"name": linux.name, "tests": paired_tests}


def CompareTests(linux: TestStats, fuchsia: TestStats) -> Dict[str, Any]:
  assert linux != None or fuchsia != None
  if linux != None and fuchsia != None:
    assert linux.name == fuchsia.name
  paired_lines = ZipListsByPredicate(linux.lines, fuchsia.lines,
                                     lambda line: line.desc)
  paired_lines = MapDictValues(paired_lines, CompareLines)
  result = {"lines": paired_lines, "unit": "ms"}  # type: Dict[str, Any]

  if linux:
    result["name"] = linux.name
    result["linux_avg"] = linux.time_avg
    result["linux_dev"] = linux.time_dev
    result["linux_cv"] = linux.cv

  if fuchsia == None:
    logging.warning("Fuchsia is missing test case {}".format(linux.name))
  else:
    result["name"] = fuchsia.name
    result["fuchsia_avg"] = fuchsia.time_avg
    result["fuchsia_dev"] = fuchsia.time_dev
    result["fuchsia_cv"] = fuchsia.cv


def CompareLines(linux: LineStats, fuchsia: LineStats) -> Dict[str, Any]:
  """CompareLines wraps two LineStats objects up as a JSON-dumpable dict, with
  missing values written as -1 (which specifically doesn't make sense for time
  elapsed measurements). It also logs a warning every time a line is given which
  can't be matched up. If both lines passed are None, or their units or
  descriptions are not the same(which should never happen) this function fails.
  """
  assert linux != None or fuchsia != None
  result = {}  # type: Dict[str, Any]
  if linux != None and fuchsia != None:
    assert linux.desc == fuchsia.desc
    assert linux.unit == fuchsia.unit

  if linux:
    result["desc"] = linux.desc
    result["unit"] = linux.unit
    result["linux_avg"] = linux.time_avg
    result["linux_dev"] = linux.time_dev
    result["linux_cv"] = linux.cv

  if fuchsia == None:
    logging.warning("Fuchsia is missing test line {}".format(linux.desc))
  else:
    result["desc"] = fuchsia.desc
    result["unit"] = fuchsia.unit
    result["fuchsia_avg"] = fuchsia.time_avg
    result["fuchsia_dev"] = fuchsia.time_dev
    result["fuchsia_cv"] = fuchsia.cv

  return result


T = TypeVar("T")
R = TypeVar("R")


def ZipListsByPredicate(left: List[T], right: List[T],
                        pred: Callable[[T], R]) -> Dict[R, Tuple[T, T]]:
  """This function takes two lists, and a predicate. The predicate is applied to
  the values in both lists to obtain a keying value from them. Each item is then
  inserted into the returned dictionary using the obtained key. Finally, after
  all items have been added to the dict, any items that do not have a pair are
  discarded after warning the user, and the new dictionary is returned. The
  predicate should not map multiple values from one list to the same key.
  """
  paired_items = {}  # type: Dict [R, Tuple[T, T]]
  for item in left:
    key = pred(item)
    # the first list shouldn't cause any key collisions
    assert key not in paired_items.keys()
    paired_items[key] = item, None

  for item in right:
    key = pred(item)
    if key in paired_items.keys():
      # elem 1 of the tuple is always None if the key exists in the map
      prev, _ = paired_items[key]
      paired_items[key] = prev, item
    else:
      paired_items[key] = None, item

  return paired_items


U = TypeVar("U")
V = TypeVar("V")


def MapDictValues(dct: Dict[T, Tuple[R, U]],
                  predicate: Callable[[R, U], V]) -> Dict[T, V]:
  """This function applies the predicate to all the values in the dictionary,
  returning a new dictionary with the new values.
  """
  out_dict = {}
  for key, val in dct.items():
    out_dict[key] = predicate(*val)
  return out_dict


def main():
  linux_avgs = DirectoryStats(target_spec.raw_linux_dir)
  fuchsia_avgs = DirectoryStats(target_spec.raw_fuchsia_dir)
  paired_targets = ZipListsByPredicate(linux_avgs, fuchsia_avgs,
                                       lambda target: target.name)
  for name, targets in paired_targets.items():
    comparison_dict = CompareTargets(*targets)
    with open("{}/{}.json".format(target_spec.results_dir, name),
              "w") as outfile:
      json.dump(comparison_dict, outfile, indent=2)


if __name__ == "__main__":
  sys.exit(main())
