#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Joel Pinto");
MODULE_DESCRIPTION("A custom GPIO controller driver");

static int __init GpioCtlInit(void)
{
    printk(KERN_INFO "GPIOCTL LKM loaded\n");

    return 0;
}

static void __exit GpioCtlExit(void)
{
    printk(KERN_INFO "GPIO LKM unloaded\n");
}

module_init(GpioCtlInit);
module_exit(GpioCtlExit);
