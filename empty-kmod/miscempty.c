#undef __KERNEL__
#define __KERNEL__

#undef MODULE
#define MODULE

#define pr_fmt(fmt) "%s (%s:%d): " fmt, KBUILD_MODNAME, __func__, __LINE__

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/types.h>

#define DRIVER_REVISION 0.1

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Simple misc driver version: " __stringify(DRIVER_REVISION));

int empty_open (struct inode *inode, struct file *filp)
{
    return nonseekable_open(inode, filp);
}

int empty_close(struct inode *inode, struct file *filp)
{
	return 0;
}

struct file_operations empty_fops = {
	.owner          = THIS_MODULE,
	.open 		= empty_open,
	.release 	= empty_close,
};

#define NUM_MISC 64
static struct miscdevice empty_misc[NUM_MISC];
static char miscnames[NUM_MISC][64];

static int num_devs;
static int max_devs = 64;
module_param(max_devs, int, 0);

static void empty_init_miscdevs(void)
{
	int i;
	for (i = 0; i < 64; i++)
	{
		snprintf(miscnames[i], 64, "miscempty%d", i);
		empty_misc[i].minor = MISC_DYNAMIC_MINOR;
		empty_misc[i].name = miscnames[i];
		empty_misc[i].fops = &empty_fops;
	}
}

static int __init empty_init(void)
{
	int ret = 0, i;
	empty_init_miscdevs();
	for (i = 0; i < max_devs && !ret; i++) {
		ret = misc_register(&empty_misc[i]);
		num_devs++;
	}
	pr_info("max_devs %d num_devs %d\n", max_devs, num_devs);
   	return 0;
}

static void __exit empty_cleanup(void)
{
	int i;
	for (i = 0; i < num_devs; i++) {
		misc_deregister(&empty_misc[i]);
	}
	pr_info("released %d of %d\n", i, num_devs);
}

module_init(empty_init);
module_exit(empty_cleanup);
