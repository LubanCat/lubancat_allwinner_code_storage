#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>  //杂项设备相关头文件
#include <linux/fs.h>          //文件系统头文件
//宏定义杂项设备名称
#define DEV_NAME        "misc_dev"

/*定义file_operations结构体(文件操作集)*/
struct file_operations misc_char_fops = {
    .owner = THIS_MODULE, //表示该文件的操作结构体所属的模块是当前的模块(所有者是这个模块)
};
/*杂项设备结构体*/
struct miscdevice misc_char_dev = {
    .minor = MISC_DYNAMIC_MINOR, //动态申请的次设备号
    .name  = DEV_NAME, //杂项设备名称
    .fops  = &misc_char_fops, //文件操作集
};


/*入口函数功能实现杂项字符设备初始化*/
static int __init misc_chardev_init(void)
{
    int ret = 0;
    printk("misc_chrdev init start\n");
    /*注册杂项设备*/
    ret = misc_register(&misc_char_dev);
    if(ret < 0){
        printk("fail to alloc misc_chrdev\n");
        goto alloc_err;
    }
    printk("misc_chrdev init succeed\n");
    return 0;

alloc_err:
    return ret;
}
module_init(misc_chardev_init);

/*出口函数功能实现*/
static void __exit misc_chardev_exit(void)
{   
    /*注销杂项设备*/
    misc_deregister(&misc_char_dev);
    printk("misc_chrdev exit\n");
}
module_exit(misc_chardev_exit);

MODULE_LICENSE("GPL v2");