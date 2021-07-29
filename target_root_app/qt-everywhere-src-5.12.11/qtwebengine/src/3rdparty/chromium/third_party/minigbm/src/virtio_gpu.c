/*
 * Copyright 2017 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <virtgpu_drm.h>
#include <xf86drm.h>

#include "drv_priv.h"
#include "helpers.h"
#include "util.h"
#include "virgl_hw.h"

#ifndef PAGE_SIZE
#define PAGE_SIZE 0x1000
#endif
#define PIPE_TEXTURE_2D 2

#define MESA_LLVMPIPE_TILE_ORDER 6
#define MESA_LLVMPIPE_TILE_SIZE (1 << MESA_LLVMPIPE_TILE_ORDER)

static const uint32_t render_target_formats[] = { DRM_FORMAT_ABGR8888, DRM_FORMAT_ARGB8888,
						  DRM_FORMAT_RGB565, DRM_FORMAT_XBGR8888,
						  DRM_FORMAT_XRGB8888 };

static const uint32_t dumb_texture_source_formats[] = { DRM_FORMAT_R8, DRM_FORMAT_YVU420,
							DRM_FORMAT_YVU420_ANDROID };

static const uint32_t texture_source_formats[] = { DRM_FORMAT_R8, DRM_FORMAT_RG88 };

struct virtio_gpu_priv {
	int has_3d;
};

static uint32_t translate_format(uint32_t drm_fourcc, uint32_t plane)
{
	switch (drm_fourcc) {
	case DRM_FORMAT_XRGB8888:
		return VIRGL_FORMAT_B8G8R8X8_UNORM;
	case DRM_FORMAT_ARGB8888:
		return VIRGL_FORMAT_B8G8R8A8_UNORM;
	case DRM_FORMAT_XBGR8888:
		return VIRGL_FORMAT_R8G8B8X8_UNORM;
	case DRM_FORMAT_ABGR8888:
		return VIRGL_FORMAT_R8G8B8A8_UNORM;
	case DRM_FORMAT_RGB565:
		return VIRGL_FORMAT_B5G6R5_UNORM;
	case DRM_FORMAT_R8:
		return VIRGL_FORMAT_R8_UNORM;
	case DRM_FORMAT_RG88:
		return VIRGL_FORMAT_R8G8_UNORM;
	default:
		return 0;
	}
}

static int virtio_dumb_bo_create(struct bo *bo, uint32_t width, uint32_t height, uint32_t format,
				 uint64_t use_flags)
{
	width = ALIGN(width, MESA_LLVMPIPE_TILE_SIZE);
	height = ALIGN(height, MESA_LLVMPIPE_TILE_SIZE);

	/* HAL_PIXEL_FORMAT_YV12 requires that the buffer's height not be aligned. */
	if (bo->format == DRM_FORMAT_YVU420_ANDROID)
		height = bo->height;

	return drv_dumb_bo_create(bo, width, height, format, use_flags);
}

