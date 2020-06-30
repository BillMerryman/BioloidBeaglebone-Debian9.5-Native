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
MODULE_DESCRIPTION("A simple module to access carveout information from the pru rproc.");
MODULE_VERSION("0.01");

/*
	/sys/class/remoteproc/remoteproc1
	is a symbolic link for 
	/sys/devices/platform/ocp/4a326004.pruss-soc-bus/4a300000.pruss/4a334000.pru/remoteproc/remoteproc1
	
	/sys/class/remoteproc/remoteproc2
	is a symbolic link for 
	/sys/devices/platform/ocp/4a326004.pruss-soc-bus/4a300000.pruss/4a338000.pru/remoteproc/remoteproc2
*/

struct carveout_kobject
{
	struct kobject kobj;					//kobject for carveout_{n} directory in the carveouts directory
	int resource_entry_number;				//position in the resource table's offsets array of the carveout
	struct fw_rsc_carveout *carveout;		//entry in the resource table of the carveout resource
	struct list_head node;					//to add to linked list of carveouts (carveout_directories) in rproc_subdev_container
};
	
struct rproc_subdev_container
{
	struct rproc_subdev rproc_subdev;		//'the' carveout monitor subdevice
	struct rproc *rproc;					//rproc this carveout monitor is a subdevice for
	struct kobject *carveouts_dir_kobj_ptr;	//the 'carveouts' directory that each carveout information dir will be added to
	struct list_head carveout_directories;	//list of type carveout_kobject, one for each carveout
};

//'The' subdevice container that implements our carveout monitor. May eventually modify to enumerate all remoteproc instances.

static struct rproc_subdev_container *rproc_subdev_container;

//The file attributes associated with each carveout: physical address (pa), length (len), and name (name)
				
//shows the physical address of the carveout				
static ssize_t pa_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{	
	struct carveout_kobject *carveout_kobject = container_of(kobj, struct carveout_kobject, kobj);
	struct fw_rsc_carveout *carveout = carveout_kobject->carveout;
	return sprintf(buf, "%x", carveout->pa);
}

//shows the length/size of the carveout					   											   
static ssize_t len_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{	
	struct carveout_kobject *carveout_kobject = container_of(kobj, struct carveout_kobject, kobj);
	struct fw_rsc_carveout *carveout = carveout_kobject->carveout;
	return sprintf(buf, "%x", carveout->len);
}

//shows the name of the carveout					   											   
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

//end: The file attributes associated with each carveout: physical address (pa), length (len), and name (name)

//Custom release method and kobj_type for our kobject container which represents the kobject for each carveout subdirectory
void obj_release(struct kobject *kobj)
{
    struct carveout_kobject *carveout_kobject = container_of(kobj, struct carveout_kobject, kobj);
    printk(KERN_INFO "freeing %s carveout_kobject\n", kobject_name(&carveout_kobject->kobj));
    kfree(carveout_kobject);
}

static struct kobj_type carveout_kobj_ktype = {
	.release	= obj_release,
	.sysfs_ops	= &kobj_sysfs_ops,
};

int rproc_access_driver_probe(struct rproc_subdev *subdev)
{
	//Get the subdevice container to get the rproc, which is used to get the resource table, 
	//and also to get the device through which we get the kobject
	//The kobject is for the remoteproc directory, in my case:
	//		/sys/devices/platform/ocp/4a326004.pruss-soc-bus/4a300000.pruss/4a334000.pru/remoteproc/remoteproc1
	//which is also aliased from 
	//		/sys/class/remoteproc/remoteproc1
	//I should probably use get_device for the device here, but I'll live dangerously for now...
	struct rproc_subdev_container *rproc_subdev_container = container_of(subdev, struct rproc_subdev_container, rproc_subdev);
	struct rproc *rproc = rproc_subdev_container->rproc;
	struct device *dev = &rproc->dev;
	struct kobject *kobj = &dev->kobj;
	struct resource_table *table = rproc->table_ptr;
	int resource_counter;
	
	if (!table) return 0;
	
	//Create the 'carveouts' directory in the remoteproc directory (remoteproc1 in this case)
	rproc_subdev_container->carveouts_dir_kobj_ptr = kobject_create_and_add("carveouts", kobj); 
	if(!rproc_subdev_container->carveouts_dir_kobj_ptr) return -ENOMEM;
	printk(KERN_INFO "%s directory created on probe\n", kobject_name(rproc_subdev_container->carveouts_dir_kobj_ptr));
	
	//Create a directory for each carveout. 
	//We will name them 'carveout_{n}' where {n} is the index of the carveout in the resource table
	for (resource_counter = 0; resource_counter < table->num; resource_counter++) {
		int offset = table->offset[resource_counter];
		struct fw_rsc_hdr *hdr = (void *)table + offset;
		void *rsc = (void *)hdr + sizeof(*hdr);
		if(hdr->type == RSC_CARVEOUT)
		{
			char carveout_folder_name[12];
			sprintf(carveout_folder_name, "carveout_%d", resource_counter);
			struct carveout_kobject *carveout_kobject = kzalloc(sizeof(*carveout_kobject), GFP_KERNEL);
			if (!carveout_kobject) {
				printk(KERN_INFO "failed to kzalloc %s subdirectory on probe.  Skipping...\n", carveout_folder_name);
				continue;
			}
			carveout_kobject->resource_entry_number = resource_counter;
			carveout_kobject->carveout = (struct fw_rsc_carveout *)rsc;    
			if (kobject_init_and_add(&carveout_kobject->kobj, &carveout_kobj_ktype, rproc_subdev_container->carveouts_dir_kobj_ptr, carveout_folder_name))
			{
				printk(KERN_INFO "failed to init and add %s subdirectory on probe.  Skipping...\n", kobject_name(&carveout_kobject->kobj));
				kfree(carveout_kobject);
				continue;
			}
			if (sysfs_create_files(&carveout_kobject->kobj, (const struct attribute **)carveout_attrs))
			{
				printk(KERN_INFO "failed create attribute files for %s on probe. skipping...\n", kobject_name(&carveout_kobject->kobj));
				kobject_put(&carveout_kobject->kobj);
				continue;
			}
			list_add_tail(&carveout_kobject->node, &rproc_subdev_container->carveout_directories);
			printk(KERN_INFO "%s subdirectory created on probe\n", kobject_name(&carveout_kobject->kobj));
		}
	}
	
	return 0;
}

