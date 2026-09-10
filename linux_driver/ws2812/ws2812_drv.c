#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/spinlock.h>

#define DEV_NAME            			"ws2812"
#define LED_NUM_MAX						(20)
//#define DEBUG

/* H618 GPIO控制器基地址(PC/PF/PG/PH/PI等) */
#define GPIO_BASE_ADDR					UL(0x0300B000)

/* 每个端口寄存器组间距 */
#define GPIO_PORT_STRIDE				(0x24)
/* 端口内寄存器偏移 */
#define GPIO_CFG_OFFSET					(0x00)	/* Pn_CFG0，共4个: 0x00/0x04/0x08/0x0C */
#define GPIO_DAT_OFFSET					(0x10)	/* Pn_DAT，端口数据寄存器 */

static volatile unsigned int *GPIO_CFG_REG;		/* 功能/方向寄存器 */
static volatile unsigned int *GPIO_LEVEL_REG;	/* 数据寄存器 */

struct ws2812_mes {
    unsigned int gpiochip;      		// 端口号: PA=0 ... PI=8
    unsigned int gpionum;       		// 端口内引脚号 0~31
    unsigned int lednum;        		// 起始LED序号，从1开始
    unsigned int ledcount;      		// 要连续控制的LED数量
    unsigned char color[LED_NUM_MAX][3];	// 每个LED的 R:G:B，color[i][0]=R [1]=G [2]=B
};

static int major = 0;
static struct class *ws2812_class;
static int bit;
static unsigned int temp;

static DEFINE_SPINLOCK(ws2812_lock);

static int ws2812_drv_open(struct inode *node, struct file *file)
{
	return 0;
}

static void ws2812_reset(void)
{
	/* Reset: 拉低 > 280us */
	temp &= (~(1 << bit));
	*GPIO_LEVEL_REG = temp;
	udelay(500);
}

#define W1  *GPIO_LEVEL_REG = temp;
#define W2  W1 W1
#define W4  W2 W2
#define W8  W4 W4
#define W16 W8 W8

/*
 * 宏定义0码/1码，编译时内联展开
 *   T0H(3次)≈300ns  T0L(8次)≈800ns
 *   T1H(8次)≈800ns  T1L(3次)≈300ns
 */
#define WRITE_FRAME_0()  do {                  \
    temp |= 1 << bit;   /* 拉高 */              \
    W2 W1               /* T0H: 3次 */          \
    temp &= (~(1 << bit)); /* 拉低 */           \
    W8                 /* T0L: 8次 */         \
} while(0)

#define WRITE_FRAME_1()  do {                  \
    temp |= 1 << bit;   /* 拉高 */              \
    W8                 /* T1H: 8次 */         \
    temp &= (~(1 << bit)); /* 拉低 */           \
    W2 W1               /* T1L: 6次 */          \
} while(0)

static void ws2812_write_byte(unsigned char byte)
{
	int i = 0;

	for(i = 0; i < 8; i++)
	{
		if((byte << i) & 0x80)
			WRITE_FRAME_1();
	  	else
			WRITE_FRAME_0();
	}
}

static int ws2812_setup_gpio(unsigned int port, unsigned int pin)
{
	unsigned long port_base;
	unsigned int cfg_shift;

	if(port > 8)
	{
		printk(KERN_ERR"ws2812.gpiochip(port) must >= 0 && <= 8 (PA~PI)\n");
		return -1;
	}
	if(pin > 31)
	{
		printk(KERN_ERR"ws2812.gpionum(pin) must >= 0 && <= 31\n");
		return -1;
	}

	port_base = GPIO_BASE_ADDR + port * GPIO_PORT_STRIDE;

	GPIO_CFG_REG	= ioremap(port_base + GPIO_CFG_OFFSET + (pin / 8) * 4, 4);
	GPIO_LEVEL_REG	= ioremap(port_base + GPIO_DAT_OFFSET, 4);

#ifdef DEBUG
	printk("GPIO_CFG_REG : 0x%lX\n", port_base + GPIO_CFG_OFFSET + (pin / 8) * 4);
	printk("GPIO_LEVEL_REG : 0x%lX\n", port_base + GPIO_DAT_OFFSET);
#endif

	if(GPIO_LEVEL_REG == NULL || GPIO_CFG_REG == NULL)
	{
		printk(KERN_ERR"GPIO_LEVEL_REG or GPIO_CFG_REG is NULL\n");
		return -1;
	}

	bit = pin;

	cfg_shift = (pin % 8) * 4;
	temp = *GPIO_CFG_REG;
	temp &= (~(0xF << cfg_shift));
	temp |=  (0x1 << cfg_shift);
	*GPIO_CFG_REG = temp;

	temp = *GPIO_LEVEL_REG;
	temp &= (~(1 << bit));
	*GPIO_LEVEL_REG = temp;

	return 0;
}

