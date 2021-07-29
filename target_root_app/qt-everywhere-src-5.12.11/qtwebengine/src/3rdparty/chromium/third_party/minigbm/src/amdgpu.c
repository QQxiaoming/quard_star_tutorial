/*
 * Copyright 2016 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifdef DRV_AMDGPU
#include <amdgpu.h>
#include <amdgpu_drm.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <xf86drm.h>

#include "dri.h"
#include "drv_priv.h"
#include "helpers.h"
#include "util.h"

#ifdef __ANDROID__
#define DRI_PATH "/vendor/lib/dri/radeonsi_dri.so"
#else
#define DRI_PATH "/usr/lib64/dri/radeonsi_dri.so"
#endif

#define TILE_TYPE_LINEAR 0
/* DRI backend decides tiling in this case. */
#define TILE_TYPE_DRI 1

struct amdgpu_priv {
	struct dri_driver dri;
	int drm_version;
};

const static uint32_t render_target_formats[] = { DRM_FORMAT_ABGR8888, DRM_FORMAT_ARGB8888,
						  DRM_FORMAT_RGB565, DRM_FORMAT_XBGR8888,
						  DRM_FORMAT_XRGB8888 };

const static uint32_t texture_source_formats[] = { DRM_FORMAT_GR88, DRM_FORMAT_R8, DRM_FORMAT_NV21,
						   DRM_FORMAT_NV12, DRM_FORMAT_YVU420_ANDROID };

static int amdgpu_init(struct driver *drv)
{
	struct amdgpu_priv *priv;
	drmVersionPtr drm_version;
	struct format_metadata metadata;
	uint64_t use_flags = BO_USE_RENDER_MASK;

	priv = calloc(1, sizeof(struct amdgpu_priv));
	if (!priv)
		return -ENOMEM;

	drm_version = drmGetVersion(drv_get_fd(drv));
	if (!drm_version) {
		free(priv);
		return -ENODEV;
	}

	priv->drm_version = drm_version->version_minor;
	drmFreeVersion(drm_version);

	drv->priv = priv;

	if (dri_init(drv, DRI_PATH, "radeonsi")) {
		free(priv);
		drv->priv = NULL;
		return -ENODEV;
	}

	metadata.tiling = TILE_TYPE_LINEAR;
	metadata.priority = 1;
	metadata.modifier = DRM_FORMAT_MOD_LINEAR;

	drv_add_combinations(drv, render_target_formats, ARRAY_SIZE(render_target_formats),
			     &metadata, use_flags);

	drv_add_combinations(drv, texture_source_formats, ARRAY_SIZE(texture_source_formats),
			     &metadata, BO_USE_TEXTURE_MASK);

	/* Linear formats supported by display. */
	drv_modify_combination(drv, DRM_FORMAT_ARGB8888, &metadata, BO_USE_CURSOR | BO_USE_SCANOUT);
	drv_modify_combination(drv, DRM_FORMAT_XRGB8888, &metadata, BO_USE_CURSOR | BO_USE_SCANOUT);
	drv_modify_combination(drv, DRM_FORMAT_XBGR8888, &metadata, BO_USE_SCANOUT);

	/* YUV formats for camera and display. */
	drv_modify_combination(drv, DRM_FORMAT_NV12, &metadata,
			       BO_USE_CAMERA_READ | BO_USE_CAMERA_WRITE | BO_USE_SCANOUT);

	drv_modify_combination(drv, DRM_FORMAT_NV21, &metadata, BO_USE_SCANOUT);

	/*
	 * R8 format is used for Android's HAL_PIXEL_FORMAT_BLOB and is used for JPEG snapshots
	 * from camera.
	 */
	drv_modify_combination(drv, DRM_FORMAT_R8, &metadata,
			       BO_USE_CAMERA_READ | BO_USE_CAMERA_WRITE);

	/*
	 * The following formats will be allocated by the DRI backend and may be potentially tiled.
	 * Since format modifier support hasn't been implemented fully yet, it's not
	 * possible to enumerate the different types of buffers (like i915 can).
	 */
	use_flags &= ~BO_USE_RENDERSCRIPT;
	use_flags &= ~BO_USE_SW_WRITE_OFTEN;
	use_flags &= ~BO_USE_SW_READ_OFTEN;
	use_flags &= ~BO_USE_LINEAR;

	metadata.tiling = TILE_TYPE_DRI;
	metadata.priority = 2;

	drv_add_combinations(drv, render_target_formats, ARRAY_SIZE(render_target_formats),
			     &metadata, use_flags);

	/* Potentially tiled formats supported by display. */
	drv_modify_combination(drv, DRM_FORMAT_ARGB8888, &metadata, BO_USE_CURSOR | BO_USE_SCANOUT);
	drv_modify_combination(drv, DRM_FORMAT_XRGB8888, &metadata, BO_USE_CURSOR | BO_USE_SCANOUT);
	drv_modify_combination(drv, DRM_FORMAT_XBGR8888, &metadata, BO_USE_SCANOUT);
	return 0;
}

static void amdgpu_close(struct driver *drv)
{
	dri_close(drv);
	free(drv->priv);
	drv->priv = NULL;
}

