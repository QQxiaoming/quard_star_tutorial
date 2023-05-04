#
# SPDX-License-Identifier: BSD-2-Clause
#

# Compiler flags
platform-cppflags-y =
platform-cflags-y =
platform-asflags-y =
platform-ldflags-y =

# Objects to build
platform-objs-y += platform.o

# Blobs to build
FW_JUMP=y
FW_TEXT_START=0xBFF80000
FW_JUMP_ADDR=0x0
