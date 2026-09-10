/*
*
*   file: ws2812_app.c
*   date: 2026-09-10
*   usage:
*       sudo gcc -o ws2812_app ws2812_app.c
*       sudo ./ws2812_app 1 FF0000              # 单灯：LED1=红
*       sudo ./ws2812_app 3 FF0000              # 单灯：LED3=红
*       sudo ./ws2812_app 1 FF000000FF00        # 双灯：LED1=红, LED2=绿
*       sudo ./ws2812_app 3 FF000000FF000000FF  # 从LED3起3灯：红/绿/蓝
*       sudo ./ws2812_app 1                     # 循环变色(仅LED1)
*
*   说明:
*       参数1 = 起始LED序号(1-based)
*       参数2 = 颜色字符串，长度必须是6的倍数，每6个字符(RRGGBB)对应1个LED
*       不传参数2 = 循环变色模式
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <signal.h>

#define WS2812_DATA_GPIOCHIP     2    /* PC口: PA=0 PB=1 PC=2 ... */
#define WS2812_DATA_GPIONUM      7    /* PC7 */
#define LED_NUM_MAX              20

/* 循环显示的颜色序列 (RGB) */
static const char *color_list[] = {
    "FF0000",   /* 红 */
    "00FF00",   /* 绿 */
    "0000FF",   /* 蓝 */
    "FFFFFF",   /* 白 */
    "00FFFF",   /* 青 */
    "FF00FF",   /* 品红 */
    "FFFF00",   /* 黄 */
    "FF8000",   /* 橙 */
    "8000FF",   /* 紫 */
    "00FF80",   /* 青绿 */
};
#define COLOR_NUM (sizeof(color_list) / sizeof(color_list[0]))

static int running = 1;

static void sig_handler(int sig)
{
    running = 0;
}

struct ws2812_mes {
    unsigned int gpiochip;
    unsigned int gpionum;
    unsigned int lednum;
    unsigned int ledcount;
    unsigned char color[LED_NUM_MAX][3];
};

static int send_colors(int fd, int start_led, int ledcount, const char *hex_str)
{
    struct ws2812_mes ws2812;
    int i;

    ws2812.gpiochip = WS2812_DATA_GPIOCHIP;
    ws2812.gpionum  = WS2812_DATA_GPIONUM;
    ws2812.lednum   = start_led;
    ws2812.ledcount = ledcount;

    memset(ws2812.color, 0, sizeof(ws2812.color));
    for(i = 0; i < ledcount; i++)
    {
        if(sscanf(hex_str + i * 6, "%2hhx%2hhx%2hhx",
                  &ws2812.color[i][0], &ws2812.color[i][1], &ws2812.color[i][2]) != 3)
        {
            printf("Error: parse color[%d] failed\n", i);
            return -1;
        }
    }

    return write(fd, &ws2812, sizeof(struct ws2812_mes));
}

int main(int argc, char **argv)
{
    int fd;
    int start_led;
    int hex_len;
    int ledcount;
    char *endptr;

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    if(argc < 2 || argc > 3)
    {
        printf("Usage: %s <start_led> [hex_colors]\n", argv[0]);
        printf("  指定颜色: %s 1 FF0000             (单灯LED1红)\n", argv[0]);
        printf("  指定颜色: %s 3 FF0000             (单灯LED3红)\n", argv[0]);
        printf("  多灯同色: %s 1 FF0000FF0000       (LED1,LED2均红)\n", argv[0]);
        printf("  多灯异色: %s 3 FF000000FF000000FF (从LED3起3灯：红/绿/蓝)\n", argv[0]);
        printf("  循环变色: %s 1                    (仅LED1循环)\n", argv[0]);
        return -1;
    }

    /* 参数1检查：起始LED序号 */
    start_led = (int)strtol(argv[1], &endptr, 10);
    if(*endptr != '\0' || start_led < 1 || start_led > LED_NUM_MAX)
    {
        printf("Error: start_led must be 1~%d\n", LED_NUM_MAX);
        return -1;
    }

    fd = open("/dev/ws2812", O_RDWR);
    if(fd == -1)
    {
        printf("can not open file /dev/ws2812\n");
        return -1;
    }

    if(argc == 3)
    {
        /* 指定颜色模式 */
        hex_len = strlen(argv[2]);
        if(hex_len == 0 || hex_len % 6 != 0)
        {
            printf("Error: hex_colors length must be multiple of 6, got %d\n", hex_len);
            close(fd);
            return -1;
        }

        ledcount = hex_len / 6;
        if(ledcount < 1 || ledcount > LED_NUM_MAX)
        {
            printf("Error: led count must be 1~%d, got %d\n", LED_NUM_MAX, ledcount);
            close(fd);
            return -1;
        }
        if(start_led + ledcount - 1 > LED_NUM_MAX)
        {
            printf("Error: start_led + ledcount - 1 must <= %d\n", LED_NUM_MAX);
            close(fd);
            return -1;
        }

        send_colors(fd, start_led, ledcount, argv[2]);
    }
    else
    {
        /* 循环变色模式(仅LED1) */
        int idx = 0;
        printf("循环显示颜色中(LED1)，按 Ctrl+C 退出...\n");
        while(running)
        {
            send_colors(fd, 1, 1, color_list[idx]);
            printf("LED1: %s\n", color_list[idx]);
            idx = (idx + 1) % COLOR_NUM;
            usleep(500000);
        }
        /* 退出时熄灭 */
        send_colors(fd, 1, 1, "000000");
        printf("\n已熄灭，退出\n");
    }

    close(fd);
    return 0;
}
