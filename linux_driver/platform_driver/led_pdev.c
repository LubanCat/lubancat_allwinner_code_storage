#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#define GPIO_BASE (0x0300B000) //GPIO(PC,PF,PG,PH,PI)组基地址
#define GPIO_PF_CFG (GPIO_BASE + 0x00B4) //PF Configure Register，设置输入或者输出
#define GPIO_PF_DAT (GPIO_BASE + 0x00C4) //PF Data Register，设置输出的电压

static struct resource led_resource[] = {
	[0] = DEFINE_RES_MEM(GPIO_PF_DAT, 4),
	[1] = DEFINE_RES_MEM(GPIO_PF_CFG, 4),
};

static void led_release(struct device *dev)
{
	
}
/* led hardware information */
unsigned int led_hwinfo[1] = { 6 };  //偏移，PF6

/*  led device */ 
static struct platform_device led_pdev = {
	.name = "led_pdev",
	.id = 0,
	.num_resources = ARRAY_SIZE(led_resource),
	.resource = led_resource,
	.dev = {
			.release = led_release,
			.platform_data = led_hwinfo,
		},
};

static __init int led_pdev_init(void)
{
	printk("pdev init\n");
	platform_device_register(&led_pdev);
	return 0;
}

module_init(led_pdev_init);

static __exit void led_pdev_exit(void)
{
	printk("pdev exit\n");
	platform_device_unregister(&led_pdev);
}

module_exit(led_pdev_exit);

MODULE_DESCRIPTION("Platform LED device");
MODULE_AUTHOR("embedfire");
MODULE_LICENSE("GPL");