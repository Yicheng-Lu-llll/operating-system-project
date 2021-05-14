/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

#define MASK 0xFF
/*The linux file is in F:\ece391_share\work\source\linux-2.6.22.5\arch\ppc\syslib\i8259.c */

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = MASK; /* IRQs 0-7  */
uint8_t slave_mask = MASK;  /* IRQs 8-15 */

/* 
 * i8259_init
 *  DESCRIPTION: Mask all interrupts and exectute the initialize sequence of I8259.
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: Initialize the 8259 PIC.
 */
void i8259_init(void) {

    printf("Init i8259...\n");
    
    /* Mask all the things in 8259 */
    outb(MASK,MASTER_8259_DATA);
    outb(MASK,SLAVE_8259_DATA);

   /*Tell the 8259A the beginning of initialization*/ 
    outb(ICW1, MASTER_8259_PORT); //select 8259 master init
    outb(ICW1,SLAVE_8259_PORT);  //select 8259 slave init
    
    outb(ICW2_MASTER, MASTER_8259_DATA); //IR0-7 mapped to 0x20 - 0x27
    outb(ICW2_SLAVE, SLAVE_8259_DATA);   //IR0-7 mapped to 0x28 - 0x2f           
    
    outb(ICW3_MASTER, MASTER_8259_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_DATA);              
    
    outb(ICW4, MASTER_8259_DATA);//select 8086 mode
    outb(ICW4, SLAVE_8259_DATA);//select 8086 mode

    
    outb(master_mask,MASTER_8259_DATA);//restore master IRQ mask
    outb(slave_mask,SLAVE_8259_DATA);//restore slave IRQ mask

    enable_irq(SLAVE_IRQ_NUM);
}


/* 
 * enable_irq
 *  DESCRIPTION: tell i8259 to accept the interrup specified by irq_num
 *  INPUTS: irq_num -- range from 0 - 15
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: Enable (unmask) the specified IRQ
 */
void enable_irq(uint32_t irq_num) {
    /*check the range of irq_num. If out of range just return */
    if((irq_num > IRQ_NUM_MAX) || (irq_num < IRQ_NUM_MIN)){
        return;
    }

    /*0-7 is on master,8-15 is on slave*/
    if((irq_num < PORTS_NUMBER) && (irq_num >= IRQ_NUM_MIN)){
        master_mask &= ~(1 << irq_num);
        outb(master_mask,MASTER_8259_DATA);
        return;
    }
    else{
        slave_mask &= ~(1 << (irq_num - PORTS_NUMBER));       
        outb(slave_mask, SLAVE_8259_DATA);
        return;
    }
}

/* 
 * disable_irq
 *  DESCRIPTION: tell i8259 to disable the interrup specified by irq_num
 *  INPUTS: irq_num
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: Disable (mask) the specified IRQ
 */
void disable_irq(uint32_t irq_num) {
    /*check the range of irq_num. If out of range just return */
    if((irq_num > IRQ_NUM_MAX) || (irq_num < IRQ_NUM_MIN)){
        return;
    }

    /*0-7 is on master,8-15 is on slave*/
    if((irq_num < PORTS_NUMBER) && (irq_num >= IRQ_NUM_MIN)){
        master_mask |= 1 << irq_num;
        outb(master_mask,MASTER_8259_DATA);
        return;
    }
    else{
        slave_mask |= 1 << (irq_num-8);       
        outb(slave_mask, SLAVE_8259_DATA);
        return;
    }
}

/* 
 * send_eoi
 *  DESCRIPTION: Send end-of-interrupt signal for the specified IRQ
 *  INPUTS: irq_num -- range from 0 - 15
 *  OUTPUTS: none
 *  RETURN VALUE: none
 */
void send_eoi(uint32_t irq_num) {
    /*check the range of irq_num. If out of range just return */
    if((irq_num > IRQ_NUM_MAX) || (irq_num < IRQ_NUM_MIN)){
        return;
    }

    /*0-7 is on master,8-15 is on slave*/
    if((irq_num < PORTS_NUMBER) && (irq_num >= IRQ_NUM_MIN)){
        outb(EOI | irq_num, MASTER_8259_PORT); 
        return;
    }
    else{
    outb(EOI | (irq_num-PORTS_NUMBER), SLAVE_8259_PORT);     /* irq bigger than 8, slave */                                     
    outb(EOI | SLAVE_IRQ_NUM, MASTER_8259_PORT); 
        return;
    }
}

/*Version 1 ZLH*/



