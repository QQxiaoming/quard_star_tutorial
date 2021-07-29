#!/usr/bin/python
#
# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Set up everything for the roll.
#
# --setup: set up the host to do a roll.  Idempotent, but probably doesn't need
#          to be run more than once in a while.
# --test:  configure ffmpeg for the host machine, and try running media unit
#          tests and the ffmpeg regression tests.
# --build: build ffmpeg configs for all platforms, then generate gn config.
# --all:   do everything.  Right now, this is the same as "--test --build".

import getopt
import os
import sys

from robo_lib import log
import robo_lib
import robo_build
import robo_setup

def main(argv):
  robo_configuration = robo_lib.RoboConfiguration()
  robo_configuration.chdir_to_ffmpeg_home();

  parsed, remaining = getopt.getopt(argv, "",
          ["setup", "test", "build", "all"])

  for opt, arg in parsed:
    if opt == "--setup":
      robo_setup.InstallPrereqs(robo_configuration)
      robo_setup.EnsureToolchains(robo_configuration)
      robo_setup.EnsureASANDirWorks(robo_configuration)
    elif opt == "--test":
      robo_build.BuildAndImportFFmpegConfigForHost(robo_configuration)
      robo_build.RunTests(robo_configuration)
    elif opt == "--build":
      robo_build.BuildAndImportAllFFmpegConfigs(robo_configuration)
      # TODO: run check_merge.py
      # TODO: run find_patches.py
    elif opt == "--all":
      robo_build.BuildAndImportFFmpegConfigForHost(robo_configuration)
      robo_build.RunTests(robo_configuration)
      robo_build.BuildAndImportAllFFmpegConfigs(robo_configuration)
      # TODO: run some sanity checks to see if this might be okay to auto-roll.
    else:
      raise Exception("Unknown option '%s'" % opt);

if __name__ == "__main__":
  main(sys.argv[1:])
