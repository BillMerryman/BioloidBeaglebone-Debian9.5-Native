#!/bin/sh

config-pin P8.27 pruin; \
config-pin P8.28 pruin; \
config-pin P8.29 pruin; \
config-pin P8.39 pruin; \
config-pin P8.40 pruin; \
config-pin P8.41 pruin; \
config-pin P8.42 pruin; \
config-pin P8.43 pruin; \
config-pin P8.44 pruin; \
config-pin P8.45 pruin; \
config-pin P8.46 pruin; \
config-pin P8.34 pwm; \
config-pin P9.24 pru_uart; \
config-pin P9.25 pruout; \
config-pin P9.26 pru_uart; \
if [ -d /sys/class/pwm/pwmchip4/pwm-4:1 ]; \
then \
echo 0 > /sys/class/pwm/pwmchip4/pwm-4:1/enable; \
echo 1 > /sys/class/pwm/pwmchip4/unexport; \
fi; \
echo 1 > /sys/class/pwm/pwmchip4/export; \
echo 62 > /sys/class/pwm/pwmchip4/pwm-4:1/period; \
echo 31 > /sys/class/pwm/pwmchip4/pwm-4:1/duty_cycle; \
echo 1 > /sys/class/pwm/pwmchip4/pwm-4:1/enable

