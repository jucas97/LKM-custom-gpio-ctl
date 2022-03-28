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
#include <linux/gpio/consumer.h>
#include <linux/gpio.h>
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
static int create_gpios_desc(void);
static void drop_gpios(void);

// C tag struct initialization
struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .read = gpio_ctl_read,
    .write = gpio_ctl_write,
    .unlocked_ioctl = gpio_ctl_ioctl,
    .open = gpio_ctl_open,
    .release = gpio_ctl_release
};

static struct gpio_ctl_priv gpio_priv = {
    .pins_legacy_id = {23, 24}
};

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

    ret = create_gpios_desc();
    if (ret < 0)
        goto failed_gpios_setup;

    return 0;

failed_gpios_setup:
    drop_gpios();
    device_destroy(gpio_priv.my_class, gpio_priv.my_dev_num);
failed_device_create:
    class_destroy(gpio_priv.my_class);
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
    drop_gpios();
    device_destroy(gpio_priv.my_class, gpio_priv.my_dev_num);
    class_destroy(gpio_priv.my_class);
    cdev_del(gpio_priv.my_cdev);
    unregister_chrdev_region(gpio_priv.my_dev_num, GPIO_CTL_DEV_COUNT);
}

/**
 * @brief Create a gpios desc object
 *
 * @return int
 */
static int create_gpios_desc(void)
{
    int i;
    int success = 0;

    for (i = 0; i < GPIO_CTL_GPIOS_LEN; ++i) {
        int ret;

        ret = gpio_request(gpio_priv.pins_legacy_id[i], NULL);
        if (ret < 0) {
            CDRV_LOG(WARNING, "Failed to obtain gpio %u through legacy API, error code %d\n", gpio_priv.pins_legacy_id[i], ret);
            success = -ENOENT;
            break;
        }

        gpio_priv.pins_desc[i] = gpio_to_desc(gpio_priv.pins_legacy_id[i]);
        if (gpio_priv.pins_desc[i] == NULL) {
            CDRV_LOG(WARNING, "Failed to cast pin %u to gpio descriptor\n", gpio_priv.pins_legacy_id[i]);
            success = ENOENT;
            break;
        }

        ret = gpiod_direction_output(gpio_priv.pins_desc[i], 0);
        if (ret < 0) {
            success = ENOENT;
            CDRV_LOG(WARNING, "Failed to set direction \n");
            break;
        }
    }

    /**
    * Unexpected failure.
    gpio_test = gpiod_get(gpio_priv.my_device, "fod", GPIOD_OUT_LOW);
    if (IS_ERR(gpio_test)) {
        ret = (int) PTR_ERR(gpio_test);
        CDRV_LOG(WARNING, "Failed to obtain specific gpio, error code %d", ret);
    }
    */

    return success;
}
 
/**
 * @brief
 *
 */
static void drop_gpios(void)
{
    int i;

    for (i = 0; i < GPIO_CTL_GPIOS_LEN; ++i) {
        int pin;
        pin = desc_to_gpio(gpio_priv.pins_desc[i]);
        gpio_priv.pins_desc[i] = NULL;
        gpio_free(pin);
    }
}

module_init(gpio_ctl_init);
module_exit(gpio_init_exit);
