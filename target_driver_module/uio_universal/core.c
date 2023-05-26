// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/uio_driver.h>
#include <linux/of.h>
#include <linux/dma-map-ops.h>


#define DRIVER_NAME "uio_universal"

struct uio_universal {
	phys_addr_t start;
	phys_addr_t end;
	int irq;
	struct uio_info info;
};

static irqreturn_t uio_universal_irq_hander(int irq, void *dev_id)
{
	struct uio_universal *uioudrv = (struct uio_universal *)dev_id;
	uio_event_notify(&uioudrv->info);

	return IRQ_HANDLED;
}

static int mmap(struct uio_info *info, struct vm_area_struct *vma)
{
    struct uio_universal *uioudrv = container_of(info, struct uio_universal, info);
	int err;
	vma->vm_page_prot = pgprot_dmacoherent(vma->vm_page_prot);
	err = remap_pfn_range(vma,vma->vm_start,
		(uioudrv->start)>>PAGE_SHIFT,
		vma->vm_end - vma->vm_start,
		vma->vm_page_prot);
	if (err != 0)
		return -EAGAIN;
	return 0;
}

static int uio_universal_probe(struct platform_device *pdev)
{
	struct uio_universal *uioudrv;
	int err;
	struct resource *res;
	struct uio_mem *uiomem;
	const char *uio_name;
	struct device_node *np = pdev->dev.of_node;

	uioudrv = devm_kzalloc(&pdev->dev, sizeof(*uioudrv), GFP_KERNEL);
	if (!uioudrv)
		return -ENOMEM;

	platform_set_drvdata(pdev, uioudrv);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (IS_ERR(res))
		return PTR_ERR(res);
	uioudrv->start = res->start;
	uioudrv->end = res->end;

	uioudrv->irq = platform_get_irq(pdev, 0);
	if (uioudrv->irq < 0)
		return -ENODEV;

	if (of_property_read_string(np, "uio-name", &uio_name) < 0) {
		return -EINVAL;
	}

	err = devm_request_irq(&pdev->dev, uioudrv->irq,
			       uio_universal_irq_hander,
			       0, pdev->name, uioudrv);
	if (err)
		return err;

	uioudrv->info.name = "uio_universal";
	uioudrv->info.version = "0.0.1";
	uioudrv->info.irq = UIO_IRQ_CUSTOM;
	uioudrv->info.mmap = mmap;

	uiomem = &uioudrv->info.mem[0];
	uiomem->memtype = UIO_MEM_PHYS;
	uiomem->addr = uioudrv->start & PAGE_MASK;
	uiomem->offs = uioudrv->start & ~PAGE_MASK;
	uiomem->size =
		(uiomem->offs + resource_size(res) + PAGE_SIZE - 1) &
		PAGE_MASK;
	uiomem->name = uio_name;

	return uio_register_device(&pdev->dev, &uioudrv->info);
}

static int uio_universal_remove(struct platform_device *pdev)
{
	struct uio_universal *uioudrv = platform_get_drvdata(pdev);
	uio_unregister_device(&uioudrv->info);
	return 0;
}

static const struct of_device_id uio_universal_match[] = {
	{ .compatible = "uio-universal", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, uio_universal_match);

static struct platform_driver uio_universal_driver = {
	.probe = uio_universal_probe,
	.remove = uio_universal_remove,
	.driver = {
		.name = DRIVER_NAME,
		.of_match_table = uio_universal_match,
	},
	.remove = __exit_p(uio_universal_remove),
};


static int __init uio_universal_init(void)
{
	int ret = 0;
	ret = platform_driver_register(&uio_universal_driver);
	if (ret) {
		pr_err("register platform drv %s failed\n", uio_universal_driver.driver.name);
		return ret;
	}
	return 0;
}

static void __exit uio_universal_exit(void)
{
	platform_driver_unregister(&uio_universal_driver);
}

module_init(uio_universal_init);
module_exit(uio_universal_exit);

MODULE_AUTHOR("qiaoqm@aliyun.com");
MODULE_DESCRIPTION("uio universal driver");
MODULE_LICENSE("GPL v2");
