#!/usr/bin/env bash
# Install 7z to be used from command line

set -ex

brew update
brew install --build-from-source p7zip
