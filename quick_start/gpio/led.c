#include <stdio.h>
#include <unistd.h>
#include <gpiod.h>

#define msleep(t) usleep((t)*1000)

int main(int argc, char const *argv[])
{
    struct gpiod_chip *gpiochip1;
    struct gpiod_line *led;
    struct gpiod_line_request_config config;
    int req;
    /* 板载LED灯：PF6=(F-1)*32+6=(6-1)*32+6=166 */
    int PF6=166;

    /* 打开 GPIO 控制器 */
    gpiochip1 = gpiod_chip_open("/dev/gpiochip1");
    if (!gpiochip1)
    {
        printf("gpiochip open error\n");
        return -1;
    }

    /* 获取PF6引脚 */
    led = gpiod_chip_get_line(gpiochip1, PF6);
    if (!led)
    {
        gpiod_chip_close(gpiochip1);
        printf("led get error.\n");
        return -1;
    }
    
    /* 配置引脚  输出模式 name为“led666” 初始电平为high*/
    req = gpiod_line_request_output(led, "led666", 1);
    if (req)
    {
        fprintf(stderr, "led request error.\n");
        return -1;
    }

    while (1)
    {
        /* 设置引脚电平 */
        gpiod_line_set_value(led, 1);
        printf("led close\n"); //板载LED为低电平触发
        msleep(500);
        gpiod_line_set_value(led, 0);
        printf("led open\n");
        msleep(500);
    }

    return 0;
}
