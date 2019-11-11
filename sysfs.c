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


#define __NO_VERSION__
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/fcntl.h>
#include <linux/file.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/of_device.h>
#include <linux/sysfs.h>

#include <include/io.h>
#include <include/global.h>
typedef struct file FILE;

// extern funcs
extern void sendfan(void);
//extern void setpin(int id, int val);

// extern variables
extern unsigned char* fanbuf;
extern unsigned char* GPIO;

static ssize_t pwm_show(struct device *dev, struct device_attribute *attr, char *buf)
{
 int id = 0;
 struct attribute* att = &attr->attr;
 sscanf(att->name, "pwm%d", &id);

 if (id > DEVCHS) {
  printk("HWPWM: Illegal device access (exceeded channel limit)\n");
  return 0;
 }

 sprintf(buf,"%d\n",fanbuf[id-1]);
 return strlen(buf);
}

static ssize_t pwm_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
 int id = 0;
 struct attribute* att = &attr->attr;
 sscanf(att->name, "pwm%d", &id);

 if (id > DEVCHS) {
  printk("HWPWM: Illegal device access (exceeded channel limit)\n");
  return PAGE_SIZE;
 }

 int tmp;
 sscanf(buf, "%d", &tmp);
 if (tmp > 0xFF) { return PAGE_SIZE; }
 printk(KERN_INFO "PWM%d set to %d\n",id,tmp);
 fanbuf[id-1] = 0;
 fanbuf[id-1] += tmp;
 sendfan();
 return PAGE_SIZE;
}

static ssize_t gpio_show(struct device *dev, struct device_attribute *attr, char *buf)
{
 int id = 0;
 struct attribute* att = &attr->attr;
 sscanf(att->name, "gpio%d", &id);

 if (id >= DEVCHSG) {
  printk("HWPWM: Illegal device access (exceeded channel limit)\n");
  return 0;
 }

 sprintf(buf,"0\n");
 return strlen(buf);
}

static ssize_t gpio_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
 int id = 0;
 struct attribute* att = &attr->attr;
 sscanf(att->name, "gpio%d", &id);

 if (id >= DEVCHSG) {
  printk("HWPWM: Illegal device access (exceeded channel limit)\n");
  return PAGE_SIZE;
 }

 int tmp;
 sscanf(buf, "%d", &tmp);

 printk(KERN_INFO "GPIO%d set to %d\n",id,tmp);
 setpin(id,tmp);
 return PAGE_SIZE;
}

#define FPERM 0644 // sysfs file perm

static const DEVICE_ATTR(pwm1,   		FPERM       ,	    pwm_show               , pwm_store);
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

static const DEVICE_ATTR(gpio0,         FPERM       ,       gpio_show               , gpio_store);
static const DEVICE_ATTR(gpio1,         FPERM       ,       gpio_show               , gpio_store);
static const DEVICE_ATTR(gpio2,         FPERM       ,       gpio_show               , gpio_store);
static const DEVICE_ATTR(gpio3,         FPERM       ,       gpio_show               , gpio_store);
static const DEVICE_ATTR(gpio4,         FPERM       ,       gpio_show               , gpio_store);
static const DEVICE_ATTR(gpio5,         FPERM       ,       gpio_show               , gpio_store);
static const DEVICE_ATTR(gpio6,         FPERM       ,       gpio_show               , gpio_store);
static const DEVICE_ATTR(gpio7,         FPERM       ,       gpio_show               , gpio_store);
static const DEVICE_ATTR(gpio8,         FPERM       ,       gpio_show               , gpio_store);
static const DEVICE_ATTR(gpio9,         FPERM       ,       gpio_show               , gpio_store);
static const DEVICE_ATTR(gpio10,        FPERM       ,       gpio_show               , gpio_store);
static const DEVICE_ATTR(gpio11,        FPERM       ,       gpio_show               , gpio_store);
static const DEVICE_ATTR(gpio12,        FPERM       ,       gpio_show               , gpio_store);
static const DEVICE_ATTR(gpio13,        FPERM       ,       gpio_show               , gpio_store);
static const DEVICE_ATTR(gpio14,        FPERM       ,       gpio_show               , gpio_store);
static const DEVICE_ATTR(gpio15,        FPERM       ,       gpio_show               , gpio_store);


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

 device_create_file(pdev, &dev_attr_gpio0);
 device_create_file(pdev, &dev_attr_gpio1);
 device_create_file(pdev, &dev_attr_gpio2);
 device_create_file(pdev, &dev_attr_gpio3);
 device_create_file(pdev, &dev_attr_gpio4);
 device_create_file(pdev, &dev_attr_gpio5);
 device_create_file(pdev, &dev_attr_gpio6);
 device_create_file(pdev, &dev_attr_gpio7);
 device_create_file(pdev, &dev_attr_gpio8);
 device_create_file(pdev, &dev_attr_gpio9);
 device_create_file(pdev, &dev_attr_gpio10);
 device_create_file(pdev, &dev_attr_gpio11);
 device_create_file(pdev, &dev_attr_gpio12);
 device_create_file(pdev, &dev_attr_gpio13);
 device_create_file(pdev, &dev_attr_gpio14);
 device_create_file(pdev, &dev_attr_gpio15);


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

 device_remove_file(pdev, &dev_attr_gpio0);
 device_remove_file(pdev, &dev_attr_gpio1);
 device_remove_file(pdev, &dev_attr_gpio2);
 device_remove_file(pdev, &dev_attr_gpio3);
 device_remove_file(pdev, &dev_attr_gpio4);
 device_remove_file(pdev, &dev_attr_gpio5);
 device_remove_file(pdev, &dev_attr_gpio6);
 device_remove_file(pdev, &dev_attr_gpio7);
 device_remove_file(pdev, &dev_attr_gpio8);
 device_remove_file(pdev, &dev_attr_gpio9);
 device_remove_file(pdev, &dev_attr_gpio10);
 device_remove_file(pdev, &dev_attr_gpio11);
 device_remove_file(pdev, &dev_attr_gpio12);
 device_remove_file(pdev, &dev_attr_gpio13);
 device_remove_file(pdev, &dev_attr_gpio14);
 device_remove_file(pdev, &dev_attr_gpio15);

 return 0;
}
