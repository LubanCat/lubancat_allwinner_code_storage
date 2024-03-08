KERNEL_DIR=../../../kernel/
# 指定工具链并导出环境变量
ARCH=arm64
CROSS_COMPILE=aarch64-linux-gnu-
export  ARCH  CROSS_COMPILE
# 编译成模块的目标文件名。
obj-m := misc_chardev.o

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules

.PHONY:clean
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean

