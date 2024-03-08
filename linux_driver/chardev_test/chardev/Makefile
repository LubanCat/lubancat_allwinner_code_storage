KERNEL_DIR=../../../kernel/
# 指定工具链并导出环境变量
ARCH=arm64
CROSS_COMPILE=aarch64-linux-gnu-
export  ARCH  CROSS_COMPILE
# 编译成模块的目标文件名。
obj-m := chrdev.o
# 编译成测试程序的目标文件名
out =  chrdev_test

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules
	$(CROSS_COMPILE)gcc -o $(out) main.c

.PHONY:clean
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean
	rm $(out)
