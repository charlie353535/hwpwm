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

#include "sysfs.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Charlie Camilleri");
MODULE_DESCRIPTION("Driver for FPGA PWM controller (ttySX) ");
MODULE_VERSION("12");

static char *PORT = "NOTTY";
module_param(PORT, charp, S_IRUGO);
MODULE_PARM_DESC(PORT, "Serial port to use (e.g. /dev/ttyS0) ");

typedef struct file FILE;

#define FAN_COUNT 16
#define FAN_DEFAULT (unsigned char)'~'

#define DEVICE_NAME "fpgafan0"
#define CLASS_NAME  "fpgafan"

unsigned char *fanbuf;
EXPORT_SYMBOL(fanbuf);
static FILE* filp;

static void sendb(unsigned char b) {
 loff_t pos = 0;

 unsigned char *buf = (unsigned char*)kmalloc(1,GFP_KERNEL);
 buf[0] = b;

 kernel_write(filp, buf, 1, &pos);

 kfree((void*)buf);
}

void sendfan(void) {
 for (unsigned char i=0; i<FAN_COUNT; i++) {
  sendb(i);
  sendb(fanbuf[i]);
 }
}

#define CDEV_MSG "fpgafan Copyright Charlie Camilleri 2019 \nThis device is useless, but required for the module to work!\n"
int ind = 0;
static ssize_t dev_read(struct file *filep, char *buf, size_t len, loff_t *offset){
   if (ind==1) {
    ind=0;
    return 0;
   }

   int err = 0;
   err = copy_to_user(buf, CDEV_MSG, strlen(CDEV_MSG));

   if (err>0) {
      printk(KERN_INFO "FPGAFAN: failed to send data to /dev/fpgafan0\n");
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

static int __init fpgafan_init(void){
 printk(KERN_INFO "fpgafan Copyright Charlie Camilleri 2019\n");

 if (!strcmp(PORT,"NOTTY")) {
  printk(KERN_ALERT "FPGAFAN: Please specify the PORT parameter\n");
  return -EINVAL;
 }

 fanbuf = (unsigned char*)kmalloc(FAN_COUNT, GFP_KERNEL);

 filp = filp_open(PORT, O_RDWR, 0);
 if (IS_ERR(filp)) {
  printk(KERN_ERR "FPGAFAN: Error opening %s!\n",PORT);
  return -ENODEV;
 }

 for (int i=0; i<FAN_COUNT; i++) {
  fanbuf[i] = FAN_DEFAULT;
 }

 sendfan();

 major = register_chrdev(0, DEVICE_NAME, &fops);
 if (major<0){
  printk(KERN_ALERT "FPGAFAN failed to register a major number\n");
  return major;
 }

 dclass = class_create(THIS_MODULE, CLASS_NAME);
 if (IS_ERR(dclass)){
  unregister_chrdev(major, DEVICE_NAME);
  printk(KERN_ALERT "FPGAFAN failed to register device class\n");
  return PTR_ERR(dclass);
 }

 fdevice = device_create(dclass, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
 if (IS_ERR(fdevice)){
  class_destroy(dclass);
  unregister_chrdev(major, DEVICE_NAME);
  printk(KERN_ALERT "Failed to create the device\n");
  return PTR_ERR(fdevice);
 }

 createattrs(fdevice);

 return 0;
}

static void __exit fpgafan_exit(void){
 printk(KERN_INFO "Unloading FPGAFAN\n");

 delattrs(fdevice);

 device_destroy(dclass, MKDEV(major, 0));
 class_unregister(dclass);
 class_destroy(dclass);
 unregister_chrdev(major, DEVICE_NAME);

 filp_close(filp, NULL);
}

module_init(fpgafan_init);
module_exit(fpgafan_exit);
