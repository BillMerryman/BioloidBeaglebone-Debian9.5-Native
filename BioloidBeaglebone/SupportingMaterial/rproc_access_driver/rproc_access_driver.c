#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/remoteproc.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bill Merryman");
MODULE_DESCRIPTION("A simple module to access information from the PRU rproc.");
MODULE_VERSION("0.01");

static int __init rproc_access_driver_init(void)
{
	struct device_node *np;
	struct platform_device * pdev;
	struct rproc *rproc;

	np = of_find_node_by_path("/ocp/pruss_soc_bus@4a326004/pruss@0/pru@34000");
	if(!np)
	{
		printk(KERN_INFO "pru device node could not be acquired.\n");
		return -ENODEV;
	}
	printk(KERN_INFO "pru device acquired. full_name: %s.\n", np->full_name);
	pdev = of_find_device_by_node(np);
	of_node_put(np);

	if (!pdev) return -EPROBE_DEFER;

	if (!strstr(dev_name(&pdev->dev), "pru") && !strstr(dev_name(&pdev->dev), "rtu"))
	{
		put_device(&pdev->dev);
		return -ENODEV;
	}

	rproc = platform_get_drvdata(pdev);
	put_device(&pdev->dev);
	if (!rproc) return -EPROBE_DEFER;

	printk(KERN_INFO "rproc acquired. name: %s.\n", rproc->name);

	//get_device(&rproc->dev);

	return 0;
}

static void __exit rproc_access_driver_exit(void)
{
	printk(KERN_INFO "Unloading rproc_access_driver\n");
}

module_init(rproc_access_driver_init);
module_exit(rproc_access_driver_exit);
