KERNEL_DIR=../../kernel/
# KERNEL_DIR=/home/dev/hyw/aw-image-build/source/kernel/linux-5.4-h618/
ARCH=arm64
CROSS_COMPILE=aarch64-linux-gnu-
# CROSS_COMPILE=/home/dev/hyw/aw-image-build/toolchains/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-
export  ARCH  CROSS_COMPILE

obj-m := ws2812_drv.o

all:
		$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules

.PHONE:clean copy

clean:
		$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean
