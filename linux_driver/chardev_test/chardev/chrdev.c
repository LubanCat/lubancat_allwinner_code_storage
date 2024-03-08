#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#define DEV_NAME            "EmbedCharDev"
#define DEV_CNT                 (1)
#define BUFF_SIZE               128
//定义字符设备的设备号
static dev_t devno;
//定义字符设备结构体chr_dev
static struct cdev chr_dev;
//数据缓冲区
static char vbuf[BUFF_SIZE];
static int chr_dev_open(struct inode *inode, struct file *filp);
static int chr_dev_release(struct inode *inode, struct file *filp);
static ssize_t chr_dev_write(struct file *filp, const char __user * buf, size_t count, loff_t *ppos);
static ssize_t chr_dev_read(struct file *filp, char __user * buf, size_t count, loff_t *ppos);
// 定义file_operations结构体
static struct file_operations  chr_dev_fops = 
{
    .owner = THIS_MODULE,  //表示该文件的操作结构体所属的模块是当前的模块(所有者是这个模块)
    .open = chr_dev_open,       //应用层调用open函数，驱动层调用chr_dev_open函数
    .release = chr_dev_release, //应用层调用close函数，驱动层调用chr_dev_release函数
    .write = chr_dev_write,     //应用层调用write函数，驱动层调用chr_dev_write函数
    .read = chr_dev_read,       //应用层调用read函数，驱动层调用chr_dev_read函数
};
// chr_dev_open函数定义
static int chr_dev_open(struct inode *inode, struct file *filp)
{
    printk("\nopen\n");
    return 0;
}
// chr_dev_release函数定义
static int chr_dev_release(struct inode *inode, struct file *filp)
{
    printk("\nrelease\n");
    return 0;
}
// chr_dev_write函数定义
static ssize_t chr_dev_write(struct file *filp, const char __user * buf, size_t count, loff_t *ppos)
{
    unsigned long p = *ppos;
    int ret;
    int tmp = count ;
    if(p > BUFF_SIZE)
        return 0;
    if(tmp > BUFF_SIZE - p)
        tmp = BUFF_SIZE - p;
    ret = copy_from_user(vbuf, buf, tmp);
    *ppos += tmp;
    return tmp;
}
// chr_dev_read函数定义
static ssize_t chr_dev_read(struct file *filp, char __user * buf, size_t count, loff_t *ppos)
{
    unsigned long p = *ppos;
    int ret;
    int tmp = count ;
    if(p >= BUFF_SIZE)
        return 0;
    if(tmp > BUFF_SIZE - p)
        tmp = BUFF_SIZE - p;
    ret = copy_to_user(buf, vbuf+p, tmp);
    *ppos +=tmp;
    return tmp;
}
// 入口函数功能实现字符设备初始化
static int __init chrdev_init(void)
{
    int ret = 0;
    printk("chrdev init\n");
    //第一步
    //采用动态分配的方式，获取设备编号，次设备号为0，
    //设备名称为EmbedCharDev，可通过命令cat  /proc/devices查看
    //DEV_CNT为1，当前只申请一个设备编号
    ret = alloc_chrdev_region(&devno, 0, DEV_CNT, DEV_NAME);
    if(ret < 0){
        printk("fail to alloc devno\n");
        goto alloc_err;
    }
    //第二步
    //关联字符设备结构体cdev与文件操作结构体file_operations
    cdev_init(&chr_dev, &chr_dev_fops);
    //第三步
    //添加设备至cdev_map散列表中
    ret = cdev_add(&chr_dev, devno, DEV_CNT);
    if(ret < 0)
    {
        printk("fail to add cdev\n");
        goto add_err;
    }
    return 0;

add_err:
    //添加设备失败时，需要注销设备号
    unregister_chrdev_region(devno, DEV_CNT);
alloc_err:
    return ret;
}

module_init(chrdev_init);
// 出口函数功能实现
static void __exit chrdev_exit(void)
{
    printk("chrdev exit\n");
    // 注销设备号
    unregister_chrdev_region(devno, DEV_CNT);
    // 移除字符设备
    cdev_del(&chr_dev);
}
module_exit(chrdev_exit);

MODULE_LICENSE("GPL v2");
