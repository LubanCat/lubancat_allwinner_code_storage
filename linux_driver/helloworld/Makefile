KERNEL_DIR=../../kernel/
# KERNEL_DIR的路径得是内核源码路径

#声明编译的架构为arm64，编译器前缀为aarch64-linux-gnu-
ARCH=arm64
CROSS_COMPILE=aarch64-linux-gnu-
export  ARCH  CROSS_COMPILE
#obj-m：编译成模块
obj-m := helloworld.o
all:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules

#伪目标
.PHONE:clean
#指当make命令后紧跟clean时(即make clean)，执行以下伪目标clean对应的指令
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean