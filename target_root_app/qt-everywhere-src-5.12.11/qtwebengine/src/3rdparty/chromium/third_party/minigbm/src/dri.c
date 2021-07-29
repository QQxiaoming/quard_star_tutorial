/*
 * Copyright 2017 Advanced Micro Devices. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifdef DRV_AMDGPU

#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <xf86drm.h>

#include "dri.h"
#include "drv_priv.h"
#include "helpers.h"
#include "util.h"

static const struct {
	uint32_t drm_format;
	int dri_image_format;
} drm_to_dri_image_formats[] = {
	{ DRM_FORMAT_R8, __DRI_IMAGE_FORMAT_R8 },
	{ DRM_FORMAT_GR88, __DRI_IMAGE_FORMAT_GR88 },
	{ DRM_FORMAT_RGB565, __DRI_IMAGE_FORMAT_RGB565 },
	{ DRM_FORMAT_XRGB8888, __DRI_IMAGE_FORMAT_XRGB8888 },
	{ DRM_FORMAT_ARGB8888, __DRI_IMAGE_FORMAT_ARGB8888 },
	{ DRM_FORMAT_XBGR8888, __DRI_IMAGE_FORMAT_XBGR8888 },
	{ DRM_FORMAT_ABGR8888, __DRI_IMAGE_FORMAT_ABGR8888 },
	{ DRM_FORMAT_XRGB2101010, __DRI_IMAGE_FORMAT_XRGB2101010 },
	{ DRM_FORMAT_ARGB2101010, __DRI_IMAGE_FORMAT_ARGB2101010 },
};

static int drm_format_to_dri_format(uint32_t drm_format)
{
	uint32_t i;
	for (i = 0; i < ARRAY_SIZE(drm_to_dri_image_formats); i++) {
		if (drm_to_dri_image_formats[i].drm_format == drm_format)
			return drm_to_dri_image_formats[i].dri_image_format;
	}

	return 0;
}

static bool lookup_extension(const __DRIextension *const *extensions, const char *name,
			     int min_version, const __DRIextension **dst)
{
	while (*extensions) {
		if ((*extensions)->name && !strcmp((*extensions)->name, name) &&
		    (*extensions)->version >= min_version) {
			*dst = *extensions;
			return true;
		}

		extensions++;
	}

	return false;
}

/*
 * The DRI GEM namespace may be different from the minigbm's driver GEM namespace. We have
 * to import into minigbm.
 */
static int import_into_minigbm(struct dri_driver *dri, struct bo *bo)
{
	uint32_t handle;
	int prime_fd, ret;

	if (!dri->image_extension->queryImage(bo->priv, __DRI_IMAGE_ATTRIB_FD, &prime_fd))
		return -errno;

	ret = drmPrimeFDToHandle(bo->drv->fd, prime_fd, &handle);
	if (ret) {
		drv_log("drmPrimeFDToHandle failed with %s\n", strerror(errno));
		return ret;
	}

	bo->handles[0].u32 = handle;
	close(prime_fd);
	return 0;
}

/*
 * The caller is responsible for setting drv->priv to a structure that derives from dri_driver.
 */
int dri_init(struct driver *drv, const char *dri_so_path, const char *driver_suffix)
{
	char fname[128];
	const __DRIextension **(*get_extensions)();
	const __DRIextension *loader_extensions[] = { NULL };

	struct dri_driver *dri = drv->priv;
	dri->driver_handle = dlopen(dri_so_path, RTLD_NOW | RTLD_GLOBAL);
	if (!dri->driver_handle)
		return -ENODEV;

	snprintf(fname, sizeof(fname), __DRI_DRIVER_GET_EXTENSIONS "_%s", driver_suffix);
	get_extensions = dlsym(dri->driver_handle, fname);
	if (!get_extensions)
		goto free_handle;

	dri->extensions = get_extensions();
	if (!dri->extensions)
		goto free_handle;

	if (!lookup_extension(dri->extensions, __DRI_CORE, 2,
			      (const __DRIextension **)&dri->core_extension))
		goto free_handle;

	/* Version 4 for createNewScreen2 */
	if (!lookup_extension(dri->extensions, __DRI_DRI2, 4,
			      (const __DRIextension **)&dri->dri2_extension))
		goto free_handle;

	dri->device = dri->dri2_extension->createNewScreen2(0, drv_get_fd(drv), loader_extensions,
							    dri->extensions, &dri->configs, NULL);
	if (!dri->device)
		goto free_handle;

	dri->context =
	    dri->dri2_extension->createNewContext(dri->device, *dri->configs, NULL, NULL);

	if (!dri->context)
		goto free_screen;

	if (!lookup_extension(dri->core_extension->getExtensions(dri->device), __DRI_IMAGE, 12,
			      (const __DRIextension **)&dri->image_extension))
		goto free_context;

	if (!lookup_extension(dri->core_extension->getExtensions(dri->device), __DRI2_FLUSH, 4,
			      (const __DRIextension **)&dri->flush_extension))
		goto free_context;

	return 0;

free_context:
	dri->core_extension->destroyContext(dri->context);
free_screen:
	dri->core_extension->destroyScreen(dri->device);
free_handle:
	dlclose(dri->driver_handle);
	dri->driver_handle = NULL;
	return -ENODEV;
}

