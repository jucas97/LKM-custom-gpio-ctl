#pragma once

#define FOOD_DISP_MINOR     0
#define FOOD_DISP_DEV_COUNT 1
#define FOOD_DISP_GPIOS_LEN 2

struct food_disp_priv {
    dev_t my_dev_num;
    const uint32_t pins_legacy_id[FOOD_DISP_GPIOS_LEN];
    struct cdev *my_cdev;
    struct class *my_class;
    struct device *my_device;
    struct gpio_desc *pins_desc[2];
    struct pwm_device *pwm;
};
