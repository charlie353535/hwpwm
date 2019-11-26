obj-m := hwpwm12.o
hwpwm12-objs := hwpwm.o sysfs.o io.o
ccflags-y := --std=gnu99 -Wno-declaration-after-statement -I$(PWD)
PORT="/dev/ttyACM0"

all: compile

test:
	make compile
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
	sudo insmod hwpwm12.ko PORT=$(PORT)

unload:
	sudo rmmod hwpwm12
