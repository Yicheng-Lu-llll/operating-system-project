#include "mouse.h"
#include "terminal.h"



int32_t x_p=0;
int32_t y_p=0;
int8_t right_button=0;
int8_t left_button=0;
int8_t middle_button=0;



void init_mouse(void){
    uint8_t status;
    response_wait();
    //Enable Auxiliary Device
    outb(enable_aux,MOUSE_PORT);
    response_wait();
    //enable the aux port to generate IRQ12
    outb(compaq_status,MOUSE_PORT);

    //waiting status bytes
    read_wait();
    status=inb(KEYBORD_PORT);
    status = status|BIT1_SET;//set bit number 1 (value=2, Enable IRQ12
    status = status&BIT5_CLR;// clear bit number 5 (value=0x20, Disable Mouse Clock).

    //Then send command byte 0x60 ("Set Compaq Status") to port 0x64
    response_wait();
    outb(Set_Compaq_Status,MOUSE_PORT);
    // followed by the modified Status byte to port 0x60
    response_wait();
    outb(status,KEYBORD_PORT);

    //Set Defaults	Disables streaming, sets the packet rate to 100 per second, and resolution to 4 pixels per mm.
    write_to_command(Set_Defaults);
    read_from_command();

    //Enable Packet Streaming	The mouse starts sending automatic packets when the mouse moves or is clicked.
    write_to_command(Enable_Packet_Streaming);
    read_from_command();

    //Set Sample Rate	Requires an additional data byte: automatic packets per second (see below for legal values).
    write_to_command(Set_Sample_Rate);
    read_from_command();
    response_wait();
    outb(200,KEYBORD_PORT);

    //enable irq
    enable_irq(MOUSE_IRQ);


}

void mouse_handler(void){
    int i = 0;
    send_eoi(MOUSE_IRQ);

    mouse_packet packet;
    //read packet from 0x60
    packet.val = read_from_command();

    //check packet vaild
    if(packet.Y_overflow==1 || packet.X_overflow==1 || packet.Always_1!=1){
        return;
    }
    //if valid, next 2 8byte will be x movement and y movement
    int32_t x_movement, y_movement; //8+1bit 2's complement
    //first get the 1 byte signless number
    x_movement=read_from_command();
    y_movement=read_from_command(); 
    //get sign from byte1 in packet
    if(packet.X_sign_bit==1){
        x_movement=(x_movement | Negation_32bit);
    }
    if(packet.Y_sign_bit==1){
        y_movement= (y_movement | Negation_32bit);
    }
    left_button=packet.Left_Btn;
    right_button=packet.Right_Btn;
    middle_button=packet.Middle_Btn;

    if(left_button){
        if (current_terminal_number == 0){
            i = 2;
        }
        if (current_terminal_number == 1){
            i = 0;
        }
        if (current_terminal_number == 2){
            i = 1;
        }
        switch_terminal(i);
    }

    if(right_button){
        if (current_terminal_number == 0){
            i = 1;
        }
        if (current_terminal_number == 1){
            i = 2;
        }
        if (current_terminal_number == 2){
            i = 0;
        }
        switch_terminal(i);
    }


}






void response_wait(void){
    int i = 50000;
    while(i-- && inb(MOUSE_PORT)&BIT1_SET); //All output to port 0x60 or 0x64 must be preceded by waiting for bit 1 (value=2) of port 0x64 to become clear
}

void read_wait(void){
    int i = 50000;
    while(i-- && inb(MOUSE_PORT)&1);// bytes cannot be read from port 0x60 until bit 0 (value=1) of port 0x64 is set
}
void write_to_command(uint8_t command_id){
    response_wait();
    outb(Command_Byte,MOUSE_PORT);
    response_wait();
    outb(command_id,KEYBORD_PORT);

}

uint8_t read_from_command(void){
    read_wait();
    return inb(KEYBORD_PORT);
}



