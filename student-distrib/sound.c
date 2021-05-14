#include "sound.h"



static int8_t  lower_page_registers[page_num]={page0_add,page1_add,page2_add,page3_add,page4_add,page5_add,page6_add,page7_add};//page 4 is not used
static uint32_t sound_inode=0;
static int8_t DMA_Buffer[DMA_Buffer_size] __attribute__((aligned(buffer_align))) = {};
static uint32_t offset;
static int8_t *target_block=DMA_Buffer;
static int8_t *block1 = DMA_Buffer;
static int8_t *block2 = &(DMA_Buffer[DMA_Buffer_size/2]);
int8_t play = 0;
void DSP_RESET(){
    uint8_t i=0;
    //Write a 1 to the reset port (2x6)
    outb(1,BASE_IO+reset_port);

    //Wait for 3 microseconds
    while (i<three_ms){
        i+=1;
    }

    //Write a 0 to the reset port (2x6)
    outb(0,BASE_IO+reset_port);

    //Poll the read-buffer status port (2xE) until bit 7 is set
    uint8_t read_buff_result=0;
    while(1){
        read_buff_result=inb(BASE_IO+read_buffer_status);
        if((read_buff_result>>shift_7)!=0){
            break;
        }
    }

    //Poll the read data port (2xA) until you receive an AA
    while(1){
        read_buff_result=inb(BASE_IO+read_data_port);
        if(read_buff_result==down_DSP_set_signal){
            break;
        }
    }
}



void DMA_Programing(uint8_t channel, uint8_t mode, uint32_t addr, uint32_t size){

    //Disable the sound card DMA channel by setting the appropriate mask bit
    if(channel<=channel_in_DMA1){
        outb((magic_four|channel),DAM1_bit_mask);
    }
    else{
        outb((magic_four|(channel%magic_four)),DAM2_bit_mask);
    }

    //Clear the byte pointer flip-flop
    if(channel<=channel_in_DMA1){
        outb(0,DMA1_clr_byte);
    }
    else{
        outb(0,DMA2_clr_byte);
    }

    //define DMA1_mode_reg 0x0B
    if(channel<=channel_in_DMA1){
        outb((mode|channel),DMA1_mode_reg);
    }
    else{
        outb((mode|(channel%magic_four)),DMA2_mode_reg);
    }

    //Write the offset of the buffer, low byte followed by high byte
    if(channel<=channel_in_DMA1){
        outb((addr>>shift_16),lower_page_registers[channel]);
        outb((addr & lower_2byte),  (channel << 1));
        outb(((addr >>shift_8) & lower_2byte), (channel << 1));
    }
    else{
        outb(((addr>>shift_16)&magic_set_page),lower_page_registers[channel]);
        outb((addr / magic_two) & lower_2byte, DMA_2_BASE + (channel << magic_two));
        outb(((addr / magic_two) >> shift_8) & lower_2byte, DMA_2_BASE + (channel << magic_two));
    }

    //Write the transfer length
    uint32_t count=size-1;
    if(channel<=channel_in_DMA1){
        outb(count & lower_2byte,   (channel << 1) + 1);
        outb((count >> 8) & 0xFF,   (channel << 1) + 1);
    }
    else{
        outb((count / magic_two) & lower_2byte, DMA_2_BASE + (channel << magic_two) + magic_two);
        outb(((count / magic_two) >> shift_8) & lower_2byte, DMA_2_BASE + (channel << magic_two) + magic_two);
    }

    //Enable the sound card DMA channel by clearing the appropriate mask bit
     if(channel<=channel_in_DMA1){
        outb(channel,DAM1_bit_mask);
    }
    else{
        outb((channel%magic_four),DAM2_bit_mask);
    }

}

void DSP_write(uint8_t data){
    volatile uint8_t read_buff_result=0;
    while(1){
        read_buff_result=inb(BASE_IO+write_buffer_status);
        if((read_buff_result>>shift_7)==0){
            break;
        }
    }
    outb(data,BASE_IO+write_buffer_status);
}

int DSP_read(){
    volatile uint8_t read_buff_result;
    while(1){
        read_buff_result=inb(BASE_IO+read_buffer_status);
        if((read_buff_result>>shift_7)==0){
            break;
        }
    }
    return inb(BASE_IO+Read_buffer_status);
}


void set_sampling_rate(uint16_t freq){
    //Write the command (41h for output rate, 42h for input rate)
    DSP_write(command_output_rate);
    //Write the high byte of the sampling rate (56h for 22050 hz)
    DSP_write((uint8_t)((freq>>shift_8)&lower_2byte));
    //Write the low byte of the sampling rate (22h for 22050 hz)
    DSP_write((uint8_t)(freq & lower_2byte));

}

int8_t play_back_sound(int8_t* filename){
    dentry sound_dentry;
    uint8_t magic_buf[magic_buf_size];
    char *RIFF="RIFF";
    int i;
    //if is playing, do nothing
    if(play){
        return 0;
    }
    int32_t res;
    //fill in the dentry
    res=read_dentry_by_name((uint8_t*)filename,&sound_dentry);
    if(res==-1){
        return FAIL;
    }
    sound_inode=sound_dentry.inode_num;
    //check the first 4 byte RIFF
    read_data(sound_inode,0,magic_buf,magic_buf_size);

    for(i=0;i<magic_buf_size;i++){
        if(magic_buf[i]!=RIFF[i]){
            return FAIL;
        }
    }
    DSP_RESET(); 
    open((uint8_t*)filename);
    uint32_t len = read_data(sound_inode,offset, (uint8_t*)DMA_Buffer,DMA_Buffer_size);
    DMA_Programing(1,Auto_mode,(uint32_t)(&DMA_Buffer[0]),sizeof(DMA_Buffer));
    set_sampling_rate(frequency);
    DSP_write(auto_out_command);
    DSP_write(play_mode);
    DSP_write((uint8_t)((DMA_Buffer_size/2-1) & lower_2byte));
    DSP_write((uint8_t)(((DMA_Buffer_size/2-1)>>8) & lower_2byte));
    offset+=len;
    play=1;
    return 0;
}



void sound_handler(){
    inb(BASE_IO+read_buffer_status);
    send_eoi(irq_sound);
    cli();
    uint32_t data_read=read_data(sound_inode,offset,(uint8_t*)target_block,DMA_Buffer_size/2);
    offset=offset+data_read;
    // if(play){
    //     return;
    // }
    sti();
    if (data_read==DMA_Buffer_size/2){
        uint16_t block_size=data_read;
        if(target_block==block1){
            target_block=block2;
        }
        else{
            target_block=block1;
        }
        DSP_write(auto_out_command);
        DSP_write(play_mode);
        DSP_write((uint8_t)((block_size-1) & lower_2byte));
        DSP_write((uint8_t)(((block_size-1)>>8) & lower_2byte));
        return;

    }
    else if(data_read==0){
        DSP_write(POSE_8);
        play=0;
        sound_inode=0;
        offset=0;
        target_block=block1;
    }
    return;
}







