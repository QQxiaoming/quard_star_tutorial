#!/usr/bin/python
#
# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Set up everything for the roll.

import os
from subprocess import call
import subprocess
from robo_lib import log

def InstallUbuntuPackage(robo_configuration, package):
  """Install |package|.

    Args:
      robo_configuration: current RoboConfiguration.
      package: package name.
  """

  log("Installing package %s" % package)
  if call(["sudo", "apt-get", "install", package]):
    raise Exception("Could not install %s" % package)

def InstallPrereqs(robo_configuration):
    """Install prereqs needed to build ffmpeg.

    Args:
      robo_configuration: current RoboConfiguration.
    """
    robo_configuration.chdir_to_chrome_src();

    # Check out data for ffmpeg regression tests.
    media_directory = os.path.join("media", "test", "data", "internal")
    if not os.path.exists(media_directory):
      log("Checking out media internal test data")
      if call(["git", "clone",
              "https://chrome-internal.googlesource.com/chrome/data/media",
              media_directory]):
        raise Exception(
                "Could not check out chrome media internal test data %s" %
                media_directory)

    if robo_configuration.host_operating_system() == "linux":
      InstallUbuntuPackage(robo_configuration, "yasm")
      InstallUbuntuPackage(robo_configuration, "gcc-aarch64-linux-gnu")
    else:
      raise Exception("I don't know how to install deps for host os %s" %
          robo_configuration.host_operating_system())

def EnsureASANDirWorks(robo_configuration):
    """Create the asan out dir and config for ninja builds.

    Args:
      robo_configuration: current RoboConfiguration.
    """

    directory_name = robo_configuration.absolute_asan_directory()
    if os.path.exists(directory_name):
      return

    # Dir doesn't exist, so make it and generate the gn files.  Note that we
    # have no idea what state the ffmpeg config is, but that's okay.  gn will
    # re-build it when we run ninja later (after importing the ffmpeg config)
    # if it's changed.

    log("Creating asan build dir %s" % directory_name)
    os.mkdir(directory_name)

    # TODO(liberato): ffmpeg_branding Chrome vs ChromeOS.  also add arch
    # flags, etc.  Step 28.iii, and elsewhere.
    opts = ("is_debug=false", "is_clang=true", "proprietary_codecs=true",
            "media_use_libvpx=true", "media_use_ffmpeg=true",
            'ffmpeg_branding="Chrome"',
            "use_goma=true", "is_asan=true", "dcheck_always_on=true",
            "enable_mse_mpeg2ts_stream_parser=true",
            "enable_hevc_demuxing=true")
    print opts
    with open(os.path.join(directory_name, "args.gn"), "w") as f:
      for opt in opts:
        print opt
        f.write("%s\n" % opt)

    # Ask gn to generate build files.
    log("Running gn on %s" % directory_name)
    if call(["gn", "gen", robo_configuration.local_asan_directory()]):
      raise Exception("Unable to gn gen %s" %
              robo_configuration.local_asan_directory())

def EnsureGClientTargets(robo_configuration):
  """Make sure that we've got the right sdks if we're on a linux host."""
  if not robo_configuration.host_operating_system() == "linux":
    log("Not changing gclient target_os list on a non-linux host")
    return
  log("Checking gclient target_os list")
  gclient_filename = os.path.join(robo_configuration.chrome_src(), "..",
    ".gclient")
  # Ensure that we've got our target_os line
  for line in open(gclient_filename, "r"):
    if "target_os" in line:
      if (not "'android'" in line) or (not "'win'" in line):
        log("Missing 'android' and/or 'win' in target_os, which goes at the")
        log("end of .gclient, OUTSIDE of the solutions = [] section")
        log("Example line:")
        log("target_os = [ 'android', 'win' ]")
        raise Exception("Please add 'android' and 'win' to target_os in %s" %
            gclient_filename)
      break

  # Sync regardless of whether we changed the config.
  log("Running gclient sync")
  robo_configuration.chdir_to_chrome_src()
  if call(["gclient", "sync"]):
    raise Exception("gclient sync failed")

def FetchAdditionalWindowsBinaries(robo_configuration):
  """Download some additional binaries needed by ffmpeg.  gclient sync can
  sometimes remove these.  Re-run this if you're missing llvm-nm or llvm-ar."""
  robo_configuration.chdir_to_chrome_src()
  log("Downloading some additional compiler tools")
  if call(["tools/clang/scripts/download_objdump.py"]):
    raise Exception("download_objdump.py failed")

def FetchMacSDK(robo_configuration):
  """Download the 10.10 MacOSX sdk."""
  log("Installing Mac OSX sdk")
  robo_configuration.chdir_to_chrome_src()
  sdk_base="build/win_files/Xcode.app"
  if not os.path.exists(sdk_base):
    os.mkdirs(sdk_base)
  os.chdir(sdk_base)
  if call(
      "gsutil.py cat gs://chrome-mac-sdk/toolchain-8E2002-3.tgz | tar xzvf -",
      shell=True):
    raise Exception("Cannot download and extract Mac SDK")

def EnsureLLVMSymlinks(robo_configuration):
  """Create some symlinks to clang and friends, since that changes their
  behavior somewhat."""
  log("Creating symlinks to compiler tools if needed")
  os.chdir(os.path.join(robo_configuration.chrome_src(), "third_party", "llvm-build", "Release+Asserts", "bin"))
  def EnsureSymlink(source, link_name):
    if not os.path.exists(link_name):
      os.symlink(source, link_name)
  # For windows.
  EnsureSymlink("clang", "clang-cl")
  EnsureSymlink("clang", "clang++")
  EnsureSymlink("lld", "ld.lld")
  EnsureSymlink("lld", "lld-link")
  # For mac.
  EnsureSymlink("lld", "ld64.lld")

def EnsureSysroots(robo_configuration):
  """Install arm/arm64/mips/mips64 sysroots."""
  robo_configuration.chdir_to_chrome_src()
  for arch in ["arm", "arm64", "mips", "mips64el"]:
    if call(["build/linux/sysroot_scripts/install-sysroot.py",
             "--arch=" + arch]):
      raise Exception("Failed to install sysroot for " + arch);

def EnsureToolchains(robo_configuration):
  """Make sure that we have all the toolchains for cross-compilation"""
  EnsureGClientTargets(robo_configuration)
  FetchAdditionalWindowsBinaries(robo_configuration)
  FetchMacSDK(robo_configuration)
  EnsureLLVMSymlinks(robo_configuration)
  EnsureSysroots(robo_configuration)
