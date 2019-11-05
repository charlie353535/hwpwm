#define __NO_VERSION__
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

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Charlie Camilleri");
MODULE_DESCRIPTION("Driver for FPGA PWM controller (ttySX) ");
MODULE_VERSION("12");

//#include "../GPL3.h"

typedef struct file FILE;

// extern funcs
extern void sendfan(void);

// extern variables
extern unsigned char* fanbuf;

static ssize_t pwm_show(struct device *dev, struct device_attribute *attr, char *buf)
{
 int id = 0;
 struct attribute* att = &attr->attr;
 sscanf(att->name, "pwm%d", &id);

 sprintf(buf,"%d\n",fanbuf[id-1]);
 return strlen(buf);
}

static ssize_t pwm_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
 int id = 0;
 struct attribute* att = &attr->attr;
 sscanf(att->name, "pwm%d", &id);

 int tmp;
 sscanf(buf, "%d", &tmp);
 if (tmp > 0xFF) { return PAGE_SIZE; }
 printk(KERN_INFO "PWM1 set to %d\n",tmp);
 fanbuf[id-1] = 0;
 fanbuf[id-1] += tmp;
 sendfan();
 return PAGE_SIZE;
}

#define FPERM 0644 // sysfs file perm

static const DEVICE_ATTR(pwm1,		FPERM       ,	    pwm_show               , pwm_store);
static const DEVICE_ATTR(pwm2,          FPERM       ,       pwm_show               , pwm_store);
static const DEVICE_ATTR(pwm3,          FPERM       ,       pwm_show               , pwm_store);
static const DEVICE_ATTR(pwm4,          FPERM       ,       pwm_show               , pwm_store);
static const DEVICE_ATTR(pwm5,          FPERM       ,       pwm_show               , pwm_store);
static const DEVICE_ATTR(pwm6,          FPERM       ,       pwm_show               , pwm_store);
static const DEVICE_ATTR(pwm7,          FPERM       ,       pwm_show               , pwm_store);
static const DEVICE_ATTR(pwm8,          FPERM       ,       pwm_show               , pwm_store);
static const DEVICE_ATTR(pwm9,          FPERM       ,       pwm_show               , pwm_store);
static const DEVICE_ATTR(pwm10,         FPERM       ,       pwm_show               , pwm_store);
static const DEVICE_ATTR(pwm11,         FPERM       ,       pwm_show               , pwm_store);
static const DEVICE_ATTR(pwm12,         FPERM       ,       pwm_show               , pwm_store);
static const DEVICE_ATTR(pwm13,         FPERM       ,       pwm_show               , pwm_store);
static const DEVICE_ATTR(pwm14,         FPERM       ,       pwm_show               , pwm_store);
static const DEVICE_ATTR(pwm15,         FPERM       ,       pwm_show               , pwm_store);
static const DEVICE_ATTR(pwm16,         FPERM       ,       pwm_show               , pwm_store);

int createattrs(struct device* pdev) {
 printk(KERN_INFO "Populating SYSFS\n");

 device_create_file(pdev, &dev_attr_pwm1);
 device_create_file(pdev, &dev_attr_pwm2);
 device_create_file(pdev, &dev_attr_pwm3);
 device_create_file(pdev, &dev_attr_pwm4);
 device_create_file(pdev, &dev_attr_pwm5);
 device_create_file(pdev, &dev_attr_pwm6);
 device_create_file(pdev, &dev_attr_pwm7);
 device_create_file(pdev, &dev_attr_pwm8);
 device_create_file(pdev, &dev_attr_pwm9);
 device_create_file(pdev, &dev_attr_pwm10);
 device_create_file(pdev, &dev_attr_pwm11);
 device_create_file(pdev, &dev_attr_pwm12);
 device_create_file(pdev, &dev_attr_pwm13);
 device_create_file(pdev, &dev_attr_pwm14);
 device_create_file(pdev, &dev_attr_pwm15);
 device_create_file(pdev, &dev_attr_pwm16);

 return 0;
}

int delattrs(struct device* pdev) {
 device_remove_file(pdev, &dev_attr_pwm1);
 device_remove_file(pdev, &dev_attr_pwm2);
 device_remove_file(pdev, &dev_attr_pwm3);
 device_remove_file(pdev, &dev_attr_pwm4);
 device_remove_file(pdev, &dev_attr_pwm5);
 device_remove_file(pdev, &dev_attr_pwm6);
 device_remove_file(pdev, &dev_attr_pwm7);
 device_remove_file(pdev, &dev_attr_pwm8);
 device_remove_file(pdev, &dev_attr_pwm9);
 device_remove_file(pdev, &dev_attr_pwm10);
 device_remove_file(pdev, &dev_attr_pwm11);
 device_remove_file(pdev, &dev_attr_pwm12);
 device_remove_file(pdev, &dev_attr_pwm13);
 device_remove_file(pdev, &dev_attr_pwm14);
 device_remove_file(pdev, &dev_attr_pwm15);
 device_remove_file(pdev, &dev_attr_pwm16);

 return 0;
}
