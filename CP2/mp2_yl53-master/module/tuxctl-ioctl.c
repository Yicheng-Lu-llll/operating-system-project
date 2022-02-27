/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

void __handle_intrrrupt__(unsigned b,unsigned c );
void __handle_reset__(struct tty_struct* tty);
int __TUX_INIT__(struct tty_struct* tty);
int __GET_BUTTONS__(unsigned long arg);
int __SET_LED__(struct tty_struct* tty,unsigned long arg);









int ACK;
unsigned long pressed_button=0xFFFFFFFF;
unsigned char led_buffer[6];
unsigned long led_status=0;
unsigned char led_number [16] = {0xE7, 0x06, 0xCB, 0x8F, 0x2E, 0xAD, 0xED, 0x86, 0xEF, 0xAF, 0xEE, 0x6D, 0xE1, 0x4F, 0xE9, 0xE8};





/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    
    unsigned a, b, c;

    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];

    /*printk("packet : %x %x %x\n", a, b, c); */
    switch(a)//a is n byte 0 containing opcode
    {
        case MTCP_ACK:                  //MTCP_ACK is returned meaning tux is free now.
            ACK=free;      
            return;
        case MTCP_BIOC_EVENT:
                                           //Generated when the Button Interrupt-on-change mode is enabled and 
            return __handle_intrrrupt__(~b,~c);              //a button is either pressed or released.
          
        case MTCP_RESET:  	                    //Generated when the devide re-initializes itself after a power-up, 
            return __handle_reset__(tty);       //a RESET button press, or an MTCP_RESET_DEV command.

		default:
		 	return;
    }
}

/* 
 * __handle_intrrrupt__
 *   DESCRIPTION: give button information to kernel.
 *   INPUTS: packet[1],packet[2]
 *   OUTPUTS: none
 *   RETURN VALUE: 
 *   SIDE EFFECTS: none
 */
//Generated when the Button Interrupt-on-change mode is enabled and 
//a button is either pressed or released.
//input:
// ;		byte 1  __7_____4___3___2___1_____0____
// ;			| 1 X X X | C | B | A | START |
// ;			-------------------------------
// ;		byte 2  __7_____4_____3______2______1_____0___
// ;			| 1 X X X | right | down | left | up |
//output:
//;right left down up c b a start
void __handle_intrrrupt__(unsigned b,unsigned c ){
    // int right=(c>>3)%2;//this is becasue 'right' is the fouth point. we need to shift three times and throw away other bits.
    // int down=(c>>2)%2;
    // int left=(c>>1)%2;
    // int up=c%2;
    // pressed_button=(right<<7) + (left<<6) + (down<<5) + (up<<4) +(b & 0x0F);//(b & 0x0F) is used to clear the high bits in b
    // pressed_button=~pressed_button;


	// unsigned int status_of_L;
	// unsigned int status_of_D;
	unsigned int left = (c>>1)%2;//this is becasue 'left' is the second point. we need to shift one time and throw away other bits.
	unsigned int down = (c>>2)%2;
    //
	pressed_button = ~((((b & 0x0F) | ((c & 0x0F) << 4)) & 0x9F) | (down << 5) | (left << 6));


	

}    



/* 
 * __handle_intrrrupt__
 *   DESCRIPTION: reset and reload the original led number.
 *   INPUTS: tty
 *   OUTPUTS: none
 *   RETURN VALUE: 
 *   SIDE EFFECTS: none
 */
void __handle_reset__(struct tty_struct* tty){
    unsigned char buf[2]={MTCP_BIOC_ON,MTCP_LED_USR};//contain the opcode to turn on interrupts and turn the LED to user mode
    if(ACK==busy){//if tux is busy, do nothing
        return;
    }
   
    tuxctl_ldisc_put(tty,buf,2);
    ACK=busy;
    __SET_LED__(tty,led_status);//reload the led number.
    return;
}


/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/



