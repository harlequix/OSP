KERNEL_VERSION	:= `uname -r`
KERNEL_DIR	:= /lib/modules/$(KERNEL_VERSION)/build
INSTALL_MOD_DIR	:= .

PWD		:= $(shell pwd)

obj-m		:= sarlkm.o 
sarlkm-objs     := sar.o

all: sarlkm

sarlkm:
	@echo "Building SAR module..."
	@(cd $(KERNEL_DIR) && make -C $(KERNEL_DIR) SUBDIRS=$(PWD) CROSS_COMPILE=$(CROSS_COMPILE) modules)

install:


clean: toolsclean
	-rm -f *.o *.ko .*.cmd .*.flags *.mod.c Module.symvers version.h modules.order
	-rm -rf .tmp_versions

tools:
	$(CC) -o hostnamesysctl -I$(KERNEL_DIR)/include hostnamesysctl.c

toolsclean:
	-rm -f hostnamesysctl

pack:
	tar cvf ../lab-2.tar ../src
