#!/usr/bin/python
#
# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Various functions to help build / test ffmpeg.

import os
from robo_lib import log
from subprocess import call
import subprocess

def ConfigureAndBuildFFmpeg(robo_configuration, platform, architecture):
  """Run FFmpeg's configure script, and build ffmpeg.

  Args:
    robo_configuration: RoboConfiguration.
    platform: platform name (e.g., "linux")
    architecture: (optional) arch name (e.g., "ia32").  If omitted, then we
          build all of them for |platform|.
  """
  log("Generating FFmpeg config and building for %s %s" %
          (platform, architecture))

  log("Starting FFmpeg build for %s %s" % (platform, architecture))
  robo_configuration.chdir_to_ffmpeg_home();
  command = ["./chromium/scripts/build_ffmpeg.py", platform]
  if architecture:
    command.append(architecture)
  if call(command):
      raise Exception("FFmpeg build failed for %s %s" %
              (platform, architecture))

def ImportFFmpegConfigsIntoChromium(robo_configuration):
  """Import all FFmpeg configs that have been built so far and build gn files.

  Args:
    robo_configuration: RoboConfiguration.
  """
  robo_configuration.chdir_to_ffmpeg_home();
  log("Copying FFmpeg configs")
  if call(["./chromium/scripts/copy_config.sh"]):
      raise Exception("FFmpeg copy_config.sh failed")
  log("Generating GN config for all ffmpeg versions")
  if call(["./chromium/scripts/generate_gn.py"]):
      raise Exception("FFmpeg generate_gn.sh failed")
  # TODO(liberato): deal with added / removed autorename files somehow.
  # Remember that we should only do that when we're building all configs.  If
  # we're building just for our host, then we expect that some files will be
  # missing, since they might apply to other platforms.

def BuildAndImportAllFFmpegConfigs(robo_configuration):
  """Build ffmpeg for all platforms that we can, and build the gn files.

  Args:
    robo_configuration: RoboConfiguration.
  """
  if robo_configuration.host_operating_system() == "linux":
    ConfigureAndBuildFFmpeg(robo_configuration, "all", None)
  else:
    raise Exception("I don't know how to build ffmpeg for host type %s" %
            robo_configuration.host_operating_system())

  # Now that we've built everything, import them and build the gn config.
  ImportFFmpegConfigsIntoChromium(robo_configuration)

# Build and import just the single ffmpeg version our host uses for testing.
def BuildAndImportFFmpegConfigForHost(robo_configuration):
  """Build and import FFmpeg for our host only.

  Build FFmpeg for our host, and create gn files for it.  This will probably
  produce autorename warnings which don't matter.

  This is useful for building local tests for the new ffmpeg.

  Args:
    robo_configuration: RoboConfiguration.
  """
  # TODO: optimize this out if we've done it already -- it's only needed if the
  # ffmpeg build config changes.  If you just change ffmpeg source files, then
  # just re-build chrome and it'll rebuild the library.
  ConfigureAndBuildFFmpeg(robo_configuration,
          robo_configuration.host_operating_system(),
          robo_configuration.host_architecture())

  # Note that this will import anything that you've built, but that's okay.
  ImportFFmpegConfigsIntoChromium(robo_configuration)

def BuildChromeTargetASAN(robo_configuration, target, platform, architecture):
  """Build a Chromium asan target.

  Args:
    robo_configuration: RoboConfiguration.
    target: chrome target to build (e.g., "media_unittests")
    platform: platform to build it for, which should probably be the host's.
    architecture: arch to build it for (e.g., "x64").
  """
  robo_configuration.chdir_to_chrome_src()
  if call(["ninja", "-j5000", "-C",
          robo_configuration.relative_asan_directory(), target]):
      raise Exception("Failed to build %s" % target)

def BuildAndRunChromeTargetASAN(robo_configuration, target, platform,
        architecture):
  """Build and run a Chromium asan target.

  Args:
    robo_configuration: RoboConfiguration.
    target: chrome target to build (e.g., "media_unittests")
    platform: platform to build it for, which should probably be the host's.
    architecture: arch to build it for (e.g., "x64").
  """
  log("Building and running %s" % target)
  BuildChromeTargetASAN(robo_configuration, target, platform, architecture)
  # TODO: we should be smarter about running things on android, for example.
  log("Running %s" % target)
  robo_configuration.chdir_to_chrome_src()
  if call([os.path.join(robo_configuration.absolute_asan_directory(), target)]):
    raise Exception("%s didn't complete successfully" % target)
  log("%s ran successfully" % target)

def RunTests(robo_configuration):
  """Build all tests and run them locally.

  This assumes that the FFmpeg config and gn files are up to date for the host.
  If not, then please run BuildAndImportFFmpegConfigForHost first.

  Args:
    robo_configuration: RoboConfiguration.
  """
  host_operating_system = robo_configuration.host_operating_system()
  host_architecture = robo_configuration.host_architecture()
  BuildAndRunChromeTargetASAN(robo_configuration, "media_unittests",
          host_operating_system, host_architecture)
  BuildAndRunChromeTargetASAN(robo_configuration, "ffmpeg_regression_tests",
          host_operating_system, host_architecture)
  # chrome works, if you want to do some manual testing.
  #  BuildAndRunChromeTargetASAN(robo_configuration, "chrome",
  #          host_operating_system, host_architecture)