static ssize_t ws2812_drv_write(struct file *filp, const char __user * buf, size_t count, loff_t * ppos)
{
	int err;
	struct ws2812_mes ws2812_usr;
	int i = 1;
	unsigned long flags;

	err = copy_from_user(&ws2812_usr, buf, sizeof(ws2812_usr));
	if(err != 0)
	{
		printk(KERN_ERR"get ws2812 struct err!\n");
		return err;
	}

#ifdef DEBUG
	printk("ws2812_usr.gpiochip(port) : %d\n", ws2812_usr.gpiochip);
	printk("ws2812_usr.gpionum(pin) : %d\n", ws2812_usr.gpionum);
	printk("ws2812_usr.lednum : %d\n", ws2812_usr.lednum);
	printk("ws2812_usr.ledcount : %d\n", ws2812_usr.ledcount);
	{
		int k;
		for(k = 0; k < ws2812_usr.ledcount; k++)
			printk("color[%d] : R=%d G=%d B=%d\n", k,
				ws2812_usr.color[k][0], ws2812_usr.color[k][1], ws2812_usr.color[k][2]);
	}
#endif

	if(ws2812_usr.lednum < 1 || ws2812_usr.lednum > LED_NUM_MAX)
	{
		printk(KERN_ERR"ws2812.lednum must >= 1 && <= %d\n", LED_NUM_MAX);
		return -1;
	}
	if(ws2812_usr.ledcount < 1 || ws2812_usr.ledcount > LED_NUM_MAX)
	{
		printk(KERN_ERR"ws2812.ledcount must >= 1 && <= %d\n", LED_NUM_MAX);
		return -1;
	}
	if(ws2812_usr.lednum + ws2812_usr.ledcount - 1 > LED_NUM_MAX)
	{
		printk(KERN_ERR"ws2812.lednum + ledcount must <= %d\n", LED_NUM_MAX);
		return -1;
	}

	err = ws2812_setup_gpio(ws2812_usr.gpiochip, ws2812_usr.gpionum);
	if(err != 0)
		return err;

	spin_lock_irqsave(&ws2812_lock, flags);

	ws2812_reset();

	{
		unsigned char first_byte = (ws2812_usr.lednum > 1) ? 0x00 : ws2812_usr.color[0][1];
		int j;

		if((first_byte << 0) & 0x80)
			WRITE_FRAME_1();
		else
			WRITE_FRAME_0();

		for(j = 1; j < 8; j++)
		{
			if((first_byte << j) & 0x80)
				WRITE_FRAME_1();
			else
				WRITE_FRAME_0();
		}
	}

	if(ws2812_usr.lednum > 1)
	{
		ws2812_write_byte(0x00);
		ws2812_write_byte(0x00);
		for(i = 2; i < ws2812_usr.lednum; i++)
		{
			ws2812_write_byte(0x00);
			ws2812_write_byte(0x00);
			ws2812_write_byte(0x00);
		}
		for(i = 0; i < ws2812_usr.ledcount; i++)
		{
			ws2812_write_byte(ws2812_usr.color[i][1]);
			ws2812_write_byte(ws2812_usr.color[i][0]);
			ws2812_write_byte(ws2812_usr.color[i][2]);
		}
	}
	else
	{
		ws2812_write_byte(ws2812_usr.color[0][0]);
		ws2812_write_byte(ws2812_usr.color[0][2]);
		for(i = 1; i < ws2812_usr.ledcount; i++)
		{
			ws2812_write_byte(ws2812_usr.color[i][1]);
			ws2812_write_byte(ws2812_usr.color[i][0]);
			ws2812_write_byte(ws2812_usr.color[i][2]);
		}
	}

	ws2812_reset();

	spin_unlock_irqrestore(&ws2812_lock, flags);

#ifdef DEBUG
	printk("ws2812 write over!\n");
#endif

	return 0;
}

static int ws2812_drv_close(struct inode *node, struct file *file)
{
	iounmap(GPIO_LEVEL_REG);
	iounmap(GPIO_CFG_REG);

	GPIO_LEVEL_REG = NULL;
	GPIO_CFG_REG = NULL;

	return 0;
}

static struct file_operations ws2812_fops = {
	.owner = THIS_MODULE,
	.open = ws2812_drv_open,
	.release = ws2812_drv_close,
	.write = ws2812_drv_write,
};

static __init int ws2812_init(void)
{
	printk(KERN_INFO"Load the ws2812 module successfully!(Allwinner H618)\n");

	major = register_chrdev(0, "aw_ws2812", &ws2812_fops);

	ws2812_class = class_create(THIS_MODULE, "aw_ws2812_class");
	if (IS_ERR(ws2812_class)) {
		printk(KERN_ERR"%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		unregister_chrdev(major, "aw_ws2812");
		return PTR_ERR(ws2812_class);
	}

	device_create(ws2812_class, NULL, MKDEV(major, 0), NULL, "ws2812");

	return 0;
}
module_init(ws2812_init);

static __exit void ws2812_exit(void)
{
	printk(KERN_INFO"the ws2812 module has been remove!\n");

	device_destroy(ws2812_class, MKDEV(major, 0));
	class_destroy(ws2812_class);
	unregister_chrdev(major, "aw_ws2812");

	if(GPIO_LEVEL_REG != NULL)
		iounmap(GPIO_LEVEL_REG);
	if(GPIO_CFG_REG != NULL)
		iounmap(GPIO_CFG_REG);
	GPIO_LEVEL_REG = NULL;
	GPIO_CFG_REG = NULL;
}
module_exit(ws2812_exit);

MODULE_AUTHOR("embedfire");
MODULE_LICENSE("GPL");
