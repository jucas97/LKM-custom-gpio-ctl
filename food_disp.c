/**
 * @file food_disp.c
 * @author your name (joelstubemail@gmail.com)
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

#include "food_disp.h"
#include "food_disp_shared.h"

#ifdef FOOD_DISP_DEBUG
#define CDRV_LOG(level, msg, ...) printk(KERN_##level "food_disp driver: " msg , ##__VA_ARGS__)
#else
#define CDRV_LOG(level, msg, ...)
#endif /** FOOD_DISP_DEBUG */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Joel Pinto");
MODULE_DESCRIPTION("A custom controller driver for a food dispenser");

static int food_disp_open(struct inode *inode, struct file *file);
static int food_disp_release(struct inode *, struct file *);
static ssize_t food_disp_read(struct file *file, char __user *buffer, size_t len, loff_t *offset);
static ssize_t food_disp_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset);
static long food_disp_ioctl(struct file *file, unsigned int cmd, unsigned long param);
static int create_gpios_desc(void);
static void drop_gpios(void);

// C tag struct initialization
struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .read = food_disp_read,
    .write = food_disp_write,
    .unlocked_ioctl = food_disp_ioctl,
    .open = food_disp_open,
    .release = food_disp_release
};

static struct food_disp_priv my_priv = {
    .pins_legacy_id = {23, 24}
};

/**
 * @brief
 *
 * @param inode
 * @param file
 * @return int
 */
static int food_disp_open(struct inode *inode, struct file *file)
{
    CDRV_LOG(INFO, "open\n");
    return 0;
}

/**
 * @brief
 *
 * @return int
 */
static int food_disp_release(struct inode *inode, struct file *file)
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
static ssize_t food_disp_read(struct file *file, char __user *buffer, size_t len, loff_t *offset)
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
static ssize_t food_disp_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset)
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
static long food_disp_ioctl(struct file *file, unsigned int cmd, unsigned long param)
{
    return 0;
}

/**
 * @brief
 *
 * @return int
 */
static int __init food_disp_init(void)
{
    int ret;

    CDRV_LOG(INFO,"Loading module \n");

    /** Static device number allocation.
     * @TODO - should be updated to dynamic allocation in the future.*/
    my_priv.my_dev_num = MKDEV(FOOD_DISP_MAJOR, FOOD_DISP_MINOR);
    ret = register_chrdev_region(my_priv.my_dev_num, FOOD_DISP_DEV_COUNT, FOOD_DISP_NAME);
    if (ret < 0) {
        CDRV_LOG(WARNING, "Failed to allocate device number, error code %d\n", ret);
        goto failed_register_chrdev;
    }

    my_priv.my_cdev = cdev_alloc();
    if (my_priv.my_cdev == NULL) {
        CDRV_LOG(WARNING, "Failed to allocate cdev structure\n");
        ret = -ENOMEM;
        goto failed_cdev_alloc;
    }
    my_priv.my_cdev->owner = THIS_MODULE;
    my_priv.my_cdev->ops = &my_fops;

    /* Add c dev node to the system - /dev */
    ret = cdev_add(my_priv.my_cdev, my_priv.my_dev_num, FOOD_DISP_DEV_COUNT);
    if (ret < 0) {
        CDRV_LOG(WARNING, "Failed to propagate the char dev to the system, error code %d\n", ret);
        goto failed_cdev_add;
    }

    my_priv.my_class = class_create(THIS_MODULE, FOOD_DISP_NAME);
    if (IS_ERR(my_priv.my_class)) {
        ret = (int) PTR_ERR(my_priv.my_class);
        CDRV_LOG(WARNING, "Failed to create class, error %d\n", ret);
        goto failed_cdev_add;
    }

    my_priv.my_device = device_create(my_priv.my_class, NULL, my_priv.my_dev_num, NULL, FOOD_DISP_NAME);
    if (IS_ERR(my_priv.my_device)) {
        ret = (int) PTR_ERR(my_priv.my_device);
        CDRV_LOG(WARNING, "Failed to create device, error code %d \n", ret);
        goto failed_device_create;
    }

    ret = create_gpios_desc();
    if (ret < 0)
        goto failed_gpios_setup;

    return 0;

failed_gpios_setup:
    drop_gpios();
    device_destroy(my_priv.my_class, my_priv.my_dev_num);
failed_device_create:
    class_destroy(my_priv.my_class);
failed_cdev_add:
    cdev_del(my_priv.my_cdev);
failed_cdev_alloc:
    unregister_chrdev_region(my_priv.my_dev_num, FOOD_DISP_DEV_COUNT);
failed_register_chrdev:
    return ret;
}

/**
 * @brief
 *
 */
static void __exit food_disp_exit(void)
{
    CDRV_LOG(INFO, "Unloading food_disp module\n");
    drop_gpios();
    device_destroy(my_priv.my_class, my_priv.my_dev_num);
    class_destroy(my_priv.my_class);
    cdev_del(my_priv.my_cdev);
    unregister_chrdev_region(my_priv.my_dev_num, FOOD_DISP_DEV_COUNT);
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

    for (i = 0; i < FOOD_DISP_GPIOS_LEN; ++i) {
        int ret;

        ret = gpio_request(my_priv.pins_legacy_id[i], NULL);
        if (ret < 0) {
            CDRV_LOG(WARNING, "Failed to obtain gpio %u through legacy API, error code %d\n", my_priv.pins_legacy_id[i], ret);
            success = -ENOENT;
            break;
        }

        my_priv.pins_desc[i] = gpio_to_desc(my_priv.pins_legacy_id[i]);
        if (my_priv.pins_desc[i] == NULL) {
            CDRV_LOG(WARNING, "Failed to cast pin %u to gpio descriptor\n", my_priv.pins_legacy_id[i]);
            success = ENOENT;
            break;
        }

        ret = gpiod_direction_output(my_priv.pins_desc[i], 0);
        if (ret < 0) {
            success = ENOENT;
            CDRV_LOG(WARNING, "Failed to set direction \n");
            break;
        }
    }

    /**
    * Unexpected failure.
    gpio_test = gpiod_get(my_priv.my_device, "fod", GPIOD_OUT_LOW);
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

    for (i = 0; i < FOOD_DISP_GPIOS_LEN; ++i) {
        int pin;
        pin = desc_to_gpio(my_priv.pins_desc[i]);
        my_priv.pins_desc[i] = NULL;
        gpio_free(pin);
    }
}

module_init(food_disp_init);
module_exit(food_disp_exit);
