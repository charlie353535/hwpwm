obj-m += fpgafan.o
fpgafan-y := main.o sysfs.o
ccflags-y := --std=gnu99 -Wno-declaration-after-statement

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
