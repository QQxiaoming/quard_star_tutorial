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
#include <video.h>
#include <version.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_VIDEO
int video_show_board_info(void) 
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device(UCLASS_VIDEO, 0, &dev);
	if (ret)
		return ret;
	video_clear(dev);
	ret = video_bmp_display(dev, (ulong)video_get_u_boot_logo(), 240, 160, true);
	if (ret)
		return ret;

	return 0;
}
#endif

phys_size_t get_effective_memsize(void)
{
	return 64*1024*1024;
}

int board_init(void)
{
	/*
	 * Make sure virtio bus is enumerated so that peripherals
	 * on the virtio bus can be discovered by their drivers
	 */
	virtio_init();

	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#ifdef CONFIG_VIDEO
	video_show_board_info();
#endif
	return 0;
}
#endif
