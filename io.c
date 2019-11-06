/*
This file is part of FPGAFAN.

FPGAFAN is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

FPGAFAN is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with FPGAFAN.  If not, see <https://www.gnu.org/licenses/>.
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

//#include <include/sysfs.h>

typedef struct file FILE;

extern FILE* filp;
extern unsigned char* fanbuf;

extern int PROTOCOL;
extern int FAN_COUNT;

void sendb(unsigned char b) {
 loff_t pos = 0;

 unsigned char *buf = (unsigned char*)kmalloc(1,GFP_KERNEL);
 buf[0] = b;

 kernel_write(filp, buf, 1, &pos);

 kfree((void*)buf);
}

unsigned char recvb(void) {
 loff_t pos = 0;
 unsigned char b;
 kernel_read(filp, &b, 1, &pos);
 return b;
}

void sendfan(void) {
 for (unsigned char i=0; i<FAN_COUNT; i++) {
  if (PROTOCOL == 2) {
   sendb(fanbuf[i]);
  }
  if (PROTOCOL == 1 || PROTOCOL == 3) {
   sendb(i);
   sendb(fanbuf[i]);
  }
 }
}

void setreg(unsigned char reg, unsigned char val) {
 sendb(reg); sendb(val);
}

unsigned char readreg(unsigned char reg) {
 sendb(0xFF); sendb(reg);
 return recvb();
}

unsigned char GPIO[2];

#define LOW 0
#define HIGH 1

void setpin(int id, int val) {
 if (id>7) {
  if (val>0) { GPIO[1] |=  (1UL << (id-8) ); }
  if (val==0){ GPIO[1] &= ~(1UL << (id-8) ); }
 }
 else {
  if (val>0) { GPIO[0] |=  (1UL << (id) ); }
  if (val==0){ GPIO[0] &= ~(1UL << (id) ); }
 }
 setreg(16,GPIO[0]);
 setreg(17,GPIO[1]);
 //printk(KERN_INFO "GPIO0 = 0x%X, GPIO1 = 0x%X\n",GPIO[0],GPIO[1]);
}

// "arduino compatability" functions (why not?)
void digitalWrite(int pin, int value) {
 if (pin > 16) { return; }
 return setpin(pin-1,value);
}

void analogWrite(int pin, unsigned char value) {
 if (pin > 16) { return; }
 fanbuf[pin-1] = value;
}

char DEVNAME[9];
unsigned char RDONLYP = 0;

unsigned char regs[256];

char* devname(void) {
 return DEVNAME;
}

// register map
// 0-15 fans
// 16-17 GPIO
// 245 Read only region offset
// 246 Read only region policy
// 247-255 Device name

void initio(void) {
 if (PROTOCOL < 3) { return; }

 // Check RDONLY policy
 RDONLYP = readreg(246);
 if (RDONLYP==0) { printk(KERN_INFO "FPGAFAN device reported read only policy: \"ignore\" \n"); }
 if (RDONLYP==1) { printk(KERN_INFO "FPGAFAN device reported read only policy: \"read\" \n"); }
 regs[246] = RDONLYP;

 // Read device name (REGS 247 to 254)
 for (unsigned char i=0; i<8; i++) {
  DEVNAME[i] = (char)readreg(247+i);
 }
 printk(KERN_INFO "FPGAFAN device reported name: %s\n",DEVNAME);

 regs[245] = readreg(245);
 printk(KERN_INFO "FPGAFAN device reported read only region offset: 0x%X\n",regs[245]);

 // Set all GPIOs to LOW
 for (int i=0; i<16; i++) {
  setpin(i,0);
 }
}