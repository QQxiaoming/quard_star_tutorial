/*
 * Copyright 2017 Advanced Micro Devices. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifdef DRV_AMDGPU

typedef int GLint;
typedef unsigned int GLuint;
typedef unsigned char GLboolean;

#include "GL/internal/dri_interface.h"
#include "drv.h"

struct dri_driver {
	void *driver_handle;
	__DRIscreen *device;
	__DRIcontext *context; /* Needed for map/unmap operations. */
	const __DRIextension **extensions;
	const __DRIcoreExtension *core_extension;
	const __DRIdri2Extension *dri2_extension;
	const __DRIimageExtension *image_extension;
	const __DRI2flushExtension *flush_extension;
	const __DRIconfig **configs;
};

int dri_init(struct driver *drv, const char *dri_so_path, const char *driver_suffix);
void dri_close(struct driver *drv);
int dri_bo_create(struct bo *bo, uint32_t width, uint32_t height, uint32_t format,
		  uint64_t use_flags);
int dri_bo_import(struct bo *bo, struct drv_import_fd_data *data);
int dri_bo_destroy(struct bo *bo);
void *dri_bo_map(struct bo *bo, struct vma *vma, size_t plane, uint32_t map_flags);
int dri_bo_unmap(struct bo *bo, struct vma *vma);

#endif
