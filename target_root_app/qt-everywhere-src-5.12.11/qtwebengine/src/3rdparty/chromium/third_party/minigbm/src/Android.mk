# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

ifeq ($(strip $(BOARD_USES_MINIGBM)), true)

MINIGBM_GRALLOC_MK := $(call my-dir)/Android.gralloc.mk
LOCAL_PATH := $(call my-dir)
intel_drivers := i915 i965

MINIGBM_SRC := \
	amdgpu.c \
	drv.c \
	evdi.c \
	exynos.c \
	helpers_array.c \
	helpers.c \
	i915.c \
	marvell.c \
	mediatek.c \
	meson.c \
	msm.c \
	nouveau.c \
	radeon.c \
	rockchip.c \
	tegra.c \
	udl.c \
	vc4.c \
	vgem.c \
	virtio_gpu.c

MINIGBM_CPPFLAGS := -std=c++14
MINIGBM_CFLAGS := \
	-D_GNU_SOURCE=1 -D_FILE_OFFSET_BITS=64 \
	-Wall -Wsign-compare -Wpointer-arith \
	-Wcast-qual -Wcast-align \
	-Wno-unused-parameter

ifneq ($(filter $(intel_drivers), $(BOARD_GPU_DRIVERS)),)
MINIGBM_CPPFLAGS += -DDRV_I915
MINIGBM_CFLAGS += -DDRV_I915
LOCAL_SHARED_LIBRARIES += libdrm_intel
endif

ifneq ($(filter meson, $(BOARD_GPU_DRIVERS)),)
MINIGBM_CPPFLAGS += -DDRV_MESON
MINIGBM_CFLAGS += -DDRV_MESON
endif

include $(CLEAR_VARS)

SUBDIRS := cros_gralloc

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libdrm

LOCAL_SRC_FILES := $(MINIGBM_SRC)

include $(MINIGBM_GRALLOC_MK)

LOCAL_CFLAGS := $(MINIGBM_CFLAGS)
LOCAL_CPPFLAGS := $(MINIGBM_CPPFLAGS)

LOCAL_MODULE := gralloc.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_TAGS := optional
# The preferred path for vendor HALs is /vendor/lib/hw
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)
LOCAL_HEADER_LIBRARIES += \
	libhardware_headers libnativebase_headers libsystem_headers
LOCAL_SHARED_LIBRARIES += libnativewindow libsync liblog
LOCAL_STATIC_LIBRARIES += libarect
include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_STATIC_LIBRARIES := libdrm

LOCAL_SRC_FILES += $(MINIGBM_SRC) gbm.c gbm_helpers.c

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
LOCAL_CFLAGS := $(MINIGBM_CFLAGS)
LOCAL_CPPFLAGS := $(MINIGBM_CPPFLAGS)

LOCAL_MODULE := libminigbm
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

endif
