/*
 * mdiodev.c
 *
 *  Created on: May 1, 2019
 *      Author: zhurish
 */

#include <linux/kernel.h>
#include <linux/phy.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/brcmphy.h>
#include <linux/rtnetlink.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <linux/mii.h>
#include "mdiodev.h"




struct mdiodev mdiodev;

static ssize_t mdiodev_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
	int ret = 0;
	struct midodev_cmd cmd;
	if (copy_from_user(&cmd, buf, sizeof(struct midodev_cmd)))
		return -EFAULT;
	cmd.val = ret = mdiobus_read_nested(mdiodev.bus, cmd.addr, cmd.regnum);
	if (copy_to_user((void __user *)buf, &cmd, sizeof(struct midodev_cmd))) {
			printk(KERN_ERR " read mdio bus copy_to_user\n");
			return -EFAULT;
	}
	return sizeof(cmd);
}

static ssize_t mdiodev_write(struct file *file, const char __user *data, size_t len, loff_t *ppos)
{
	int ret = 0;
	struct midodev_cmd cmd;
	if (copy_from_user(&cmd, data, sizeof(struct midodev_cmd)))
			return -EFAULT;
	ret = mdiobus_write_nested(mdiodev.bus, cmd.addr, cmd.regnum, cmd.val);		

	return ret;
}

static int mdiodev_open(struct inode *inode, struct file *file)
{
	file->private_data = &mdiodev;
	return nonseekable_open(inode, file);
}

static int mdiodev_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}



static long mdiodev_ioctl(struct file *file, unsigned int command,
			     unsigned long u)
{
	int ret = -EINVAL;
	void __user *argp = (void __user *)u;
	struct midodev_cmd cmd;

	if (copy_from_user(&cmd, argp, sizeof(struct midodev_cmd)))
		return -EFAULT;

	if (command == B53_IO_W)
	{
		ret = mdiobus_write_nested(mdiodev.bus, cmd.addr, cmd.regnum, cmd.val);
	}
	else if (command == B53_IO_R)
	{
		cmd.val = mdiobus_read_nested(mdiodev.bus, cmd.addr, cmd.regnum);
		if (copy_to_user((void __user *)u, &cmd, sizeof(struct midodev_cmd))) {
			printk(KERN_ERR " read mdio bus copy_to_user\n");
			return -EFAULT;
		}
		ret = 0;
	}
	return ret;
}





static const struct file_operations mdiodev_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = mdiodev_ioctl,
	.read = mdiodev_read,
	.write = mdiodev_write,
	.open = mdiodev_open,
	.release = mdiodev_release,
	.llseek = no_llseek,
};


static int mdiodev_register(struct mdiodev *mdiodev)
{
	struct device *d = NULL;
	struct mdio_device *mdio = NULL;
	d = bus_find_device_by_name(&mdio_bus_type, NULL, "stmmac-0:1e");
	if (!d) {
		printk(KERN_ERR " Mdio Device %s not found\n", "stmmac-0:1e");
		return -1;
	}
	mdio = to_mdio_device(d);
	mdiodev->bus = mdio->bus;
	mdiodev->dev_num = register_chrdev(0, MDIO_DEVICE_NAME, &mdiodev_fops);
	if (mdiodev->dev_num < 0) 
	{
		printk(KERN_ERR "%s[%d] register_chrdev() error.\n", __FILE__,
		       __LINE__);
		return -1;
	}
	if ((mdiodev->class = class_create(THIS_MODULE, MDIO_MODULE_NAME)) == NULL) 
	{
		unregister_chrdev(mdiodev->dev_num, MDIO_DEVICE_NAME);
		printk(KERN_ERR "%s[%d] class_create() error.\n", __FILE__,
		       __LINE__);
		return -1;
	}
	mdiodev->dev = device_create(mdiodev->class, NULL, MKDEV(mdiodev->dev_num, 0),
			  NULL, "%s%d", MDIO_DEVICE_NAME, 0);
	if (mdiodev->dev == NULL) 
	{
		unregister_chrdev(mdiodev->dev_num, MDIO_DEVICE_NAME);
		printk(KERN_ERR "%s[%d] device_create() error.\n", __FILE__,
		       __LINE__);
		return -1;
	}
	pr_info("mdio driver module load(%d)\n", mdiodev->dev_num);
	return 0;
}

static void mdiodev_unregister(struct mdiodev *mdiodev)
{
	unregister_chrdev(mdiodev->dev_num, MDIO_DEVICE_NAME);
	device_destroy(mdiodev->class, mdiodev->dev_num);
}

static __init int mdiodev_init(void)
{
	return mdiodev_register(&mdiodev);
}

static __exit void mdiodev_exit(void)
{
	if(mdiodev.dev)
		mdiodev_unregister(&mdiodev);
	return ;
}


module_init(mdiodev_init);
module_exit(mdiodev_exit);

MODULE_AUTHOR("zhurish <zhurish@163.com>");
MODULE_DESCRIPTION("MDIO Dev");
MODULE_LICENSE("Dual BSD/GPL");

