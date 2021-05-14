#ifndef _PIT_H
#define _PIT_H

#define PIT_COMMAND_PORT 0x43
#define PIT_DATA_PORT_CHANNEL0 0x40
#define PIT_DATA_PORT_CHANNEL1 0x41
#define PIT_DATA_PORT_CHANNEL2 0x42
#define pit_irq_number 0

#define PIT_MODE 0x37 
#define LATCH (1193180 + (100/2)) / 100//11932  //(1193180 + (10/2)) / 10//11932  
#include "types.h"
extern int32_t scheduled_index;
void init_pit(void);
void pit_handler(void);
void scheduling(void);
void flush_TLB(void);
void process_video_switch(void);
int32_t init_shells(const uint8_t* command);
void process_switch(void);
extern int is_process_switch;




#endif







