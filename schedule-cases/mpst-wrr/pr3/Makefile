# This file has been automatically generated by gen_deployment.
# Do not make manual modifications there or they will be lost.

export POK_PATH=$(CURDIR)/../../..
export ARCH=x86
export BSP=x86-qemu
export DEPLOYMENT_HEADER=$(CURDIR)/deployment.h

include $(POK_PATH)/misc/mk/config.mk

TARGET = pr3.elf
OBJS = main.o activity.o

all: libpok $(TARGET)

.PHONY: clean distclean install all

clean: common-clean libpok-clean

include $(POK_PATH)/misc/mk/common-$(ARCH).mk
include $(POK_PATH)/misc/mk/rules-common.mk
include $(POK_PATH)/misc/mk/rules-partition.mk