static int amdgpu_create_bo(struct bo *bo, uint32_t width, uint32_t height, uint32_t format,
			    uint64_t use_flags)
{
	int ret;
	uint32_t plane, stride;
	struct combination *combo;
	union drm_amdgpu_gem_create gem_create;
	struct amdgpu_priv *priv = bo->drv->priv;

	combo = drv_get_combination(bo->drv, format, use_flags);
	if (!combo)
		return -EINVAL;

	if (combo->metadata.tiling == TILE_TYPE_DRI)
		return dri_bo_create(bo, width, height, format, use_flags);

	stride = drv_stride_from_format(format, width, 0);
	if (format == DRM_FORMAT_YVU420_ANDROID)
		stride = ALIGN(stride, 128);
	else
		stride = ALIGN(stride, 64);

	drv_bo_from_format(bo, stride, height, format);

	memset(&gem_create, 0, sizeof(gem_create));
	gem_create.in.bo_size = bo->total_size;
	gem_create.in.alignment = 256;
	gem_create.in.domain_flags = 0;

	if (use_flags & (BO_USE_LINEAR | BO_USE_SW))
		gem_create.in.domain_flags |= AMDGPU_GEM_CREATE_CPU_ACCESS_REQUIRED;

	gem_create.in.domains = AMDGPU_GEM_DOMAIN_GTT;
	if (!(use_flags & (BO_USE_SW_READ_OFTEN | BO_USE_SCANOUT)))
		gem_create.in.domain_flags |= AMDGPU_GEM_CREATE_CPU_GTT_USWC;

	/* If drm_version >= 21 everything exposes explicit synchronization primitives
	   and chromeos/arc++ will use them. Disable implicit synchronization. */
	if (priv->drm_version >= 21) {
		gem_create.in.domain_flags |= AMDGPU_GEM_CREATE_EXPLICIT_SYNC;
	}

	/* Allocate the buffer with the preferred heap. */
	ret = drmCommandWriteRead(drv_get_fd(bo->drv), DRM_AMDGPU_GEM_CREATE, &gem_create,
				  sizeof(gem_create));
	if (ret < 0)
		return ret;

	for (plane = 0; plane < bo->num_planes; plane++)
		bo->handles[plane].u32 = gem_create.out.handle;

	return 0;
}

static int amdgpu_import_bo(struct bo *bo, struct drv_import_fd_data *data)
{
	struct combination *combo;
	combo = drv_get_combination(bo->drv, data->format, data->use_flags);
	if (!combo)
		return -EINVAL;

	if (combo->metadata.tiling == TILE_TYPE_DRI)
		return dri_bo_import(bo, data);
	else
		return drv_prime_bo_import(bo, data);
}

static int amdgpu_destroy_bo(struct bo *bo)
{
	if (bo->priv)
		return dri_bo_destroy(bo);
	else
		return drv_gem_bo_destroy(bo);
}

static void *amdgpu_map_bo(struct bo *bo, struct vma *vma, size_t plane, uint32_t map_flags)
{
	int ret;
	union drm_amdgpu_gem_mmap gem_map;

	if (bo->priv)
		return dri_bo_map(bo, vma, plane, map_flags);

	memset(&gem_map, 0, sizeof(gem_map));
	gem_map.in.handle = bo->handles[plane].u32;

	ret = drmIoctl(bo->drv->fd, DRM_IOCTL_AMDGPU_GEM_MMAP, &gem_map);
	if (ret) {
		drv_log("DRM_IOCTL_AMDGPU_GEM_MMAP failed\n");
		return MAP_FAILED;
	}

	vma->length = bo->total_size;

	return mmap(0, bo->total_size, drv_get_prot(map_flags), MAP_SHARED, bo->drv->fd,
		    gem_map.out.addr_ptr);
}

static int amdgpu_unmap_bo(struct bo *bo, struct vma *vma)
{
	if (bo->priv)
		return dri_bo_unmap(bo, vma);
	else
		return munmap(vma->addr, vma->length);
}

static uint32_t amdgpu_resolve_format(uint32_t format, uint64_t use_flags)
{
	switch (format) {
	case DRM_FORMAT_FLEX_IMPLEMENTATION_DEFINED:
		/* Camera subsystem requires NV12. */
		if (use_flags & (BO_USE_CAMERA_READ | BO_USE_CAMERA_WRITE))
			return DRM_FORMAT_NV12;
		/*HACK: See b/28671744 */
		return DRM_FORMAT_XBGR8888;
	case DRM_FORMAT_FLEX_YCbCr_420_888:
		return DRM_FORMAT_NV12;
	default:
		return format;
	}
}

const struct backend backend_amdgpu = {
	.name = "amdgpu",
	.init = amdgpu_init,
	.close = amdgpu_close,
	.bo_create = amdgpu_create_bo,
	.bo_destroy = amdgpu_destroy_bo,
	.bo_import = amdgpu_import_bo,
	.bo_map = amdgpu_map_bo,
	.bo_unmap = amdgpu_unmap_bo,
	.resolve_format = amdgpu_resolve_format,
};

#endif
