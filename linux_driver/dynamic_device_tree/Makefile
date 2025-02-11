KERNEL_DIR=../../kernel/
# KERNEL_DIR=/home/guest/LubanCat_H618_SDK/aw-image-build/source/kernel/linux-5.4-h618/
ARCH=arm64
CROSS_COMPILE=aarch64-linux-gnu-
# CROSS_COMPILE=/home/guest/LubanCat_H618_SDK/aw-image-build/toolchains/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-
export  ARCH  CROSS_COMPILE

obj-m := led_test.o
CFLAGS_led_test.o := -fno-stack-protector
out = test_app


all:
		$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules
		$(CROSS_COMPILE)gcc -o $(out) test_app.c

.PHONE:clean

clean:
		$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean
		rm $(out)