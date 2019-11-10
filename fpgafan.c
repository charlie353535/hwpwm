/*
This file is part of HWPWM.

HWPWM is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

HWPWM is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with HWPWM.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
//#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <linux/file.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/of_device.h>
#include <linux/sysfs.h>

#include <include/sysfs.h>
#include <include/io.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Charlie Camilleri");
MODULE_DESCRIPTION("Driver for Hardware PWM controller (ttySX) ");
MODULE_VERSION("12.1");

static char *PORT = "NOTTY";
module_param(PORT, charp, S_IRUGO | S_IRUSR);
MODULE_PARM_DESC(PORT, "Serial port to use (e.g. /dev/ttyS0) ");

typedef struct file FILE;

int FAN_COUNT = 16;
#define FAN_DEFAULT 0xFF

#define DEVICE_NAME "hwpwm0"
#define CLASS_NAME  "hwpwm"

int PROTOCOL = 3; // Protocol to use
module_param(PROTOCOL, int, S_IRUGO | S_IRUSR);
MODULE_PARM_DESC(PROTOCOL, "Protocol to use (default: 3) ");

unsigned char *fanbuf;
EXPORT_SYMBOL(fanbuf);
FILE* filp;

/*
static void sendb(unsigned char b) {
 loff_t pos = 0;

 unsigned char *buf = (unsigned char*)kmalloc(1,GFP_KERNEL);
 buf[0] = b;

 kernel_write(filp, buf, 1, &pos);

 kfree((void*)buf);
}

void sendfan(void) {
 for (unsigned char i=0; i<FAN_COUNT; i++) {
  if (PROTOCOL == 2) {
   sendb(fanbuf[i]);
  }
  if (PROTOCOL == 1) {
   sendb(i);
   sendb(fanbuf[i]);
  }
 }
}
*/

const char *CDEV_MSG =	"HWPWM Copyright Charlie Camilleri 2019 \n" \
			" --> This device is useless, but required for the module to work! <--\n" \
			"\n" \
			"HWPWM is free software: you can redistribute it and/or modify\n" \
			"it under the terms of the GNU General Public License as published by\n" \
			"the Free Software Foundation, either version 3 of the License, or\n" \
			"(at your option) any later version.\n" \
			"\n" \
			"HWPWM is distributed in the hope that it will be useful, \n" \
			"but WITHOUT ANY WARRANTY; without even the implied warranty of \n" \
			"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the \n" \
			"GNU General Public License for more details. \n" \
			"\n" \
			"You should have received a copy of the GNU General Public License \n" \
			"along with HWPWM.  If not, see <https://www.gnu.org/licenses/>. \n" \
			"\n";

int ind = 0;
static ssize_t dev_read(struct file *filep, char *buf, size_t len, loff_t *offset){
   if (ind==1) {
    ind=0;
    return 0;
   }

   int err = 0;
   err = copy_to_user(buf, CDEV_MSG, strlen(CDEV_MSG));

   if (err>0) {
      printk(KERN_INFO "HWPWM: failed to send data to /dev/hwpwm0\n");
      return -EFAULT;
   }

   ind++;
   return (strlen(CDEV_MSG));
}

static struct file_operations fops =
{
 .read = dev_read,
};


EXPORT_SYMBOL(sendfan);

static struct device *fdevice;
static struct class *dclass;
static int major;

static int __init hwpwm_init(void){
 int ret=0;
 printk(KERN_INFO "HWPWM Copyright Charlie Camilleri 2019\n");

 if (!strcmp(PORT,"NOTTY")) {
  printk(KERN_ALERT "HWPWM: Please specify the PORT parameter\n");
  return -EINVAL;
 }

 fanbuf = (unsigned char*)kmalloc(FAN_COUNT, GFP_KERNEL);

 filp = filp_open(PORT, O_RDWR, 0);
 if (IS_ERR(filp)) {
  printk(KERN_ERR "HWPWM: Error opening %s!\n",PORT);
  return -ENODEV;
 }

 for (int i=0; i<FAN_COUNT; i++) {
  fanbuf[i] = FAN_DEFAULT;
 }

 ret += initio();
 sendfan();

 major = register_chrdev(0, DEVICE_NAME, &fops);
 if (major<0){
  printk(KERN_ALERT "HWPWM failed to register a major number\n");
  return major;
 }

 dclass = class_create(THIS_MODULE, CLASS_NAME);
 if (IS_ERR(dclass)){
  unregister_chrdev(major, DEVICE_NAME);
  printk(KERN_ALERT "HWPWM failed to register device class\n");
  return PTR_ERR(dclass);
 }

 fdevice = device_create(dclass, NULL, MKDEV(major, 0), NULL, devname());
 if (IS_ERR(fdevice)){
  class_destroy(dclass);
  unregister_chrdev(major, DEVICE_NAME);
  printk(KERN_ALERT "Failed to create the device\n");
  return PTR_ERR(fdevice);
 }

 createattrs(fdevice);

 if (ret>0) {
  delattrs(fdevice);

  device_destroy(dclass, MKDEV(major, 0));
  class_unregister(dclass);
  class_destroy(dclass);
  unregister_chrdev(major, DEVICE_NAME);

  filp_close(filp, NULL);
 }

 return ret;
}

static void __exit hwpwm_exit(void){
 printk(KERN_INFO "Unloading HWPWM\n");

 delattrs(fdevice);

 device_destroy(dclass, MKDEV(major, 0));
 class_unregister(dclass);
 class_destroy(dclass);
 unregister_chrdev(major, DEVICE_NAME);

 filp_close(filp, NULL);
}

module_init(hwpwm_init);
module_exit(hwpwm_exit);
