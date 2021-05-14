#ifndef _SYSTEM_CALL_H
#define _SYSTEM_CALL_H


#include "types.h"


#define MAX_FD_NUM 8
#define MAX_FILE_LEN 32+1
#define BUF_SIZE 32+1//128
#define LEN_ELF 4
#define ELF_buf_size 4
#define pcb_array_size 6
#define mgc_num0 0x7f
#define mgc_num1 0x45
#define mgc_num2 0x4c
#define mgc_num3 0x46
#define user_page_idx 0x08000000>>22
#define user_page_start_address 0x08000000
#define file_data_base_add 0x08048000
#define kernel_end_add 0x800000
#define pcb_stack_size 0x2000
#define user_stack_size 0x400000

#define USER_SPACE_START 0x8000000
#define USER_SPACE_END   0x8400000
#define FILE_TYPE_RTC    0
#define RTC_INODE_INDEX  -1
#define NUM_DRIVERS      3

#define KERNAL_MEN_END   0x800000
#define KERNAL_STACK_SIZE 0x2000
//#define KERNEL_DS 0x0018
//#define TEMINAL_INDEX  
#define RTC_INDEX      0
#define FILE_INDEX     2
#define DIR_INDEX      1
#define avoid_page_fault_fence 4
#define EXCEPTION_STATUS  0x0F
#define EXCEPTION_RETURNVALUE 256
#define offset_22 22
#define user_PD_index 0x8000000 >>22
#define user_video_start_address 0x8400000
#define user_PT_index 0
typedef struct file_op_table{
    int32_t (*open)(const uint8_t* filename);
    int32_t (*close)(int32_t fd);
    int32_t (*read)(int32_t fd, void* buf,uint32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);

}file_op_table;


typedef struct fd_table{
    file_op_table ops;
    uint32_t inode_index;
    uint32_t file_position;
    uint32_t flags;
    uint8_t* filename;
}fd_table;





//Process Control Block, for CP2, PCB will only contain the file_array
typedef struct PCB{
    fd_table file_array[MAX_FD_NUM];

    int32_t pid;
    int32_t parent_pid;

    uint32_t tss_esp0;

    uint32_t esp;// previous one's esp
    uint32_t ebp;// previous one's ebp

    uint8_t* arg_buffer;
    uint8_t shell_indicator;

    uint32_t cur_esp;//cur one' esp
    uint32_t cur_ebp;//cur one' ebp
    ///////////
    //just for test
    uint32_t cur_esp0;//cur one' ebp
    //////////

    int32_t rtc_count_val;

}PCB;



extern PCB* PCB_array[pcb_array_size];
extern PCB* current_PCB;
extern int32_t program_count;


int32_t halt (uint8_t status);
void close_file_array(fd_table* file_array);
int32_t execute(const uint8_t* command);
void create_new_PCB(int8_t* arg, int32_t new_pid);
int32_t bad_call_open(const uint8_t* filename);
int32_t bad_call_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t bad_call_close(int32_t fd);
int32_t bad_call_read(int32_t fd, void* buf,uint32_t length);
int32_t split_argument(const uint8_t* command,int8_t* filename,int8_t* arg);
int32_t set_user_page(int32_t new_pid);
int32_t open(const uint8_t* filename);
int32_t close(int32_t fd);
int32_t read(int32_t fd, void* buf, uint32_t length);
int32_t write(int32_t fd, const void* buf, int32_t nbytes);


int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screen_start);
int32_t set_handler (int32_t signum, void* handler_address);
int32_t sigreturn (void);

//file_op_table get_terminal_fop(void);
file_op_table get_rtc_fop(void);
file_op_table get_file_fop(void);
file_op_table get_dir_fop(void);
int32_t init_fop_table(void);


#endif
