#undef __KERNEL__
#define __KERNEL__

#undef MODULE
#define MODULE

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>    /* __init and __exit macros */

#define MODPFX "empty: "

/*
 *--------------------------------------------------------------------------
 *
 * empty_init -
 *
 *    Module init entry, called at module load time.
 *
 * Return:
 *
 *    Non-zero value indicates module could not be loaded.
 *
 *--------------------------------------------------------------------------
 */
static int __init empty_init(void)
{
   printk(KERN_INFO MODPFX "module_init!\n");
   return 0;
}

/*
 *--------------------------------------------------------------------------
 *
 * empty_cleanup -
 *
 *    Module cleanup entry.
 *
 *--------------------------------------------------------------------------
 */
static void __exit empty_cleanup(void)
{
   printk(KERN_INFO MODPFX "module_exit!\n");
}

module_init(empty_init);
module_exit(empty_cleanup);
