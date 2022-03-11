#pragma once

#define GPIO_CTL_MINOR     0
#define GPIO_CTL_DEV_COUNT 1

struct gpio_ctl_priv {
    dev_t my_dev_num;
    struct cdev *my_cdev;
    struct class *my_class;
    struct device *my_device;
};