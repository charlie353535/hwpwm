#ifndef IO_H
#define IO_H

extern void sendb(unsigned char b);
extern void sendfan(void);
extern void initio(void);
extern char* devname(void);
extern void setpin(int id, int val);

// Arduino-like compatibility (no reason, feel free to remove)
void analogWrite(int pin, unsigned char value);
void digitalWrite(int pin, int value);

// Variables
extern unsigned char DEVCHS; // Number of PWM channels
extern unsigned char DEVCHSG; // Number of GPIOs

#endif
