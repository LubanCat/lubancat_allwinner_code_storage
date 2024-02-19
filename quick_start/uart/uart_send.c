#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <string.h>
#include <sys/ioctl.h>

//第一部分代码/
//根据具体的设备修改
const char default_path[] = "/dev/ttyAS5";


int main(int argc, char *argv[])
{
    int fd;
    int res;
    char *path;
    char send_buf[1024] = "Embedfire tty send test.\n";

    // 标准输出设备
    int save_fd = dup(STDOUT_FILENO);

    //第二部分代码/
    //若无输入参数则使用默认终端设备
    if (argc > 1)
        path = argv[1];
    else
        path = (char *)default_path;

    //获取串口设备描述符
    printf("This is tty/uart send demo.\n");
    fd = open(path, O_RDWR);
    if (fd < 0) {
        printf("Fail to Open %s device\n", path);
        return 0;
    }


    //第三部分代码/
    struct termios opt;
    //清空串口接收缓冲区
    tcflush(fd, TCIOFLUSH);
    // 获取串口参数opt
    tcgetattr(fd, &opt);
    //设置串口输出波特率
    cfsetospeed(&opt, B9600);
    //设置串口输入波特率
    cfsetispeed(&opt, B9600);
    
    //设置数据位数
    opt.c_cflag &= ~CSIZE;
    opt.c_cflag |= CS8;
    //校验位
    opt.c_cflag &= ~PARENB;
    opt.c_iflag &= ~INPCK;
    //设置停止位
    opt.c_cflag &= ~CSTOPB;
    //更新配置
    tcsetattr(fd, TCSANOW, &opt);
    printf("Device %s is set to 9600bps,8N1\n",path);

    //第四部分代码/
    //write函数串口发送字符串
    write(fd, send_buf, strlen(send_buf));
    int send_time=0; //发送次数
    while (1)
    {
        // 切换为printf串口输出（重定向）
        dup2(fd,STDOUT_FILENO);
        send_time++;
        printf("lubancat %s output message 第%d次\n",path,send_time);
        // 切换回printf标准输出（重定向）
        dup2(save_fd,STDOUT_FILENO);
        printf("完成%d次串口数据发送\n",send_time);
        sleep(2);
    }
    
    close(fd);
    return 0;
}