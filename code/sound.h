#ifndef SOUND_H
#define SOUND_H

#include "types.h"
#include "lib.h"
#include "systemfile.h"
#include "system_call.h"
#include "idt.h"
#include "i8259.h"


#define BASE_IO 0x220
#define reset_port 0x6
#define three_ms 30
#define read_buffer_status 0xE
#define shift_7 7
#define read_data_port 0xA
#define down_DSP_set_signal 0xAA
#define channel_in_DMA1 3
#define magic_four 4
#define DAM1_bit_mask 0x0A
#define DAM2_bit_mask 0xD4
#define DMA1_clr_byte 0x0C
#define DMA2_clr_byte 0xD8
#define DMA1_mode_reg 0x0B
#define DMA2_mode_reg 0xD8
#define page_num 8
#define page0_add 0x87
#define page1_add 0x83
#define page2_add 0x81
#define page3_add 0x82
#define page4_add 0x00
#define page5_add 0x8B
#define page6_add 0x89
#define page7_add 0x8A
#define shift_16 16
#define magic_set_page 0xFE
#define lower_2byte 0xFF
#define shift_8 8
#define magic_two 2
#define DMA_2_BASE 0xC0
#define write_buffer_status 0xC
#define Read_buffer_status 0XA
#define command_output_rate 0x41
#define magic_buf_size 4
#define Auto_mode 0x48
#define DMA_Buffer_size 64*1024
#define buffer_align 0x8000 
#define frequency 8500
#define auto_out_command 0xC6
#define play_mode 0
#define irq_sound 5
#define POSE_8    0xD0

extern int8_t play;


void DSP_RESET();

void DMA_Programing(uint8_t channel, uint8_t mode, uint32_t addr, uint32_t size);
void DSP_write(uint8_t data);
void set_sampling_rate(uint16_t freq);
int8_t play_back_sound(int8_t* filename);
int DSP_read();



















#endif

