// SPDX-License-Identifier: GPL-2.0-only
#include <linux/device.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/onenand.h>
#include <linux/mtd/partitions.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/slab.h>

#define DRIVER_NAME "quard_star-onenand"

struct quard_star_onenand {
	struct platform_device *pdev;
	struct mtd_info mtd;
	struct onenand_chip onenand;
};

static int quard_star_onenand_probe(struct platform_device *pdev)
{
	struct resource *res;
	int err;
	struct quard_star_onenand *nand;
	struct device *dev = &pdev->dev;

	nand = devm_kzalloc(dev, sizeof(struct quard_star_onenand), GFP_KERNEL);
	if (!nand)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(dev, "error getting memory resource\n");
		return -EINVAL;
	}

	nand->onenand.base = devm_ioremap_resource(dev, res);
	if (IS_ERR(nand->onenand.base))
		return PTR_ERR(nand->onenand.base);
	
	nand->onenand.irq = platform_get_irq(pdev, 0);
	if (nand->onenand.irq < 0) {
		dev_err(dev, "error getting irq\n");
		return -EINVAL;
	}

	nand->mtd.priv = &nand->onenand;
	nand->mtd.name = dev_name(dev);
	nand->mtd.dev.parent = dev;
	mtd_set_of_node(&nand->mtd, dev->of_node);
	err = onenand_scan(&nand->mtd, 1);
	if (err < 0) {
		dev_err(dev, "onenand_scan err\n");
		goto err_onenand_scan;
	}

	err = mtd_device_register(&nand->mtd, NULL, 0);
	if (err) {
		dev_err(dev, "mtd_device_register err\n");
		goto err_mtd_device_register;
	}

	nand->pdev = pdev;
	platform_set_drvdata(pdev, nand);

	return 0;

err_mtd_device_register:
	onenand_release(&nand->mtd);
err_onenand_scan:
	return err;
}

static int quard_star_onenand_remove(struct platform_device *pdev)
{
	struct quard_star_onenand *nand = dev_get_drvdata(&pdev->dev);

	onenand_release(&nand->mtd);
	return 0;
}

static const struct of_device_id quard_star_onenand_id_table[] = {
	{ .compatible = "quard,quard-star-onenand", },
	{},
};
MODULE_DEVICE_TABLE(of, quard_star_onenand_id_table);

static struct platform_driver quard_star_onenand_driver = {
	.probe		= quard_star_onenand_probe,
	.remove		= quard_star_onenand_remove,
	.driver		= {
		.name	= DRIVER_NAME,
		.of_match_table = quard_star_onenand_id_table,
	},
};

module_platform_driver(quard_star_onenand_driver);

MODULE_ALIAS("platform:" DRIVER_NAME);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("QiaoQiming, <qiaoqm@aliyun.com>");
MODULE_DESCRIPTION("Glue layer for OneNAND flash on Quard Star");
