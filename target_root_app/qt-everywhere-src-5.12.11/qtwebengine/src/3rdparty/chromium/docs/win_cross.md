# Cross-compiling Chrome/win

It's possible to build most parts of the codebase on a Linux or Mac host while
targeting Windows.  This document describes how to set that up, and current
restrictions.

What does *not* work:

* goma. Sorry. ([internal bug](http://b/64390790)) You can use the
  [jumbo build](jumbo.md) for faster build times.
* 64-bit renderer processes don't use V8 snapshots, slowing down their startup
  ([bug](https://crbug.com/803591))
* on Mac hosts, building a 32-bit chrome ([bug](https://crbug.com/794838))

All other targets build fine (including `chrome`, `browser_tests`, ...).

Uses of `.asm` files have been stubbed out.  As a result, some of Skia's
software rendering fast paths are not present in cross builds, Crashpad cannot
report crashes, and NaCl defaults to disabled and cannot be enabled in
cross builds ([.asm bug](https://crbug.com/762167)).

## .gclient setup

1. Tell gclient that you need Windows build dependencies by adding
   `target_os = ['win']` to the end of your `.gclient`.  (If you already
   have a `target_os` line in there, just add `'win'` to the list.) e.g.

       solutions = [
         {
           ...
         }
       ]
       target_os = ['android', 'win']

1. `gclient sync`, follow instructions on screen.

If you're at Google, this will automatically download the Windows SDK for you.
If this fails with an error:

    Please follow the instructions at
    https://chromium.googlesource.com/chromium/src/+/master/docs/windows_build_instructions.md

then you may need to re-authenticate via:

    cd path/to/chrome/src
    # Follow instructions, enter 0 as project id.
    download_from_google_storage --config

If you are not at Google, you can package your Windows SDK installation
into a zip file by running the following on a Windows machine:

    cd path/to/depot_tools/win_toolchain
    # customize the Windows SDK version numbers
    python package_from_installed.py 2017 -w 10.0.17134.0

These commands create a zip file named `<hash value>.zip`. Then, to use the
generated file in a Linux or Mac host, the following environment variables
need to be set:

    export DEPOT_TOOLS_WIN_TOOLCHAIN_BASE_URL=<path/to/sdk/zip/file>
    export GYP_MSVS_<toolchain hash>=<hash value>

`<toolchain hash>` is hardcoded in `src/build/vs_toolchain.py` and can be found by
setting `DEPOT_TOOLS_WIN_TOOLCHAIN_BASE_URL` and running `gclient sync`:

    gclient sync
    ...
    Running hooks:  17% (11/64) win_toolchain
    ________ running '/usr/bin/python src/build/vs_toolchain.py update --force' in <chromium dir>
    Windows toolchain out of date or doesn't exist, updating (Pro)...
    current_hashes:
    desired_hash: <toolchain hash>

## GN setup

Add `target_os = "win"` to your args.gn.  Then just build, e.g.

    ninja -C out/gnwin base_unittests.exe

## Copying and running chrome

A convenient way to copy chrome over to a Windows box is to build the
`mini_installer` target.  Then, copy just `mini_installer.exe` over
to the Windows box and run it to install the chrome you just built.

## Running tests on swarming

You can run the Windows binaries you built on swarming, like so:

    tools/run-swarmed.py -C out/gnwin -t base_unittests [ --gtest_filter=... ]

See the contents of run-swarmed.py for how to do this manually.

The
[linux-win_cross-rel](https://ci.chromium.org/buildbot/chromium.clang/linux-win_cross-rel/)
buildbot does 64-bit release cross builds, and also runs tests. You can look at
it to get an idea of which tests pass in the cross build.
