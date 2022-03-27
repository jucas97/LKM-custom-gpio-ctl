/**
 * @file gpio_ctl.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-03-10
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
/**
 * Kernel log level
 * #include <linux/kern_levels.h>
 */

#include "gpio_ctl.h"
#include "gpio_ctl_shared.h"

#ifdef GPIO_CTL_DEBUG
#define CDRV_LOG(level, msg, ...) printk(KERN_##level "gpio_ctl driver: " msg , ##__VA_ARGS__)
#else
#define CDRV_LOG(level, msg, ...)
#endif /** GPIO_CTL_DEBUG */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Joel Pinto");
MODULE_DESCRIPTION("A custom GPIO controller driver");

static int gpio_ctl_open(struct inode *inode, struct file *file);
static int gpio_ctl_release(struct inode *, struct file *);
static ssize_t gpio_ctl_read(struct file *file, char __user *buffer, size_t len, loff_t *offset);
static ssize_t gpio_ctl_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset);
static long gpio_ctl_ioctl(struct file *file, unsigned int cmd, unsigned long param);

// C tag struct initialization
struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .read = gpio_ctl_read,
    .write = gpio_ctl_write,
    .unlocked_ioctl = gpio_ctl_ioctl,
    .open = gpio_ctl_open,
    .release = gpio_ctl_release
};

static struct gpio_ctl_priv gpio_priv;

/**
 * @brief
 *
 * @param inode
 * @param file
 * @return int
 */
static int gpio_ctl_open(struct inode *inode, struct file *file)
{
    CDRV_LOG(INFO, "open\n");
    return 0;
}

/**
 * @brief
 *
 * @return int
 */
static int gpio_ctl_release(struct inode *inode, struct file *file)
{
    CDRV_LOG(INFO, "release\n");
    return 0;
}

/**
 * @brief
 *
 * @param file
 * @param buffer
 * @param len
 * @param offset
 * @return ssize_t
 */
static ssize_t gpio_ctl_read(struct file *file, char __user *buffer, size_t len, loff_t *offset)
{
    CDRV_LOG(INFO, "read\n");
    return 0;
}

/**
 * @brief
 *
 * @param file
 * @param buffer
 * @param len
 * @param offset
 * @return ssize_t
 */
static ssize_t gpio_ctl_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset)
{
    CDRV_LOG(INFO, "write\n");
    return 1;
}

/**
 * @brief
 *
 * @param file
 * @param cmd
 * @param param
 * @return long
 */
static long gpio_ctl_ioctl(struct file *file, unsigned int cmd, unsigned long param)
{
    return 0;
}

/**
 * @brief
 *
 * @return int
 */
static int __init gpio_ctl_init(void)
{
    int ret;

    CDRV_LOG(INFO,"Loading module \n");

    /** Static device number allocation.
     * @TODO - should be updated to dynamic allocation in the future.*/
    gpio_priv.my_dev_num = MKDEV(GPIO_CTL_MAJOR, GPIO_CTL_MINOR);
    ret = register_chrdev_region(gpio_priv.my_dev_num, GPIO_CTL_DEV_COUNT, GPIO_CTL_NAME);
    if (ret < 0) {
        CDRV_LOG(WARNING, "Failed to allocate device number, error code %d\n", ret);
        goto failed_register_chrdev;
    }

    gpio_priv.my_cdev = cdev_alloc();
    if (gpio_priv.my_cdev == NULL) {
        CDRV_LOG(WARNING, "Failed to allocate cdev structure\n");
        ret = -ENOMEM;
        goto failed_cdev_alloc;
    }
    gpio_priv.my_cdev->owner = THIS_MODULE;
    gpio_priv.my_cdev->ops = &my_fops;

    /* Add c dev node to the system - /dev */
    ret = cdev_add(gpio_priv.my_cdev, gpio_priv.my_dev_num, GPIO_CTL_DEV_COUNT);
    if (ret < 0) {
        CDRV_LOG(WARNING, "Failed to propagate the char dev to the system, error code %d\n", ret);
        goto failed_cdev_add;
    }

    gpio_priv.my_class = class_create(THIS_MODULE, GPIO_CTL_NAME);
    if (IS_ERR(gpio_priv.my_class)) {
        ret = (int) PTR_ERR(gpio_priv.my_class);
        CDRV_LOG(WARNING, "Failed to create class, error %d\n", ret);
        goto failed_cdev_add;
    }

    gpio_priv.my_device = device_create(gpio_priv.my_class, NULL, gpio_priv.my_dev_num, NULL, GPIO_CTL_NAME);
    if (IS_ERR(gpio_priv.my_device)) {
        ret = (int) PTR_ERR(gpio_priv.my_device);
        CDRV_LOG(WARNING, "Failed to create device, error code %d \n", ret);
        goto failed_device_create;
    }

    return 0;

failed_device_create:
    device_destroy(gpio_priv.my_class, gpio_priv.my_dev_num);
failed_cdev_add:
    cdev_del(gpio_priv.my_cdev);
failed_cdev_alloc:
    unregister_chrdev_region(gpio_priv.my_dev_num, GPIO_CTL_DEV_COUNT);
failed_register_chrdev:
    return ret;
}

/**
 * @brief
 *
 */
static void __exit gpio_init_exit(void)
{
    CDRV_LOG(INFO, "Unloading gpio_ctl module\n");
    device_destroy(gpio_priv.my_class, gpio_priv.my_dev_num);
    class_destroy(gpio_priv.my_class);
    cdev_del(gpio_priv.my_cdev);
    unregister_chrdev_region(gpio_priv.my_dev_num, GPIO_CTL_DEV_COUNT);
}

module_init(gpio_ctl_init);
module_exit(gpio_init_exit);
