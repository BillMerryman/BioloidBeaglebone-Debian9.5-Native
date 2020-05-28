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
MODULE_DESCRIPTION("A simple module to access carveout information from the PRU rproc.");
MODULE_VERSION("0.01");

/*
	/sys/class/remoteproc/remoteproc1
	is a symbolic link for 
	/sys/devices/platform/ocp/4a326004.pruss-soc-bus/4a300000.pruss/4a334000.pru/remoteproc/remoteproc1
	
	/sys/class/remoteproc/remoteproc2
	is a symbolic link for 
	/sys/devices/platform/ocp/4a326004.pruss-soc-bus/4a300000.pruss/4a338000.pru/remoteproc/remoteproc2
	
	/*
		Next up, get the tear-down working on remove rproc_access_driver_remove. Will need to add the 
		carveout subdirectories into the list on creation and then 'put' each in the list before freeing 
		the carveouts_dir_kobj_ptr in rproc_subdev_container on disposal.
	*/

*/

struct carveout_kobject
{
	struct list_head node;
	int resource_entry_number;
	struct fw_rsc_carveout *carveout;
	struct kobject kobj;
};
	
struct rproc_subdev_container
{
	struct rproc *rproc;
	struct rproc_subdev rproc_subdev;
	struct kobject *carveouts_dir_kobj_ptr;
	struct list_head carveout_directories;
};

static struct rproc_subdev_container *rproc_subdev_container;
				   											   
static ssize_t pa_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{	
	struct carveout_kobject *carveout_kobject = container_of(kobj, struct carveout_kobject, kobj);
	struct fw_rsc_carveout *carveout = carveout_kobject->carveout;
	return sprintf(buf, "%x", carveout->pa);
}
						   											   
static ssize_t len_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{	
	struct carveout_kobject *carveout_kobject = container_of(kobj, struct carveout_kobject, kobj);
	struct fw_rsc_carveout *carveout = carveout_kobject->carveout;
	return sprintf(buf, "%x", carveout->len);
}
						   											   
static ssize_t name_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{	
	struct carveout_kobject *carveout_kobject = container_of(kobj, struct carveout_kobject, kobj);
	struct fw_rsc_carveout *carveout = carveout_kobject->carveout;
	return sprintf(buf, "%s", carveout->name);
}

static struct kobj_attribute carveout_pa_kobj_attribute = __ATTR_RO(pa);
static struct kobj_attribute carveout_len_kobj_attribute = __ATTR_RO(len);
static struct kobj_attribute carveout_name_kobj_attribute = __ATTR_RO(name);

struct attribute *carveout_attrs[] = {
    &carveout_pa_kobj_attribute.attr,
    &carveout_len_kobj_attribute.attr,
    &carveout_name_kobj_attribute.attr,
    NULL,
};

void obj_release(struct kobject *kobj)
{
    struct carveout_kobject *carveout_kobject = container_of(kobj, struct carveout_kobject, kobj);
    printk(KERN_INFO "carveout_kobject release\n");
    kfree(carveout_kobject);
}

static struct kobj_type carveout_kobj_ktype = {
	.release	= obj_release,
	.sysfs_ops	= &kobj_sysfs_ops,
};

int rproc_access_driver_probe(struct rproc_subdev *subdev)
{
	int error = 0;
	
	//Get the subdevice container to get the rproc
	struct rproc_subdev_container *rproc_subdev_container = container_of(subdev, struct rproc_subdev_container, rproc_subdev);
	struct rproc *rproc = rproc_subdev_container->rproc;
	struct device *remoteproc_device_ptr = &rproc->dev;
	struct kobject *remoteproc_device_kobj_ptr = &remoteproc_device_ptr->kobj;
	struct resource_table *table = rproc->table_ptr;
	int resource_counter;
	
	if (!table) return 0;
	
	//Create the 'carveouts' directory
	rproc_subdev_container->carveouts_dir_kobj_ptr = kobject_create_and_add("carveouts", remoteproc_device_kobj_ptr); 
	if(!rproc_subdev_container->carveouts_dir_kobj_ptr) return -ENOMEM;
	printk(KERN_INFO "%s subdirectory created on probe\n", kobject_name(rproc_subdev_container->carveouts_dir_kobj_ptr));
	
	//Create a directory for each carveout by name
	
	for (resource_counter = 0; resource_counter < table->num; resource_counter++) {
		int offset = table->offset[resource_counter];
		struct fw_rsc_hdr *hdr = (void *)table + offset;
		void *rsc = (void *)hdr + sizeof(*hdr);
		if(hdr->type == RSC_CARVEOUT)
		{
			char carveout_folder_name[12];
			sprintf(carveout_folder_name, "carveout_%d", resource_counter);
			
			//This is where we will need to make the custom kobject, instead of this create and add.
			//Are we going to want to add these to some kind of linked list for later removal?
			struct carveout_kobject *carveout_kobject = kzalloc(sizeof(*carveout_kobject), GFP_KERNEL);
			if (!carveout_kobject) {
				return -ENOMEM;
			}
			carveout_kobject->resource_entry_number = resource_counter;
			carveout_kobject->carveout = (struct fw_rsc_carveout *)rsc;    
			kobject_init_and_add(&carveout_kobject->kobj, &carveout_kobj_ktype, rproc_subdev_container->carveouts_dir_kobj_ptr, carveout_folder_name);
			error = sysfs_create_files(&carveout_kobject->kobj, (const struct attribute **)carveout_attrs);
			if (error)
			{
				printk(KERN_INFO "create obj attribute failed!\n");
				kobject_put(&carveout_kobject->kobj);
				return error;
			}
		}
	}
	
	return error;
}

