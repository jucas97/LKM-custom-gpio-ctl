#pragma once

#define GPIO_CTL_MINOR     0
#define GPIO_CTL_DEV_COUNT 1
#define GPIO_CTL_GPIOS_LEN 2

struct gpio_ctl_priv {
    dev_t my_dev_num;
    const uint32_t pins_legacy_id[GPIO_CTL_GPIOS_LEN];
    struct cdev *my_cdev;
    struct class *my_class;
    struct device *my_device;
    struct gpio_desc *pins_desc[2];
};
