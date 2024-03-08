KERNEL_DIR=../../../kernel/

ARCH=arm64
CROSS_COMPILE=aarch64-linux-gnu-
export  ARCH  CROSS_COMPILE

obj-m := chardev_n1.o

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules

.PHONY:clean
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean
