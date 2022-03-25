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
#include <linux/dma-mapping.h>

#include <sound/dmaengine_pcm.h>
#include <sound/pcm_params.h>

/* Audio register offsets */
#define MP_AUDIO_PLAYBACK_MODE  0x00
#define MP_AUDIO_CLOCK_DIV      0x18
#define MP_AUDIO_IRQ_STATUS     0x20
#define MP_AUDIO_IRQ_ENABLE     0x24
#define MP_AUDIO_TX_START_LO    0x28
#define MP_AUDIO_TX_THRESHOLD   0x2C
#define MP_AUDIO_TX_STATUS      0x38
#define MP_AUDIO_TX_START_HI    0x40

/* Status register and IRQ enable bits */
#define MP_AUDIO_TX_HALF        (1 << 6)
#define MP_AUDIO_TX_FULL        (1 << 7)

/* Playback mode bits */
#define MP_AUDIO_16BIT_SAMPLE   (1 << 0)
#define MP_AUDIO_PLAYBACK_EN    (1 << 7)
#define MP_AUDIO_CLOCK_24MHZ    (1 << 9)
#define MP_AUDIO_MONO           (1 << 14)

struct quard_star_i2s_data {
	void __iomem *base;
	struct platform_device *pdev;
	struct snd_soc_dai_driver *dai_drv;
	struct snd_dmaengine_dai_dma_data dma_data_tx;
	char *dmabuffer;
};

static irqreturn_t quard_star_i2s_isr(int irq, void *priv)
{
	struct quard_star_i2s_data *i2s = (struct quard_star_i2s_data *)priv;
	dev_info(&i2s->pdev->dev,"TODO:quard_star_i2s_isr\n");
	return IRQ_HANDLED;
}

static int quard_star_i2s_set_sysclk(struct snd_soc_dai *cpu_dai,
				int clk_id, unsigned int freq, int dir)
{
	dev_info(cpu_dai->dev, "TODO:I2S MCLK frequency is %uHz dir %x\n", freq, dir);
	return 0;
}

static int quard_star_i2s_set_dai_fmt(struct snd_soc_dai *cpu_dai, unsigned int fmt)
{
	dev_info(cpu_dai->dev, "TODO:quard_star_i2s_set_dai_fmt fmt %x\n", fmt);
	return 0;
}

static int quard_star_i2s_startup(struct snd_pcm_substream *substream,
			     struct snd_soc_dai *cpu_dai)
{
	dev_info(cpu_dai->dev,"TODO:quard_star_i2s_startup\n");
	return 0;
}

static int quard_star_i2s_hw_params(struct snd_pcm_substream *substream,
			       struct snd_pcm_hw_params *params,
			       struct snd_soc_dai *cpu_dai)
{
	dev_info(cpu_dai->dev,"TODO:quard_star_i2s_hw_params\n");
	return 0;
}

static int quard_star_i2s_trigger(struct snd_pcm_substream *substream, int cmd,
			     struct snd_soc_dai *cpu_dai)
{
	dev_info(cpu_dai->dev,"TODO:quard_star_i2s_trigger\n");
	return 0;
}

static void quard_star_i2s_shutdown(struct snd_pcm_substream *substream,
			       struct snd_soc_dai *cpu_dai)
{
	dev_info(cpu_dai->dev,"TODO:quard_star_i2s_shutdown\n");
}

static int quard_star_dai_probe(struct snd_soc_dai *cpu_dai)
{
	struct quard_star_i2s_data *i2s = dev_get_drvdata(cpu_dai->dev);

	iowrite32(i2s->dma_data_tx.addr&0xFFFF,i2s->base+MP_AUDIO_TX_START_LO);
	iowrite32((i2s->dma_data_tx.addr>>16)&0xFFFF,i2s->base+MP_AUDIO_TX_START_HI);
	snd_soc_dai_init_dma_data(cpu_dai, &i2s->dma_data_tx, NULL);
	return 0;
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
	struct quard_star_i2s_data *i2s;
	struct resource *res;
	int irq, ret;

	i2s = devm_kzalloc(&pdev->dev, sizeof(*i2s), GFP_KERNEL);
	if (!i2s)
		return -ENOMEM;

	platform_set_drvdata(pdev, i2s);

	i2s->pdev = pdev;
	i2s->dai_drv = &dai_drv;

	i2s->dmabuffer = dma_alloc_coherent(&pdev->dev, 4096, &i2s->dma_data_tx.addr, GFP_KERNEL);
	if (!i2s->dmabuffer) {
		return -ENOMEM;
	}
	i2s->dma_data_tx.addr_width = DMA_SLAVE_BUSWIDTH_UNDEFINED;
	i2s->dma_data_tx.maxburst = 4;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	i2s->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(i2s->base))
		return PTR_ERR(i2s->base);

	/* Get irqs */
	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		return irq;

	ret = devm_request_irq(&pdev->dev, irq, quard_star_i2s_isr, IRQF_ONESHOT,
			       dev_name(&pdev->dev), i2s);
	if (ret) {
		dev_err(&pdev->dev, "irq request returned %d\n", ret);
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
