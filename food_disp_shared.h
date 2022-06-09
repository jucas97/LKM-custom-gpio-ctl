#pragma once

#define FOOD_DISP_NAME  "food_disp_chr"
// Confirm major availability
#define FOOD_DISP_MAJOR 155 
#define FOOD_DISP_MINOR 0

#define FOOD_DISP_IOCTL_PWM_CHANNEL       _IOWR(FOOD_DISP_MAJOR, 1, int *)
#define FOOD_DISP_IOCTL_PWM_APPLY_STATE   _IOWR(FOOD_DISP_MAJOR, 2, struct pwm_config *)
#define FOOD_DISP_IOCTL_PWM_RELEASE       _IOWR(FOOD_DISP_MAJOR, 3, struct pwm_config *)

struct pwm_config {
    uint64_t period;
    uint64_t duty_cycle;
    int channel;
    bool enabled;
};
