obj-m := fpgafan12.o
fpgafan12-objs := fpgafan.o sysfs.o
ccflags-y := --std=gnu99 -Wno-declaration-after-statement -I$(PWD)

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	-/usr/src/linux-headers-$(shell uname -r)/scripts/sign-file sha256 /root/mok/MOK.priv /root/mok/MOK.der fpgafan12.ko
	-make unload
	make load

compile:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

generic:
	make -C /lib/modules/5.0.0-32-generic/build M=$(PWD) modules
	/usr/src/linux-headers-5.0.0-32-generic/scripts/sign-file sha256 /root/mok/MOK.priv /root/mok/MOK.der fpgafan12.ko

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load:
	insmod fpgafan12.ko PORT="/dev/ttyV0"

unload:
	rmmod fpgafan12