void rproc_access_driver_remove(struct rproc_subdev *subdev)
{
	struct rproc_subdev_container *rproc_subdev_container = container_of(subdev, struct rproc_subdev_container, rproc_subdev);
	struct carveout_kobject *carveout_kobject, *tmp;

	//remove all of the carveout subdirectories from the carveouts directory
	list_for_each_entry_safe(carveout_kobject, tmp, &rproc_subdev_container->carveout_directories, node) {
		list_del(&carveout_kobject->node);
		kobject_put(&carveout_kobject->kobj);
	}

	//remove the carveouts directory
	kobject_put(rproc_subdev_container->carveouts_dir_kobj_ptr);
	rproc_subdev_container->carveouts_dir_kobj_ptr = NULL;
	printk(KERN_INFO "carveouts directory removed on remove\n");
}

static int __init rproc_access_driver_init(void)
{
	//Get the device node
	struct device_node *device_node = of_find_node_by_path("/ocp/pruss_soc_bus@4a326004/pruss@0/pru@34000");
	if(!device_node)
	{
		printk(KERN_INFO "pru device node could not be acquired at init\n");
		return -ENODEV;
	}
	printk(KERN_INFO "pru device node (full_name: %s) acquired at init\n", device_node->full_name);
	
	//Get the platform device
	struct platform_device *platform_device = of_find_device_by_node(device_node);
	of_node_put(device_node); //release the device node
	if (!platform_device)
	{
		printk(KERN_INFO "pru platform device could not be acquired at init\n");
		return -EPROBE_DEFER;
	}
	printk(KERN_INFO "pru platform device (name: %s) acquired at init\n", platform_device->name);
	
	//Make sure the device we got is a pru (probably not really necessary here)
	if (!strstr(dev_name(&platform_device->dev), "pru") && !strstr(dev_name(&platform_device->dev), "rtu"))
	{
		put_device(&platform_device->dev);
		return -ENODEV;
	}

	//Get the rproc
	struct rproc *rproc = platform_get_drvdata(platform_device);
	put_device(&platform_device->dev); //release the platform device.
	if (!rproc)
	{
		printk(KERN_INFO "rproc could not be acquired at init\n");
		return -EPROBE_DEFER;
	}
	printk(KERN_INFO "rproc (name: %s) acquired at init\n", rproc->name);

	//Create a subdevice container (which containes the rproc_subdev that implements the carveout monitor and handles its callbacks)
	rproc_subdev_container = kzalloc(sizeof(*rproc_subdev_container), GFP_KERNEL);
	if(!rproc_subdev_container) return -ENOMEM;
	INIT_LIST_HEAD(&rproc_subdev_container->carveout_directories);
	rproc_subdev_container->rproc = rproc;
	
	//Add the subdevice
	rproc_add_subdev(rproc, &rproc_subdev_container->rproc_subdev, rproc_access_driver_probe, rproc_access_driver_remove);

	return 0;
	
	//Do I need to do a 'get' on the rproc? If so, I should then do a matching 'put' call in the __exit function?
}

static void __exit rproc_access_driver_exit(void)
{
	if(rproc_subdev_container->carveouts_dir_kobj_ptr) rproc_access_driver_remove(&rproc_subdev_container->rproc_subdev);
	rproc_remove_subdev(rproc_subdev_container->rproc, &rproc_subdev_container->rproc_subdev);
	printk(KERN_INFO "rproc_access_driver exit\n");
}

module_init(rproc_access_driver_init);
module_exit(rproc_access_driver_exit);
