#ifndef _SYSTEMFILE_H
#define _SYSTEMFILE_H

#include "types.h"
#define RESERVE_boot_block 52
#define RESERVE_dentry 24
#define FILENAME_LEN 32
#define MAX_DENTRY 63
#define MAX_DATA_BLOCK_NUM 1023
#define BLOCK_SIZE 4096
#define MAX_FILE_NAME_LEN 32
#define FAIL -1
#define max_open_files 8


/*struct use for each d entry, at most 63 dentry can exist*/
typedef struct dentry
{
    uint8_t filename[FILENAME_LEN]; //filename
    uint32_t filetype; //type of file, 0 for rtc file; 1 for directory; 2 for regular file 
    uint32_t inode_num; //position of inode for this file
    uint8_t reserved[RESERVE_dentry];// 24B reserved
}dentry;
/*struct for boot block, only 1*/
typedef struct boot_block{
    uint32_t dir_count; //num of d entry exist, at max 63
    uint32_t inode_count; //num of inodes, smaller than num of d entry 
    uint32_t data_count;//num of data block each 4K
    uint8_t reserved[RESERVE_boot_block]; //52B rerserved
    dentry direntries[MAX_DENTRY]; // d entry array, each has struct dentry

}boot_block;
/*struct for inode*/
typedef struct inode{
    uint32_t length; //length for data in Byte
    uint32_t data_block_num[MAX_DATA_BLOCK_NUM];//data block array,each has type data block
}inode;
/*struct for data block size 4K*/
typedef struct single_data_block{
    uint8_t data[BLOCK_SIZE]; 
}single_data_block;


extern boot_block* sysfile_mem_start;
extern inode* inode_men_start;
extern single_data_block* data_block_men_start;


uint32_t init_sysfile(void);
int32_t sysfile_open(const uint8_t* filename);
int32_t sysfile_close(int32_t fd);
int32_t sysfile_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t sysfile_read(int32_t fd, void* buf,uint32_t nbytes);




int32_t sysdir_read(int32_t fd, void* buf, uint32_t  nbytes);
int32_t sysdir_open(const uint8_t* filename);
int32_t sysdir_close(int32_t fd);
int32_t sysdir_write(int32_t fd, const void* buf, int32_t nbytes);


int32_t read_dentry_by_name(const uint8_t* fname, dentry* tar_dentry);
void __create_buffer__( uint8_t*  fname_buffer, uint8_t*  cur_filename_buffer,const uint8_t *  fname,uint8_t*  cur_filename );
int32_t read_dentry_by_index(uint32_t index, dentry* tar_dentry);
int32_t read_data(uint32_t inode_idx, uint32_t offset, uint8_t* buf , uint32_t length);
// typedef struct file_op_table{
//     int32_t (*open)(const uint8_t* filename);
//     int32_t (*close)(int32_t fd);
//     int32_t (*read)(int32_t fd, void* buf,uint32_t length);
//     int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);

// }file_op_table;


// typedef struct fd_table{
//     file_op_table* ops;
//     uint32_t inode_index;
//     uint32_t file_position;
//     uint32_t flags;
// }fd_table;





// //Process Control Block, for CP2, PCB will only contain the file_array
// typedef struct PCB{
//     fd_table file_array[max_open_files];

// }PCB;



#endif