/*
 * The caller is responsible for freeing drv->priv.
 */
void dri_close(struct driver *drv)
{
	struct dri_driver *dri = drv->priv;

	dri->core_extension->destroyContext(dri->context);
	dri->core_extension->destroyScreen(dri->device);
	dlclose(dri->driver_handle);
	dri->driver_handle = NULL;
}

int dri_bo_create(struct bo *bo, uint32_t width, uint32_t height, uint32_t format,
		  uint64_t use_flags)
{
	unsigned int dri_use;
	int ret, dri_format, stride, offset;
	struct dri_driver *dri = bo->drv->priv;

	assert(bo->num_planes == 1);
	dri_format = drm_format_to_dri_format(format);

	/* Gallium drivers require shared to get the handle and stride. */
	dri_use = __DRI_IMAGE_USE_SHARE;
	if (use_flags & BO_USE_SCANOUT)
		dri_use |= __DRI_IMAGE_USE_SCANOUT;
	if (use_flags & BO_USE_CURSOR)
		dri_use |= __DRI_IMAGE_USE_CURSOR;
	if (use_flags & BO_USE_LINEAR)
		dri_use |= __DRI_IMAGE_USE_LINEAR;

	bo->priv = dri->image_extension->createImage(dri->device, width, height, dri_format,
						     dri_use, NULL);
	if (!bo->priv) {
		ret = -errno;
		return ret;
	}

	ret = import_into_minigbm(dri, bo);
	if (ret)
		goto free_image;

	if (!dri->image_extension->queryImage(bo->priv, __DRI_IMAGE_ATTRIB_STRIDE, &stride)) {
		ret = -errno;
		goto free_image;
	}

	if (!dri->image_extension->queryImage(bo->priv, __DRI_IMAGE_ATTRIB_OFFSET, &offset)) {
		ret = -errno;
		goto free_image;
	}

	bo->strides[0] = stride;
	bo->sizes[0] = stride * height;
	bo->offsets[0] = offset;
	bo->total_size = offset + bo->sizes[0];
	return 0;

free_image:
	dri->image_extension->destroyImage(bo->priv);
	return ret;
}

int dri_bo_import(struct bo *bo, struct drv_import_fd_data *data)
{
	int ret;
	struct dri_driver *dri = bo->drv->priv;

	assert(bo->num_planes == 1);

	// clang-format off
	bo->priv = dri->image_extension->createImageFromFds(dri->device, data->width, data->height,
							    data->format, data->fds, bo->num_planes,
							    (int *)data->strides,
							    (int *)data->offsets, NULL);
	// clang-format on
	if (!bo->priv)
		return -errno;

	ret = import_into_minigbm(dri, bo);
	if (ret) {
		dri->image_extension->destroyImage(bo->priv);
		return ret;
	}

	return 0;
}

int dri_bo_destroy(struct bo *bo)
{
	struct dri_driver *dri = bo->drv->priv;

	assert(bo->priv);
	dri->image_extension->destroyImage(bo->priv);
	bo->priv = NULL;
	return 0;
}

/*
 * Map an image plane.
 *
 * This relies on the underlying driver to do a decompressing and/or de-tiling
 * blit if necessary,
 *
 * This function itself is not thread-safe; we rely on the fact that the caller
 * locks a per-driver mutex.
 */
void *dri_bo_map(struct bo *bo, struct vma *vma, size_t plane, uint32_t map_flags)
{
	struct dri_driver *dri = bo->drv->priv;

	/* GBM flags and DRI flags are the same. */
	vma->addr =
	    dri->image_extension->mapImage(dri->context, bo->priv, 0, 0, bo->width, bo->height,
					   map_flags, (int *)&vma->map_strides[plane], &vma->priv);
	if (!vma->addr)
		return MAP_FAILED;

	return vma->addr;
}

int dri_bo_unmap(struct bo *bo, struct vma *vma)
{
	struct dri_driver *dri = bo->drv->priv;

	assert(vma->priv);
	dri->image_extension->unmapImage(dri->context, bo->priv, vma->priv);

	/*
	 * From gbm_dri.c in Mesa:
	 *
	 * "Not all DRI drivers use direct maps. They may queue up DMA operations
	 *  on the mapping context. Since there is no explicit gbm flush mechanism,
	 *  we need to flush here."
	 */

	dri->flush_extension->flush_with_flags(dri->context, NULL, __DRI2_FLUSH_CONTEXT, 0);
	return 0;
}

#endif