static int virtio_virgl_bo_create(struct bo *bo, uint32_t width, uint32_t height, uint32_t format,
				  uint64_t use_flags)
{
	int ret;
	ssize_t plane;
	ssize_t num_planes = drv_num_planes_from_format(format);
	uint32_t stride0;

	for (plane = 0; plane < num_planes; plane++) {
		uint32_t stride = drv_stride_from_format(format, width, plane);
		uint32_t size = drv_size_from_format(format, stride, height, plane);
		uint32_t res_format = translate_format(format, plane);
		struct drm_virtgpu_resource_create res_create;

		memset(&res_create, 0, sizeof(res_create));
		size = ALIGN(size, PAGE_SIZE);
		/*
		 * Setting the target is intended to ensure this resource gets bound as a 2D
		 * texture in the host renderer's GL state. All of these resource properties are
		 * sent unchanged by the kernel to the host, which in turn sends them unchanged to
		 * virglrenderer. When virglrenderer makes a resource, it will convert the target
		 * enum to the equivalent one in GL and then bind the resource to that target.
		 */
		res_create.target = PIPE_TEXTURE_2D;
		res_create.format = res_format;
		res_create.bind = VIRGL_BIND_RENDER_TARGET;
		res_create.width = width;
		res_create.height = height;
		res_create.depth = 1;
		res_create.array_size = 1;
		res_create.last_level = 0;
		res_create.nr_samples = 0;
		res_create.stride = stride;
		res_create.size = size;

		ret = drmIoctl(bo->drv->fd, DRM_IOCTL_VIRTGPU_RESOURCE_CREATE, &res_create);
		if (ret) {
			drv_log("DRM_IOCTL_VIRTGPU_RESOURCE_CREATE failed with %s\n",
				strerror(errno));
			goto fail;
		}

		bo->handles[plane].u32 = res_create.bo_handle;
	}

	stride0 = drv_stride_from_format(format, width, 0);
	drv_bo_from_format(bo, stride0, height, format);

	for (plane = 0; plane < num_planes; plane++)
		bo->offsets[plane] = 0;

	return 0;

fail:
	for (plane--; plane >= 0; plane--) {
		struct drm_gem_close gem_close;
		memset(&gem_close, 0, sizeof(gem_close));
		gem_close.handle = bo->handles[plane].u32;
		drmIoctl(bo->drv->fd, DRM_IOCTL_GEM_CLOSE, &gem_close);
	}

	return ret;
}

static void *virtio_virgl_bo_map(struct bo *bo, struct vma *vma, size_t plane, uint32_t map_flags)
{
	int ret;
	struct drm_virtgpu_map gem_map;

	memset(&gem_map, 0, sizeof(gem_map));
	gem_map.handle = bo->handles[0].u32;

	ret = drmIoctl(bo->drv->fd, DRM_IOCTL_VIRTGPU_MAP, &gem_map);
	if (ret) {
		drv_log("DRM_IOCTL_VIRTGPU_MAP failed with %s\n", strerror(errno));
		return MAP_FAILED;
	}

	vma->length = bo->total_size;
	return mmap(0, bo->total_size, drv_get_prot(map_flags), MAP_SHARED, bo->drv->fd,
		    gem_map.offset);
}

static int virtio_gpu_init(struct driver *drv)
{
	int ret;
	struct virtio_gpu_priv *priv;
	struct drm_virtgpu_getparam args;

	priv = calloc(1, sizeof(*priv));
	drv->priv = priv;

	memset(&args, 0, sizeof(args));
	args.param = VIRTGPU_PARAM_3D_FEATURES;
	args.value = (uint64_t)(uintptr_t)&priv->has_3d;
	ret = drmIoctl(drv->fd, DRM_IOCTL_VIRTGPU_GETPARAM, &args);
	if (ret) {
		drv_log("virtio 3D acceleration is not available\n");
		/* Be paranoid */
		priv->has_3d = 0;
	}

	drv_add_combinations(drv, render_target_formats, ARRAY_SIZE(render_target_formats),
			     &LINEAR_METADATA, BO_USE_RENDER_MASK);

	if (priv->has_3d)
		drv_add_combinations(drv, texture_source_formats,
				     ARRAY_SIZE(texture_source_formats), &LINEAR_METADATA,
				     BO_USE_TEXTURE_MASK);
	else
		drv_add_combinations(drv, dumb_texture_source_formats,
				     ARRAY_SIZE(dumb_texture_source_formats), &LINEAR_METADATA,
				     BO_USE_TEXTURE_MASK);

	return drv_modify_linear_combinations(drv);
}

static void virtio_gpu_close(struct driver *drv)
{
	free(drv->priv);
	drv->priv = NULL;
}

static int virtio_gpu_bo_create(struct bo *bo, uint32_t width, uint32_t height, uint32_t format,
				uint64_t use_flags)
{
	struct virtio_gpu_priv *priv = (struct virtio_gpu_priv *)bo->drv->priv;
	if (priv->has_3d)
		return virtio_virgl_bo_create(bo, width, height, format, use_flags);
	else
		return virtio_dumb_bo_create(bo, width, height, format, use_flags);
}

