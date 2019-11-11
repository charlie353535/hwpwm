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
#include <linux/module.h>
#include <linux/gpio.h>
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

unsigned short crc16(unsigned char* data_p, unsigned char length)
{
    unsigned char x;
    unsigned short crc = 0xFFFF;

    while (length--)
    {
        x = crc >> 8 ^ *data_p++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x <<5)) ^ ((unsigned short)x);
    }
    return crc;
}

typedef struct file FILE;

unsigned char DEVCHS = 16;
unsigned char DEVCHSG = 16;

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
//EXPORT_SYMBOL(sendfan);

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

char DEVNAME[9] = "hwpwm0";
unsigned char RDONLYP = 0;
//unsigned char DEVCHS = 16;

unsigned char regs[256];

char* devname(void) {
 return DEVNAME;
}

// register map
// 0-15 fans
// 16-17 GPIO
// 241-242 CRC16 checksum of device name
// 243 Number of GPIOs (up to 16)
// 244 Number of fan channels (up to 16)
// 245 Read only region offset
// 246 Read only region policy
// 247-255 Device name

int initio(void) {
 if (PROTOCOL < 3) { return 0; }

 // Check number of fans
 DEVCHS = readreg(244);
 printk(KERN_INFO "HWPWM device reported number of channels: %d\n",DEVCHS);

 // Check number of GPIOs
 DEVCHSG = readreg(243);
 printk(KERN_INFO "HWPWM device reported number of GPIOs: %d\n",DEVCHSG);

 unsigned char crc[2];
 crc[0] = readreg(241);
 crc[1] = readreg(242);
 printk(KERN_INFO "HWPWM device reported name checksum of: %X%X\n",crc[0],crc[1]);

 // Check RDONLY policy
 RDONLYP = readreg(246);
 if (RDONLYP==0) { printk(KERN_INFO "HWPWM device reported read only policy: \"ignore\" \n"); }
 if (RDONLYP==1) { printk(KERN_INFO "HWPWM device reported read only policy: \"read\" \n"); }
 regs[246] = RDONLYP;

 // Read device name (REGS 247 to 254)
 for (unsigned char i=0; i<8; i++) {
  DEVNAME[i] = (char)readreg(247+i);
 }
 printk(KERN_INFO "HWPWM device reported name: %s\n",DEVNAME);

 unsigned short _crc = crc16(DEVNAME,8);
 if (crc[0] != (unsigned char)(_crc >> 8) || crc[1] != ((unsigned char)_crc & 0xFFFFFFFF)){
  printk(KERN_ALERT "HWPWM device reported invalid checksum! The device may be broken, bricked or just disconnected!\n");
  return -EIO;
 }

 regs[245] = readreg(245);
 printk(KERN_INFO "HWPWM device reported read only region offset: 0x%X\n",regs[245]);

 // Set all GPIOs to LOW
 for (int i=0; i<16; i++) {
  setpin(i,0);
 }
 return 0;
}
