obj-m := fpgafan12.o
fpgafan12-objs := fpgafan.o sysfs.o
ccflags-y := --std=gnu99 -Wno-declaration-after-statement

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	make unload
	make load

compile:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load:
	insmod fpgafan12.ko PORT="/dev/ttyV0"

unload:
	rmmod fpgafan12
