obj-m := fpgafan12.o
fpgafan12-objs := fpgafan.o sysfs.o io.o
ccflags-y := --std=gnu99 -Wno-declaration-after-statement -I$(PWD)
PORT="/dev/ttyV0"

all: compile

test:
	-/usr/src/linux-headers-$(shell uname -r)/scripts/sign-file sha256 /root/mok/MOK.priv /root/mok/MOK.der fpgafan12.ko
	-stty -F $(PORT) $(cat ttysettings)
	-make unload
	make load

compile:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

generic:
	make -C /lib/modules/5.0.0-32-generic/build M=$(PWD) modules
	/usr/src/linux-headers-5.0.0-32-generic/scripts/sign-file sha256 /root/mok/MOK.priv /root/mok/MOK.der fpgafan12.ko

socat:
	socat -d -d PTY,link=/dev/ttyV0,echo=0,raw PTY,link=/dev/ttyV1,echo=0,raw

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load:
	insmod fpgafan12.ko PORT=$(PORT)

unload:
	rmmod fpgafan12
