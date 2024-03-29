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
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/fcntl.h>
#include <linux/file.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/of_device.h>
#include <linux/sysfs.h>
#include <linux/path.h>
#include <linux/namei.h>

#include <include/global.h>

#ifdef USE_RPI_GPIO_PARALLEL
#define recvb p_recvb
#define sendb p_sendb
#else
#define recvb c_recvb
#define sendb c_sendb
extern char* PORT;
#endif

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

// not implemented yet, in progress
int c_checksz(void) {
/* struct path p;
 struct kstat ks;
 kern_path(PORT, 0, &p);
 vfs_getattr(&p, &ks);
 printk(KERN_INFO "########################## size: %lld\n", ks.size);
 return ks.size;*/
 return 0;
}


#define REQ_CLK 24 // Input (requests from gpio) clock
#define OUT_CLK 25 // Output (from gpio) clock

#define D0 2    // (maybe 0) Data lines x8
#define D1 3    // maybe 1 (blame raspberry pi foundation please)
#define D2 4
#define D3 17
#define D4 27   // maybe 21
#define D5 22
#define D6 18
#define D7 23

#define P_FREQ 2 // Parallell port frequency in Hz

void p_sendb(unsigned char b){
    unsigned char bits[8];
    for (int i=0; i<8; i++) {
        bits[i] = ((1 << (i % 8)) & (b)) >> (i % 8);
    }  
    
    gpio_direction_output(D0, bits[0]);
    gpio_direction_output(D1, bits[1]);
    gpio_direction_output(D2, bits[2]);
    gpio_direction_output(D3, bits[3]);
    gpio_direction_output(D4, bits[4]);
    gpio_direction_output(D5, bits[5]);
    gpio_direction_output(D6, bits[6]);
    gpio_direction_output(D7, bits[7]);

    gpio_direction_output(OUT_CLK, 1);

    mdelay((int)(1000/P_FREQ));

    gpio_direction_output(OUT_CLK, 0);

    gpio_direction_output(D0, 0);
    gpio_direction_output(D1, 0);
    gpio_direction_output(D2, 0);
    gpio_direction_output(D3, 0);
    gpio_direction_output(D4, 0);
    gpio_direction_output(D5, 0);
    gpio_direction_output(D6, 0);
    gpio_direction_output(D7, 0);
}

unsigned char p_recvb(void) {
    gpio_direction_output(REQ_CLK, 1);

    gpio_direction_input(D0);
    gpio_direction_input(D1);
    gpio_direction_input(D2);
    gpio_direction_input(D3);
    gpio_direction_input(D4);
    gpio_direction_input(D5);
    gpio_direction_input(D6);
    gpio_direction_input(D7);    

    unsigned char ret = 0;
    ret += gpio_get_value(D0) * ( 1 << 0 );
    ret += gpio_get_value(D1) * ( 1 << 1 );
    ret += gpio_get_value(D2) * ( 1 << 2 );
    ret += gpio_get_value(D3) * ( 1 << 3 );
    ret += gpio_get_value(D4) * ( 1 << 4 );
    ret += gpio_get_value(D5) * ( 1 << 5 );
    ret += gpio_get_value(D6) * ( 1 << 6 );
    ret += gpio_get_value(D7) * ( 1 << 7 );

    msleep((int)(1000/P_FREQ));

    gpio_direction_output(REQ_CLK, 1);

    return ret;

}

void p_init(void) {
    gpio_request(REQ_CLK, "REQ_CLK");
    gpio_request(OUT_CLK, "OUT_CLK");

    gpio_request(D0, "D0");
    gpio_request(D1, "D1");
    gpio_request(D2, "D2");
    gpio_request(D3, "D3");
    gpio_request(D4, "D4");
    gpio_request(D5, "D5");
    gpio_request(D6, "D6");
    gpio_request(D7, "D7");    
}

void p_exit(void) {
    gpio_free(REQ_CLK);
    gpio_free(OUT_CLK);

    gpio_free(D0);
    gpio_free(D1);
    gpio_free(D2);
    gpio_free(D3);
    gpio_free(D4);
    gpio_free(D5);
    gpio_free(D6);
    gpio_free(D7);
}

void c_sendb(unsigned char b) {
 loff_t pos = 0;

 unsigned char *buf = (unsigned char*)kmalloc(1,GFP_KERNEL);
 buf[0] = b;

 #ifdef IOWAIT
 #warning "IOWAIT is rather slow! only use it if it is REALLY required!"
 for (unsigned long long i=0; i<IOWAIT; i++) {
  __asm__("nop");
 }
 #endif

 kernel_write(filp, buf, 1, &pos);

 kfree((void*)buf);
}

unsigned char c_recvb(void) {
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

 c_checksz();

 if (readreg(247)==0) {
  printk(KERN_ALERT "HWPWM register 247 reads zero. I don't think that there is a device connected!\n");
  return -EIO;
 }

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
 #ifdef USE_RPI_GPIO_PARALLEL
 p_init();
 #endif
 return 0;
}

void exitio(void) {
    #ifdef USE_RPI_GPIO_PARALLEL
    p_exit();
    #endif
}
