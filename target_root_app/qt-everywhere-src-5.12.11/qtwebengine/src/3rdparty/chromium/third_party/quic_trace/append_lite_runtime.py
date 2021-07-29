#!/usr/bin/env python
# Copyright (c) 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Appends the LITE_RUNTIME annotation to a proto2 protocol buffer file.
"""

import sys

if len(sys.argv) != 3:
  sys.stderr.write("Usage: append_lite_runtime.py in.proto out.proto\n")
  sys.exit(1)

with open(sys.argv[1], "rb") as source:
  with open(sys.argv[2], "wb") as output:
    for line in source:
      output.write(line)
      if line.strip() == 'syntax = "proto2";':
        output.write("option optimize_for = LITE_RUNTIME;\n")