static int virtio_gpu_bo_destroy(struct bo *bo)
{
	struct virtio_gpu_priv *priv = (struct virtio_gpu_priv *)bo->drv->priv;
	if (priv->has_3d)
		return drv_gem_bo_destroy(bo);
	else
		return drv_dumb_bo_destroy(bo);
}

static void *virtio_gpu_bo_map(struct bo *bo, struct vma *vma, size_t plane, uint32_t map_flags)
{
	struct virtio_gpu_priv *priv = (struct virtio_gpu_priv *)bo->drv->priv;
	if (priv->has_3d)
		return virtio_virgl_bo_map(bo, vma, plane, map_flags);
	else
		return drv_dumb_bo_map(bo, vma, plane, map_flags);
}

static int virtio_gpu_bo_invalidate(struct bo *bo, struct mapping *mapping)
{
	int ret;
	struct drm_virtgpu_3d_transfer_from_host xfer;
	struct virtio_gpu_priv *priv = (struct virtio_gpu_priv *)bo->drv->priv;

	if (!priv->has_3d)
		return 0;

	memset(&xfer, 0, sizeof(xfer));
	xfer.bo_handle = mapping->vma->handle;
	xfer.box.x = mapping->rect.x;
	xfer.box.y = mapping->rect.y;
	xfer.box.w = mapping->rect.width;
	xfer.box.h = mapping->rect.height;
	xfer.box.d = 1;

	ret = drmIoctl(bo->drv->fd, DRM_IOCTL_VIRTGPU_TRANSFER_FROM_HOST, &xfer);
	if (ret) {
		drv_log("DRM_IOCTL_VIRTGPU_TRANSFER_FROM_HOST failed with %s\n", strerror(errno));
		return ret;
	}

	return 0;
}

static int virtio_gpu_bo_flush(struct bo *bo, struct mapping *mapping)
{
	int ret;
	struct drm_virtgpu_3d_transfer_to_host xfer;
	struct virtio_gpu_priv *priv = (struct virtio_gpu_priv *)bo->drv->priv;

	if (!priv->has_3d)
		return 0;

	if (!(mapping->vma->map_flags & BO_MAP_WRITE))
		return 0;

	memset(&xfer, 0, sizeof(xfer));
	xfer.bo_handle = mapping->vma->handle;
	xfer.box.x = mapping->rect.x;
	xfer.box.y = mapping->rect.y;
	xfer.box.w = mapping->rect.width;
	xfer.box.h = mapping->rect.height;
	xfer.box.d = 1;

	ret = drmIoctl(bo->drv->fd, DRM_IOCTL_VIRTGPU_TRANSFER_TO_HOST, &xfer);
	if (ret) {
		drv_log("DRM_IOCTL_VIRTGPU_TRANSFER_TO_HOST failed with %s\n", strerror(errno));
		return ret;
	}

	return 0;
}

static uint32_t virtio_gpu_resolve_format(uint32_t format, uint64_t use_flags)
{
	switch (format) {
	case DRM_FORMAT_FLEX_IMPLEMENTATION_DEFINED:
		/*HACK: See b/28671744 */
		return DRM_FORMAT_XBGR8888;
	case DRM_FORMAT_FLEX_YCbCr_420_888:
		return DRM_FORMAT_YVU420;
	default:
		return format;
	}
}

const struct backend backend_virtio_gpu = {
	.name = "virtio_gpu",
	.init = virtio_gpu_init,
	.close = virtio_gpu_close,
	.bo_create = virtio_gpu_bo_create,
	.bo_destroy = virtio_gpu_bo_destroy,
	.bo_import = drv_prime_bo_import,
	.bo_map = virtio_gpu_bo_map,
	.bo_unmap = drv_bo_munmap,
	.bo_invalidate = virtio_gpu_bo_invalidate,
	.bo_flush = virtio_gpu_bo_flush,
	.resolve_format = virtio_gpu_resolve_format,
};
