#ifndef MOUSE_H
#define MOUSE_H


#include "lib.h"
#include "i8259.h"
#include "types.h"

#define MOUSE_PORT 0x64
#define BIT1_SET 2
#define enable_aux 0xA8
#define compaq_status 0x20
#define KEYBORD_PORT 0x60
#define BIT5_CLR 0xDF
#define Set_Compaq_Status 0x60
#define Command_Byte 0xD4
#define Set_Defaults 0xF6
#define Enable_Packet_Streaming 0XF4
#define Set_Sample_Rate 0XF3
#define MOUSE_IRQ 12
#define Negation_32bit 0xFFFFFF00


typedef union mouse_packet{

    uint8_t val;
    struct 
    {   
        uint8_t Left_Btn : 1; //set when left botton held down
        uint8_t Right_Btn :1; //set when right button held down
        uint8_t Middle_Btn :1; //set wehn middle button held down
        uint8_t Always_1 :1; // always set to 1 packet alignment
        uint8_t X_sign_bit :1;//set for a negative delta X
        uint8_t Y_sign_bit :1;// set for a negative delta y
        uint8_t X_overflow :1;// show X overflow, not used, if set, discard entire packet
        uint8_t Y_overflow :1;// show Y overflow, not used, if set, discard entire packet

    }__attribute__ ((packed));

} mouse_packet;




void init_mouse(void);
void mouse_handler(void);
void response_wait(void);
void read_wait(void);
void write_to_command(uint8_t command_id);
uint8_t read_from_command(void);


















#endif

