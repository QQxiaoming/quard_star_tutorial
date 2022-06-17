// SPDX-License-Identifier: GPL-2.0+

#define LOG_CATEGORY UCLASS_VIDEO

#include <common.h>
#include <clk.h>
#include <display.h>
#include <dm.h>
#include <log.h>
#include <panel.h>
#include <reset.h>
#include <video.h>
#include <video_bridge.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>
#include <dm/device-internal.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <config.h>

#define VIDEO_PL111_MAX_XRES (1024)
#define VIDEO_PL111_MAX_YRES (768)
#define VIDEO_PL111_MAX_BPP  (24)

struct pl111_priv {
	void __iomem *regs;
	enum video_log2_bpp l2bpp;
};

/* PL111 main registers */
#define CLCD_TIM0		0x00000000
#define CLCD_TIM1		0x00000004
#define CLCD_TIM2		0x00000008
#define CLCD_TIM3		0x0000000c
#define CLCD_UBAS		0x00000010
#define CLCD_LBAS		0x00000014

#define CLCD_PL111_CNTL		0x00000018
#define CLCD_PL111_IENB		0x0000001c
#define CLCD_PL111_RIS		0x00000020
#define CLCD_PL111_MIS		0x00000024
#define CLCD_PL111_ICR		0x00000028
#define CLCD_PL111_UCUR		0x0000002c
#define CLCD_PL111_LCUR		0x00000030

#define CNTL_LCDEN		(1 << 0)
#define CNTL_LCDBPP1		(0 << 1)
#define CNTL_LCDBPP2		(1 << 1)
#define CNTL_LCDBPP4		(2 << 1)
#define CNTL_LCDBPP8		(3 << 1)
#define CNTL_LCDBPP16		(4 << 1)
#define CNTL_LCDBPP16_565	(6 << 1)
#define CNTL_LCDBPP16_444	(7 << 1)
#define CNTL_LCDBPP24		(5 << 1)
#define CNTL_LCDBW		(1 << 4)
#define CNTL_LCDTFT		(1 << 5)
#define CNTL_LCDMONO8		(1 << 6)
#define CNTL_LCDDUAL		(1 << 7)
#define CNTL_BGR		(1 << 8)
#define CNTL_BEBO		(1 << 9)
#define CNTL_BEPO		(1 << 10)
#define CNTL_LCDPWR		(1 << 11)
#define CNTL_LCDVCOMP(x)	((x) << 12)
#define CNTL_LDMAFIFOTIME	(1 << 15)
#define CNTL_WATERMARK		(1 << 16)

enum pl111_pix_fmt {
	PF_BPP1 = 0,
	PF_BPP2,
	PF_BPP4,
	PF_BPP8,
	PF_BPP16,
	PF_BPP24,
	PF_BPP16_565,
	PF_BPP12_444
};

static u32 pl111_get_pixel_format(enum video_log2_bpp l2bpp)
{
	enum pl111_pix_fmt pf;

	switch (l2bpp) {
	case VIDEO_BPP1:
		pf = PF_BPP1;
		break;
	case VIDEO_BPP2:
		pf = PF_BPP2;
		break;
	case VIDEO_BPP4:
		pf = PF_BPP4;
		break;
	case VIDEO_BPP8:
		pf = PF_BPP8;
		break;
	case VIDEO_BPP16:
		pf = PF_BPP16_565;
		break;
	case VIDEO_BPP32:
	default:
		log_warning("warning %dbpp not supported yet, %dbpp instead\n",
			    VNBITS(l2bpp), VNBITS(VIDEO_BPP16));
		pf = PF_BPP16_565;
		break;
	}

	log_debug("%d bpp -> ltdc pf %d\n", VNBITS(l2bpp), pf);

	return (u32)pf;
}

static void pl111_enable(struct pl111_priv *priv)
{
	size_t regs_addr = (size_t)priv->regs;
	setbits_le32(regs_addr + CLCD_PL111_CNTL, CNTL_LCDPWR);
}

static void pl111_set_mode(struct pl111_priv *priv,
				struct display_timing *timings, ulong fb_addr)
{
	size_t regs_addr = (size_t)priv->regs;
	u32 ppl, hsw, hfp, hbp;
	u32 lpp, vsw, vfp, vbp;
	u32 format;

	ppl = (timings->hactive.typ / 16) - 1;
	hsw = timings->hsync_len.typ;
	hfp = timings->hfront_porch.typ;
	hbp = timings->hback_porch.typ;

	lpp = timings->vactive.typ - 1;
	vsw = timings->vsync_len.typ;
	vfp = timings->vfront_porch.typ;
	vbp = timings->vback_porch.typ;

	writel((ppl << 2) | (hsw << 8) | (hfp << 16) | (hbp << 24),
	       regs_addr + CLCD_TIM0);
	writel(lpp | (vsw << 10) | (vfp << 16) | (vbp << 24),
	       regs_addr + CLCD_TIM1);

	format = pl111_get_pixel_format(priv->l2bpp);

	writel(CNTL_LCDEN | CNTL_LCDTFT | CNTL_BGR | (format << 1), regs_addr + CLCD_PL111_CNTL);

	writel((uint32_t)fb_addr, regs_addr + CLCD_UBAS);
}

static int pl111_probe(struct udevice *dev)
{
	struct video_uc_plat *uc_plat = dev_get_uclass_plat(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct pl111_priv *priv = dev_get_priv(dev);
	struct display_timing timings;
	struct clk pclk;
	struct reset_ctl rst;
	int ret;

	priv->regs = (void *)dev_read_addr(dev);
	if ((fdt_addr_t)priv->regs == FDT_ADDR_T_NONE) {
		dev_err(dev, "pl111 dt register address error\n");
		return -EINVAL;
	}

	ret = clk_get_by_index(dev, 0, &pclk);
	if (ret) {
		dev_err(dev, "peripheral clock get error %d\n", ret);
		return ret;
	}

	ret = clk_enable(&pclk);
	if (ret) {
		dev_err(dev, "peripheral clock enable error %d\n", ret);
		return ret;
	}

	ret = ofnode_decode_display_timing(dev_ofnode(dev),
						0, &timings);
	if (ret) {
		dev_err(dev, "decode display timing error %d\n", ret);
		return ret;
	}

	/* TODO Below parameters are hard-coded for the moment... */
	priv->l2bpp = VIDEO_BPP16;

	/* Configure & start LTDC */
	pl111_set_mode(priv, &timings, uc_plat->base);
	pl111_enable(priv);

	uc_priv->xsize = timings.hactive.typ;
	uc_priv->ysize = timings.vactive.typ;
	uc_priv->bpix = priv->l2bpp;

	video_set_flush_dcache(dev, true);

	return 0;
}

static int pl111_bind(struct udevice *dev)
{
	struct video_uc_plat *uc_plat = dev_get_uclass_plat(dev);

	uc_plat->size = VIDEO_PL111_MAX_XRES * VIDEO_PL111_MAX_YRES *
			(VIDEO_PL111_MAX_BPP >> 3);
	dev_dbg(dev, "frame buffer max size %d bytes\n", uc_plat->size);

	return 0;
}

static const struct udevice_id pl111_ids[] = {
	{ .compatible = "arm,pl111" },
	{ }
};

U_BOOT_DRIVER(pl111) = {
	.name			= "pl111_display",
	.id			= UCLASS_VIDEO,
	.of_match		= pl111_ids,
	.probe			= pl111_probe,
	.bind			= pl111_bind,
	.priv_auto	= sizeof(struct pl111_priv),
};
