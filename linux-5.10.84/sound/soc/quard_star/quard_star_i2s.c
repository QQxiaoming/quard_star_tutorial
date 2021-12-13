// SPDX-License-Identifier: GPL-2.0-only
#include <linux/bitfield.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/regmap.h>
#include <linux/reset.h>
#include <linux/spinlock.h>

#include <sound/dmaengine_pcm.h>
#include <sound/pcm_params.h>

static void __iomem *i2s_base;

static irqreturn_t quard_star_i2s_isr(int irq, void *devid)
{
	pr_err("quard_star_i2s_isr\n");
	return IRQ_HANDLED;
}

static int quard_star_i2s_set_sysclk(struct snd_soc_dai *cpu_dai,
				int clk_id, unsigned int freq, int dir)
{
	pr_err("quard_star_i2s_set_sysclk\n");
	return 0;
}

static int quard_star_i2s_set_dai_fmt(struct snd_soc_dai *cpu_dai, unsigned int fmt)
{
	pr_err("quard_star_i2s_set_dai_fmt\n");
	return 0;
}

static int quard_star_i2s_startup(struct snd_pcm_substream *substream,
			     struct snd_soc_dai *cpu_dai)
{
	pr_err("quard_star_i2s_startup\n");
	return 0;
}

static int quard_star_i2s_hw_params(struct snd_pcm_substream *substream,
			       struct snd_pcm_hw_params *params,
			       struct snd_soc_dai *cpu_dai)
{
	pr_err("quard_star_i2s_hw_params\n");
	return 0;
}

static int quard_star_i2s_trigger(struct snd_pcm_substream *substream, int cmd,
			     struct snd_soc_dai *cpu_dai)
{
	pr_err("quard_star_i2s_trigger\n");
	return 0;
}

static void quard_star_i2s_shutdown(struct snd_pcm_substream *substream,
			       struct snd_soc_dai *cpu_dai)
{
	pr_err("quard_star_i2s_shutdown\n");
}

static int quard_star_dai_probe(struct snd_soc_dai *cpu_dai)
{
	pr_err("quard_star_dai_probe\n");
	return -1;
}

static const struct snd_soc_dai_ops quard_star_i2s_pcm_dai_ops = {
	.set_sysclk	= quard_star_i2s_set_sysclk,
	.set_fmt	= quard_star_i2s_set_dai_fmt,
	.startup	= quard_star_i2s_startup,
	.hw_params	= quard_star_i2s_hw_params,
	.trigger	= quard_star_i2s_trigger,
	.shutdown	= quard_star_i2s_shutdown,
};

static struct snd_soc_dai_driver dai_drv = {
	.probe = quard_star_dai_probe,
	.ops = &quard_star_i2s_pcm_dai_ops,
	.id = 1,
	.playback = {
		.stream_name = "playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000_192000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
	},
	.capture = {
		.stream_name = "capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000_192000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
	},
};

static const struct snd_pcm_hardware quard_star_pcm_hw = {
	.info = SNDRV_PCM_INFO_INTERLEAVED | SNDRV_PCM_INFO_MMAP,
	.buffer_bytes_max = 8 * PAGE_SIZE,
	.period_bytes_min = 1024,
	.period_bytes_max = 4 * PAGE_SIZE,
	.periods_min = 2,
	.periods_max = 8,
};

static const struct snd_dmaengine_pcm_config quard_star_pcm_config = {
	.pcm_hardware	= &quard_star_pcm_hw,
	.prepare_slave_config = snd_dmaengine_pcm_prepare_slave_config,
	.prealloc_buffer_size = PAGE_SIZE * 8,
};

static const struct snd_soc_component_driver quard_star_i2s_component = {
	.name = "quard-star-i2s",
};

static int quard_star_i2s_remove(struct platform_device *pdev)
{
	snd_dmaengine_pcm_unregister(&pdev->dev);
	snd_soc_unregister_component(&pdev->dev);

	return 0;
}

static int quard_star_i2s_probe(struct platform_device *pdev)
{
	struct resource *res;
	int irq, ret;
	return -1;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	i2s_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(i2s_base))
		return PTR_ERR(i2s_base);

	/* Get irqs */
	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		return irq;

	ret = devm_request_irq(&pdev->dev, irq, quard_star_i2s_isr, IRQF_ONESHOT,
			       dev_name(&pdev->dev), i2s_base);
	if (ret) {
		dev_err(&pdev->dev, "irq request returned %d\n", ret);
		return ret;
	}

	ret = snd_dmaengine_pcm_register(&pdev->dev, &quard_star_pcm_config, 0);
	if (ret) {
		if (ret != -EPROBE_DEFER)
			dev_err(&pdev->dev, "PCM DMA register error %d\n", ret);
		return ret;
	}

	ret = snd_soc_register_component(&pdev->dev, &quard_star_i2s_component,
					 &dai_drv, 1);
	if (ret) {
		snd_dmaengine_pcm_unregister(&pdev->dev);
		return ret;
	}


	return 0;
}

static const struct of_device_id quard_star_i2s_ids[] = {
	{
		.compatible = "quard,quard-star-i2s",
	},
	{},
};

MODULE_DEVICE_TABLE(of, quard_star_i2s_ids);

static struct platform_driver quard_star_i2s_driver = {
	.driver = {
		.name = "quard,quard-star-i2s",
		.of_match_table = quard_star_i2s_ids,
	},
	.probe = quard_star_i2s_probe,
	.remove = quard_star_i2s_remove,
};

module_platform_driver(quard_star_i2s_driver);

MODULE_DESCRIPTION("Quard Star Soc i2s Interface");
MODULE_AUTHOR("QiaoQiming, <qiaoqm@aliyun.com>");
MODULE_LICENSE("GPL v2");
