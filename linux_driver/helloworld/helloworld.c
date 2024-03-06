#include <linux/init.h>  //包含宏定义的头文件
#include <linux/module.h>   //包含初始化加载模块的头文件

//入口函数功能实现
static int hello_init(void)
{
    //内核层只能使用printk，不能使用printf，因为内核层不支持C语言
    printk(KERN_EMERG "[ KERN_EMERG ]  Hello  World Init\n"); //输出等级为0
    printk("[ default ]  Hello  World Init\n");
    return 0;
}

//出口函数功能实现
static void hello_exit(void)
{
    printk(KERN_EMERG "[ KERN_EMERG ]  Hello  World Exit\n"); //输出等级为0
    printk("[ default ]   Hello  World Exit\n");
}

module_init(hello_init); //驱动入口
module_exit(hello_exit); //驱动出口

MODULE_LICENSE("GPL v2"); //声明开源许可证
// "GPL" 是指明 这是GNU General Public License的任意版本
// “GPL v2” 是指明 这仅声明为GPL的第二版本
// "GPL and addtional"
// "Dual BSD/GPL"
// "Dual MPL/GPL"
// "Proprietary"  私有的
MODULE_AUTHOR("embedfire"); //声明作者信息
MODULE_DESCRIPTION("hello world"); //对这个模块作一个简单的描述
MODULE_ALIAS("hello world_test");  //这个模块的别名