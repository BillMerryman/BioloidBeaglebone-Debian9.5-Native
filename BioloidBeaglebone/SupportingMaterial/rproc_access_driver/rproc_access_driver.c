#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/remoteproc.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bill Merryman");
MODULE_DESCRIPTION("A simple module to access information from the PRU rproc.");
MODULE_VERSION("0.01");

/*
	/sys/class/remoteproc/remoteproc1
	is a symbolic link for 
	/sys/devices/platform/ocp/4a326004.pruss-soc-bus/4a300000.pruss/4a334000.pru/remoteproc/remoteproc1
	
	/sys/class/remoteproc/remoteproc2
	is a symbolic link for 
	/sys/devices/platform/ocp/4a326004.pruss-soc-bus/4a300000.pruss/4a338000.pru/remoteproc/remoteproc2

*/

struct rproc *rproc_ptr;
struct kobject *remoteproc_device_kobj_ptr;
struct kobject *carveout_dir_kobj_ptr;
static struct rproc_subdev *rproc_subdev;
												   											   
static ssize_t carveout_show(struct kobject *kobj, struct kobj_attribute *attr,
                      char *buf)
{
	struct resource_table *table = rproc_ptr->table_ptr;
	struct fw_rsc_carveout *c;
	int i;
	
	if (!table) {
		return 0;
	}
	
	for (i = 0; i < table->num; i++) {
		int offset = table->offset[i];
		struct fw_rsc_hdr *hdr = (void *)table + offset;
		void *rsc = (void *)hdr + sizeof(*hdr);
		if(hdr->type == RSC_CARVEOUT) c = rsc;
	}
	
	return sprintf(buf, "%x\n", (c) ? c->pa : 0);
}

static struct kobj_attribute carveout_attribute =__ATTR(carveout_address, 0440, carveout_show, NULL);

int rproc_access_driver_probe(struct rproc_subdev *subdev)
{
	int error = 0;
	carveout_dir_kobj_ptr = kobject_create_and_add("carveout", remoteproc_device_kobj_ptr); 
	if(!carveout_dir_kobj_ptr) return -ENOMEM;
	printk(KERN_INFO "Carveout subdirectory created on PRU startup\n");
	error = sysfs_create_file(carveout_dir_kobj_ptr, &carveout_attribute.attr);
	if (error) pr_debug("failed to create the carveout_address file in carveout directory\n");
	return error;
}

void rproc_access_driver_remove(struct rproc_subdev *subdev)
{
	if(carveout_dir_kobj_ptr) 
	{
		kobject_put(carveout_dir_kobj_ptr);
		carveout_dir_kobj_ptr = NULL;
		printk(KERN_INFO "Carveout subdirectory removed on PRU shutdown\n");
	}
	else
	{
		printk(KERN_INFO "rproc kobj null. Carveout subdirectory NOT removed\n");
	}
}

static int __init rproc_access_driver_init(void)
{
	struct device_node *pru_device_node_ptr;
	struct platform_device *pru_platform_device_ptr;
	struct device *remoteproc_device_ptr;

	pru_device_node_ptr = of_find_node_by_path("/ocp/pruss_soc_bus@4a326004/pruss@0/pru@34000");
	if(!pru_device_node_ptr)
	{
		printk(KERN_INFO "pru device node could not be acquired at init\n");
		return -ENODEV;
	}
	printk(KERN_INFO "pru device node acquired at init. full_name: %s\n", pru_device_node_ptr->full_name);
	pru_platform_device_ptr = of_find_device_by_node(pru_device_node_ptr);
	of_node_put(pru_device_node_ptr);

	if (!pru_platform_device_ptr) return -EPROBE_DEFER;

	if (!strstr(dev_name(&pru_platform_device_ptr->dev), "pru") && !strstr(dev_name(&pru_platform_device_ptr->dev), "rtu"))
	{
		put_device(&pru_platform_device_ptr->dev);
		return -ENODEV;
	}

	rproc_ptr = platform_get_drvdata(pru_platform_device_ptr);
	put_device(&pru_platform_device_ptr->dev);
	if (!rproc_ptr) return -EPROBE_DEFER;

	printk(KERN_INFO "rproc acquired at init. name: %s\n", rproc_ptr->name);

	//I should probably use get_device here, but I'll live dangerously for now...
	remoteproc_device_ptr = &rproc_ptr->dev;
	remoteproc_device_kobj_ptr = &remoteproc_device_ptr->kobj;
	printk(KERN_INFO "remoteproc kobj name: %s\n", remoteproc_device_kobj_ptr->name);

	rproc_subdev = kzalloc(sizeof(*rproc_subdev), GFP_KERNEL);
	if(!rproc_subdev) return -ENOMEM;

	rproc_add_subdev(rproc_ptr, rproc_subdev, rproc_access_driver_probe, rproc_access_driver_remove);

	return 0;
}

static void __exit rproc_access_driver_exit(void)
{
	/*
	 * Here we will have to account for the possibility that the rproc
	 * may have been already removed by the time we remove our driver.
	 * So we need to check for its existance, and if it exists, remove
	 * the subdevice we created for the carveout. We will also have to
	 * account if the related PRU is running, in which case we will have
	 * to do a full teardown before the main exit logic.
	 */
	struct device_node *pru_device_node_ptr;
	struct platform_device * pru_platform_device_ptr;
	struct rproc *rproc_ptr;

	pru_device_node_ptr = of_find_node_by_path("/ocp/pruss_soc_bus@4a326004/pruss@0/pru@34000");
	if(pru_device_node_ptr)
	{
		printk(KERN_INFO "pru device acquired at exit. full_name: %s\n", pru_device_node_ptr->full_name);
		pru_platform_device_ptr = of_find_device_by_node(pru_device_node_ptr);
		of_node_put(pru_device_node_ptr);

		if (pru_platform_device_ptr)
		{
			if (strstr(dev_name(&pru_platform_device_ptr->dev), "pru") || strstr(dev_name(&pru_platform_device_ptr->dev), "rtu"))
			{
				rproc_ptr = platform_get_drvdata(pru_platform_device_ptr);
				put_device(&pru_platform_device_ptr->dev);
				if (rproc_ptr)
				{
					printk(KERN_INFO "rproc acquired at exit. name: %s\n", rproc_ptr->name);
					rproc_remove_subdev(rproc_ptr, rproc_subdev);
				}
			}
			else
			{
				put_device(&pru_platform_device_ptr->dev);
			}
		}
	}
	printk(KERN_INFO "Freeing carveout subdevice memory\n");
	kfree(rproc_subdev);
}

module_init(rproc_access_driver_init);
module_exit(rproc_access_driver_exit);
