MODULE := virtual_kernel_device
INSTALL_DIR := /data/tmp
ANDROID_PROJECT_DIR := $(ANDROID_BUILD_TOP)
INCLUDEDIR :=$(ANDROID_PROJECT_DIR)/vendor/accenture/opengles2emulator/include
EXTRA_CFLAGS :=-I$(INCLUDEDIR) -fno-pic

PREBUILD:=$(ANDROID_PROJECT_DIR)/prebuilt/linux-x86/toolchain/arm-eabi-4.4.3
BIN := $(PREBUILD)/bin
KERNELDIR := $(ANDROID_PROJECT_DIR)/kernel

	obj-m := $(MODULE).o
	PWD := $(shell pwd)

default:
	$(MAKE) ARCH=arm EXTRA_CFLAGS="$(EXTRA_CFLAGS)" CROSS_COMPILE=$(BIN)/arm-eabi- -C $(KERNELDIR) M=$(PWD) modules

install:
	adb push $(MODULE).ko $(INSTALL_DIR)/$(MODULE).ko

shell:
	adb shell
 
run:
	adb push $(ANDROID_PROJECT_DIR)/vendor/accenture/opengles2emulator/virtual_kernel_device/prebuilt/$(MODULE).ko $(INSTALL_DIR)/$(MODULE).ko
	adb shell insmod $(INSTALL_DIR)/$(MODULE).ko
 
br:
	$(MAKE) ARCH=arm EXTRA_CFLAGS="$(EXTRA_CFLAGS)" CROSS_COMPILE=$(BIN)/arm-eabi- -C $(KERNELDIR) M=$(PWD) modules
	adb push $(MODULE).ko $(INSTALL_DIR)/$(MODULE).ko
	adb shell insmod $(INSTALL_DIR)/$(MODULE).ko

unload:
	adb shell rmmod $(MODULE)

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