int 
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{
    switch (cmd) {
	case TUX_INIT:        return __TUX_INIT__(tty);
	case TUX_BUTTONS:     return __GET_BUTTONS__(arg);
	case TUX_SET_LED:     return __SET_LED__(tty,arg);
	case TUX_LED_ACK:     return 0;
	case TUX_LED_REQUEST: return 0;
	case TUX_READ_LED:    return 0;
	default:
	    return -EINVAL;
    }
}



/* 
 * __TUX_INIT__
 *   DESCRIPTION: initialize the tux
 *   INPUTS: tty
 *   OUTPUTS: none
 *   RETURN VALUE:  0
 *   SIDE EFFECTS: none
 */
int __TUX_INIT__(struct tty_struct* tty){
    unsigned char buf[2]={MTCP_BIOC_ON,MTCP_LED_USR};//contain the opcode to turn on interrupts and turn the LED to user mode
    if(ACK==busy){//if tux is busy, do nothing
        return 0;
    }
    
    tuxctl_ldisc_put(tty,buf,2);
    ACK=busy;
    return 0;

}

/* 
 * __GET_BUTTONS__
 *   DESCRIPTION: user call this funtion to get button data from kernel.
 *   INPUTS: arg is a pointer. we should store the pressed buttons information to that location. RTDC
 *   OUTPUTS: 0
 *   RETURN VALUE:  0
 *   SIDE EFFECTS: none
 */
//return the pressed buttons to user
//
int __GET_BUTTONS__(unsigned long arg){
    int ret;
    if(arg==0){return -EINVAL;}
    ret=copy_to_user((void *)arg , (void *)&pressed_button,sizeof(long));//copy from kernel to user,give user the button infomation.
    if(ret>0){
        return -EFAULT;}
    else{
        return 0;}
}


/* 
 * __SET_LED__
 *   DESCRIPTION: user call this funtion to set the LED number.
 *   INPUTS: tty, a 32-bit integer of the following form: The low 16-bits specify a number whose
hexadecimal value is to be displayed on the 7-segment displays. The low 4 bits of the third byte
specifies which LED’s should be turned on. The low 4 bits of the highest byte (bits 27:24) specify
whether the corresponding decimal points should be turned on
 *   OUTPUTS: 0
 *   RETURN VALUE:  0
 *   SIDE EFFECTS: none
 */
//user call it to set LED.
int __SET_LED__(struct tty_struct* tty,unsigned long arg){
 	unsigned char display[4];
 	unsigned char leds_on;
 	int  i;		     
 	unsigned char target_buf[6];     
 	unsigned char decimal_point= (arg & (0x0F << 24)) >> 24;
 	unsigned long bitmask = 0x000F;; 
    if(ACK==busy){
        return 0;
    }
    // use the 4 continous bit must to extract the four groups of hex number 
 	for(i = 0; i < 4; ++i, bitmask <<= 4){
 		display[i] = (bitmask & arg) >> (4*i);
 	}
    // get the led on/off bits
 	leds_on = (arg & (0x0F << 16)) >> 16;
 	target_buf[0] = MTCP_LED_USR;
 	tuxctl_ldisc_put(tty, &target_buf[0], 1);
 	target_buf[0] = MTCP_LED_SET;
 	target_buf[1] = 0x0F;
    // use the bitmusk to iterate through the mask bits for dot and LEDs
 	bitmask = 0x01;
    
 	for(i = 0; i < 4; i++, bitmask <<= 1)
 	{
 		if(leds_on & bitmask)
 		{
 			display[i] = led_number[display[i]];
 		}
 		else
 		{
 			target_buf[2+i] = 0x0;
 		}
        if(decimal_point & bitmask)
        {
 			display[i]|=0x10;
        }
 		target_buf[2+i]=display[i];
 	}
    //save led status
 	led_status=arg;
 	tuxctl_ldisc_put(tty, target_buf, 6);

	return 0;
}

