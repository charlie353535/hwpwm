# HWPWM (renamed from FPGAFAN)
### Driver for FPGA-based PWM controllers
![](https://github.com/cvdcamilleri/hwpwm/workflows/build/badge.svg)

Currently, you can have *one* device on your machine. If you run out of PWM ports, just add more to the FPGA!

Really though, you could just as easily implement the controller on an arduino or the like.

Recommended version (see releases): **12.3.5** <br>
Latest Stable version:              **12.3.6** <br>
Latest Experimental version:        **12.5.0-pre1**

## PROTOCOLS

### Protocol 1
| 01        | FF            |  02        | FF            |  02        | FF            | et cetera.... | 
| --------- | ------------- | ---------- | ------------- | ---------- | ------------- | ------------- |
| Fan INDEX | Fan PWM Value |  Fan INDEX | Fan PWM Value |  Fan INDEX | Fan PWM Value |               |

- Send *ANY* fan index, then the corresponding value. To set fan 3 to 0x80, | 03 | 80 |

### Protocol 2
| FF            | FF            | FF            | et cetera.... |
| ------------- | ------------- | ------------- | ------------- |
| Fan PWM value | Fan PWM value | Fan PWM value |               |

- Send each PWM value sequentially, starting from zero and going to 16. Essentially, a big shift register.

### Protocol 3
**Protocol 3 has the transmission method as Protocol 1, but:**
- Setting register FF to XX returns the value of register 0xXX
- Registers 0xF7-FE are the device name (8 bytes)
- Registers 0x10-11 are GPIOs (each pin is a bit)
- Register  0xF6 is the read-only memory policy
- Register  0xF5 is the read-only memory offset
- Register  0xF4 is the number of PWM channels
- Register  0xF3 is the number of GPIOs
- Registers 0xF1-F2 contain a CRC16 CRC-CCITT (0xFFFF) checksum of the name

### YOU MUST SET THE SERIAL PORT BAUDRATE BEFORE USING THIS MODULE
# This is a very minimal documentation. The full documentation comes later.
