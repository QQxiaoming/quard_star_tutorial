// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <dm.h>
#include <env.h>
#include <fdtdec.h>
#include <image.h>
#include <log.h>
#include <init.h>
#include <virtio_types.h>
#include <virtio.h>

int board_init(void)
{
	/*
	 * Make sure virtio bus is enumerated so that peripherals
	 * on the virtio bus can be discovered by their drivers
	 */
	virtio_init();

	return 0;
}

int board_late_init(void)
{
	return 0;
}

phys_size_t get_effective_memsize(void)
{
	return 64*1024*1024;
}
