# fpgafan
Driver for FPGA-based PWM controllers

Really though, you could just as easily implement the controller on an arduino or the like.

### Protocols
##### Protocol 1:
   | 01        | FF            |  02        | FF            |  02        | FF            |   et cetera....   
   | Fan INDEX | Fan PWM Value |  Fan INDEX | Fan PWM Value |  Fan INDEX | Fan PWM Value |   
   Send *ANY* fan index, then the corresponding value. To set fan 3 to 0x80, | 03 | 80 |

##### Protocol 2:
   | FF            | FF            | FF            | et cetera....
	 
   | Fan PWM value | Fan PWM value | Fan PWM value |
	 
   Send each PWM value sequentially, starting from zero and going to 16.


## YOU MUST SET THE SERIAL PORT BAUDRATE BEFORE USING THIS MODULE