void rproc_access_driver_remove(struct rproc_subdev *subdev)
{
	struct rproc_subdev_container *rproc_subdev_container = container_of(subdev, struct rproc_subdev_container, rproc_subdev);
	struct rproc *rproc = rproc_subdev_container->rproc;
	struct device *remoteproc_device_ptr = &rproc->dev;
	struct kobject *remoteproc_device_kobj_ptr = &remoteproc_device_ptr->kobj;
	struct kobject *carveouts_dir_kobj_ptr;
	/*
		Do a loop here to put all of the carveout directories
	*/
	kobject_put(rproc_subdev_container->carveouts_dir_kobj_ptr);
	rproc_subdev_container->carveouts_dir_kobj_ptr = NULL;
	printk(KERN_INFO "Carveout subdirectory removed on PRU shutdown\n");
}

static int __init rproc_access_driver_init(void)
{
	//Get the device node
	struct device_node *pru_device_node_ptr = of_find_node_by_path("/ocp/pruss_soc_bus@4a326004/pruss@0/pru@34000");
	if(!pru_device_node_ptr)
	{
		printk(KERN_INFO "pru device node could not be acquired at init\n");
		return -ENODEV;
	}
	printk(KERN_INFO "pru device node acquired at init. full_name: %s\n", pru_device_node_ptr->full_name);
	
	//Get the platform device
	struct platform_device *pru_platform_device_ptr = of_find_device_by_node(pru_device_node_ptr);
	of_node_put(pru_device_node_ptr); //release the device node
	if (!pru_platform_device_ptr)
	{
		printk(KERN_INFO "pru platform device could not be acquired at init\n");
		return -EPROBE_DEFER;
	}

	//Make sure the device we got is a pru (not really necessary here)
	if (!strstr(dev_name(&pru_platform_device_ptr->dev), "pru") && !strstr(dev_name(&pru_platform_device_ptr->dev), "rtu"))
	{
		put_device(&pru_platform_device_ptr->dev);
		return -ENODEV;
	}

	//Get the rproc
	struct rproc *rproc = platform_get_drvdata(pru_platform_device_ptr);
	put_device(&pru_platform_device_ptr->dev); //release the platform device
	if (!rproc)
	{
		printk(KERN_INFO "rproc could not be acquired at init\n");
		return -EPROBE_DEFER;
	}
	printk(KERN_INFO "rproc acquired at init. name: %s\n", rproc->name);

	//Get the device from the rproc, and then the kobject from the device
	//I should probably use get_device here, but I'll live dangerously for now...
	struct device *remoteproc_device_ptr = &rproc->dev;
	struct kobject *remoteproc_device_kobj_ptr = &remoteproc_device_ptr->kobj;
	printk(KERN_INFO "remoteproc kobj name: %s\n", remoteproc_device_kobj_ptr->name);

	//Create a subdevice container with the subdevice for the callbacks to occur when the pru is started and stopped
	rproc_subdev_container = kzalloc(sizeof(*rproc_subdev_container), GFP_KERNEL);
	if(!rproc_subdev_container) return -ENOMEM;

	rproc_subdev_container->rproc = rproc;
	//Add the subdevice
	rproc_add_subdev(rproc, &rproc_subdev_container->rproc_subdev, rproc_access_driver_probe, rproc_access_driver_remove);

	return 0;
}

static void __exit rproc_access_driver_exit(void)
{
	rproc_remove_subdev(rproc_subdev_container->rproc, &rproc_subdev_container->rproc_subdev);
	printk(KERN_INFO "Freeing carveout subdevice memory\n");
	kfree(rproc_subdev_container);
}

module_init(rproc_access_driver_init);
module_exit(rproc_access_driver_exit);
