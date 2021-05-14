#include "systemfile.h"
#include "lib.h"
#include "system_call.h"
inode* inode_men_start;
single_data_block* data_block_men_start;

/* init_sysfile
 * 
 * This function is used to init file system by set the 
 * set the start add for inode and data block
 * Inputs: None
 * Outputs: 0 
 * Side Effects: None
 */
uint32_t init_sysfile(void){
    printf("Init SystemFile...\n");
    inode_men_start=(inode*)(sysfile_mem_start+1);//set add for start inode the add of one boot block after the start of system file 
    data_block_men_start=(single_data_block*)(sysfile_mem_start->inode_count+inode_men_start);//set the start add for data block by add sum of all index block

    return 0;

};
/* sysfile_open
 * 
 * This function is used to read file information with given name into buffer 
 * Inputs: filename 
 * Outputs: 0 for success, if filename not correct, return -1 
 * Side Effects: None
 */
int32_t sysfile_open(const uint8_t* filename)
{
    dentry buffer;
    int32_t is_name_vaild=read_dentry_by_name(filename,&buffer);
   
    if(is_name_vaild==FAIL ||  buffer.filetype!=2){//2 represent the regular file.
        return FAIL;
    }
        
    return 0;
}
/* sysfile_close
 * 
 * This function is used to close file 
 * Inputs: file descriptor
 * Outputs: 0 for success
 * Side Effects: None
 */
int32_t sysfile_close(int32_t fd){
    return 0;
}
/* sysfile_write
 * 
 * This function is used to write into file 
 * Inputs: file descriptor
 *         buf contain things to be write
 *         length to be write in 
 * 
 * Outputs: -1 as write is not allowed in read-only file
 * Side Effects: None
 */
int32_t sysfile_write(int32_t fd, const void* buf, int32_t nbytes){
    return FAIL;
}

/* sysfile_read
 * 
 * This function store the file information in target dentry and read the lenght of this file 
 * Inputs: fd
 *         offset
 *         length
 * 
 * Outputs: -1 for invalid, 0 for successfully read
 * Side Effects: None
 */
int32_t sysfile_read(int32_t fd, void* buf,uint32_t nbytes){
	// if(read_dentry_by_name((uint8_t*)fd, &tar_dentry) == FAIL){
        
	// 	return FAIL;
	// }
    if(fd>max_open_files ||fd<0 ){return FAIL;}
    uint32_t RESULT =read_data(current_PCB->file_array[fd].inode_index, current_PCB->file_array[fd].file_position, buf, nbytes);  
    if(RESULT==FAIL){return FAIL;}  
    current_PCB->file_array[fd].file_position=current_PCB->file_array[fd].file_position+RESULT;
	return RESULT;
}




/* read_dentry_by_name
 * 
 * This function is used to read file information by a filename given
 * return -1 on failure, indicating a non-existent file or invalid
 * index,fill in the dentry t block passed as their second argument with the file name, file
 * type, and inode number for the file, then return 0.
 * Inputs: filename
 *         empty dentry to store the information
 * Outputs: dentry with information like file type, filename and so on
 *
 * Side Effects: None
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry* tar_dentry){
    uint32_t length,dentry_num;
    int i,j;
    int is_match;
    int matched_index=-1;//-1 means we haven't find matched dentry
    uint8_t fname_buffer[33],cur_filename_buffer[33];// buffer for filename, plus 1 length for the last to maintaion \0
    dentry cur_entry;
    uint8_t* cur_filename;
    length=strlen((const int8_t*)fname);
    dentry_num=sysfile_mem_start->dir_count;//number of dentry  
    
    
    //if length of input filename is larger than filename buffer, return fail
    if (tar_dentry==NULL){//|| length>MAX_FILE_NAME_LEN
        
        return FAIL;
    }

    //this loop itrates dentrys
    for(i=0;i<dentry_num;i++){
        cur_entry=sysfile_mem_start->direntries[i];
        cur_filename=cur_entry.filename;//cur_filename is the current dentry's name. used to compare the fname.
        __create_buffer__(fname_buffer, cur_filename_buffer,fname, cur_filename );
        is_match=1;//just assume matching, if not, will change this later.
        matched_index=i;

        //loop the entire filename, check if match
        for(j=0;j<MAX_FILE_NAME_LEN;j++){
            if(fname_buffer[j]!=cur_filename_buffer[j]){
                is_match=0;      //means this
                matched_index=-1;//-1 means we haven't find matched dentry
                break;
            }
        }
        if(is_match==1){ //meaning we matches!
            break;
        }
    }

    //if matched, fill dentry
    if(is_match==1){
        strncpy((int8_t*)tar_dentry->filename,(int8_t*)sysfile_mem_start->direntries[matched_index].filename ,  sizeof(tar_dentry->filename));
        tar_dentry->filetype=sysfile_mem_start->direntries[matched_index].filetype;
        tar_dentry->inode_num=sysfile_mem_start->direntries[matched_index].inode_num;
        return 0; 
    }
    return FAIL;
    
}

/* __create_buffer__
 * 
 * This function is used to fill the buffer with the filename, one for 
 * target filename, another for current check filename
 * 
 * Inputs: fname_buffer
 *         cur_filename_buffer
 *         fname
 *         cur_filename
 * Outputs: None
 *
 * Side Effects: if length of any filename larger than 32, 
 * only the first 32 will be keep
 */
void __create_buffer__( uint8_t*  fname_buffer, uint8_t*  cur_filename_buffer,const uint8_t *  fname,uint8_t*  cur_filename ){
    int i;//index
    int fname_end=0;
    int cur_filename_end=0;
    for(i=0;i<(MAX_FILE_NAME_LEN+1);i++){
        if(fname[i]=='\0'){
            fname_end=1; //file name end
        }
        if(fname_end==0)
            fname_buffer[i]=fname[i];
        else
            fname_buffer[i]='\0';
    }
    fname_buffer[32]='\0';//last char must be '\0', 32 is the last index of the buffer.



    for(i=0;i<(MAX_FILE_NAME_LEN+1);i++){
        if(cur_filename[i]=='\0'){
            cur_filename_end=1; //current file name end
        }
        if(cur_filename_end==0)
            cur_filename_buffer[i]=cur_filename[i];
        else
            cur_filename_buffer[i]='\0'; // if file name end, fill all left with /\0
    }

}

/* read_dentry_by_index
 * 
 * This function is used to read file information by a index of the file dentry given
 * return -1 on failure, indicating a non-existent file or invalid
 * index,fill in the dentry t block passed as their second argument with the file name, file
 * type, and inode number for the file, then return 0.
 * Inputs: index
 *         empty dentry to store the information
 * Outputs: dentry with information like file type, filename and so on
 *
 * Side Effects: None
 */
int32_t read_dentry_by_index(uint32_t index, dentry* tar_dentry){
    int dentry_num;
    
    dentry_num=sysfile_mem_start->dir_count;//number of dentry 

    if(index>=dentry_num|| index<0){ //check the given index invalid?
        return FAIL;
    }
    strncpy((int8_t*)tar_dentry->filename, (int8_t*)sysfile_mem_start->direntries[index].filename , sizeof(tar_dentry->filename));//copy the filename of target filename to the output dentry
    tar_dentry->filetype=sysfile_mem_start->direntries[index].filetype;//copy the file type
    tar_dentry->inode_num=sysfile_mem_start->direntries[index].inode_num;//copy the inode place
    return 0; 

}
/* read_data
 * 
 * This function is used to read data into buffer,check that the given inode is within the
 * valid range. It does not check that the inode actually corresponds to a file (not all inodes are used). However, if a bad
 * data block number is found within the file bounds of the given inode, the function should also return -1.
 * Inputs: inode_idx, to find inode block contain all data block
 *         offset, # of data has been read
 *         buf, store read data
 *         length, totall data need to be read
 * Outputs: -1 if inode_index invalid, 0 for more data being read than need, otherwise, total read data length 
 *
 * Side Effects: None
 */
int32_t read_data(uint32_t inode_idx, uint32_t offset, uint8_t* buf , uint32_t length){
    inode* tar_node;
    //uint32_t end;
    int num_full_block,i;
    uint32_t num_read=0;
    uint32_t start_block_off;
    int start_data_block_index,end_data_block_index;
    single_data_block* start_data_block;
    single_data_block* end_data_block;
    uint32_t start_data_block_add,end_data_block_add;
    if(sysfile_mem_start->inode_count<=inode_idx){
        return FAIL; //check inode_index exsitence
    }
    tar_node=(inode*)(inode_men_start+inode_idx); //entry the target inode block with all data block
    if(offset>=tar_node->length){
        return 0;
    }
    if((offset+length)>tar_node->length){
        //end =tar_node->length-1;//data exist smaller than data need, only read what exist
        length=tar_node->length-offset;
    }
    // else {
    //     end=offset+length; 
    // }


    start_data_block_index=tar_node->data_block_num[offset/BLOCK_SIZE];//with each block contain 4K data, find the start data block num
    end_data_block_index=tar_node->data_block_num[(offset+length)/BLOCK_SIZE];//with each block contain 4K data, find the end data block num
    start_data_block = &data_block_men_start[start_data_block_index];//start data block
    end_data_block = &data_block_men_start[end_data_block_index];//end data block
    start_block_off=offset%BLOCK_SIZE;// the position to start in the start data block

    start_data_block_add=(uint32_t)&(tar_node->data_block_num[offset/BLOCK_SIZE]);//the add of start data blocjk in the inode block
    end_data_block_add=(uint32_t)&(tar_node->data_block_num[(offset+length)/BLOCK_SIZE]);//the add of end data block in the inode block
    if(length<BLOCK_SIZE-start_block_off){
        memcpy((void*)buf,(const void*)(((uint32_t)start_data_block) +start_block_off),length);
        num_read+=length; //only read several byte in the start block
        return length;
    }
    memcpy((void*)buf,(const void*)(((uint32_t)start_data_block) +start_block_off),BLOCK_SIZE-start_block_off); //first read all data remain in the start block
    num_read+=BLOCK_SIZE-start_block_off;
    if (start_data_block_add==end_data_block_add){ //if only one block need to be read
        return length;
    }
    if((end_data_block_add-start_data_block_add)==sizeof(start_data_block_add)){  //two block need to read
        memcpy((void*)(((uint32_t)buf)+num_read),(const void*)end_data_block,length-num_read);
        num_read+=length-num_read;
        return length;
    }
    else{
        num_full_block=(end_data_block_add-start_data_block_add)/sizeof(start_data_block_add)-1; //more than two block need to be read, all central block should be read all
        for(i=0;i<num_full_block;i++){
            
            
            memcpy(
                    (void*)         (((uint32_t)buf)+num_read),
                    (const void*)   (&data_block_men_start[(uint32_t)*(uint32_t*)(start_data_block_add+(i+1)*sizeof(start_data_block_add))]),
                    ( uint32_t )    BLOCK_SIZE
                    );
                                                                    
            num_read+=BLOCK_SIZE;
        }
        memcpy((void*)(((uint32_t)buf)+num_read),(const void*)end_data_block,length-num_read);
        num_read+=length-num_read;
        return length;

    }
    return FAIL;

}
/* sysdir_read
 * 
 * This function should only read one file name at a time
 * Inputs: offset, the file index 
 *         buf, store the filename
 *         nbytes, length to be read
 * Outputs: -1 if inode_index invalid, 0 for successfully read
 *
 * Side Effects: None
 */

int32_t sysdir_read(int32_t fd, void* buf, uint32_t  nbytes){
    //int32_t test=10/0;//used to test ecxeption handler
    if(fd>max_open_files ||fd<0 ){return FAIL;}
    dentry tar_dentry;
    int i;
    //if(current_PCB->file_array[fd].filename!=(uint8_t*)"."){return FAIL; }
    if(current_PCB->file_array[fd].file_position>=17){return 0;}//we have 17 files.

    int return_val= read_dentry_by_index(current_PCB->file_array[fd].file_position, &tar_dentry); //check return is valid?
	if(return_val== -1){
		return FAIL;
	}
    for (i = 0; i < MAX_FILE_NAME_LEN+1; i++){ 
        ((int8_t*)(buf))[i] = '\0'; // init buf all with \0
    }

    strncpy((int8_t*)buf, (const int8_t*)tar_dentry.filename,strlen((int8_t*)tar_dentry.filename));
    ((int8_t*)(buf))[MAX_FILE_NAME_LEN] = '\0';//32 is the last position of a filename.
    current_PCB->file_array[fd].file_position++;
    if( (int32_t)strlen((int8_t*)tar_dentry.filename) ==MAX_FILE_NAME_LEN+1){return MAX_FILE_NAME_LEN;}
    return (int32_t)strlen((int8_t*)tar_dentry.filename);



}

/* sysdir_open
 * 
 * This function open file
 * Inputs: filename
 * Outputs: 0
 *
 * Side Effects: None
 */

int32_t sysdir_open(const uint8_t* filename)
{

    return 0;
}
/* sysdir_close
 * 
 * This function close file
 * Inputs: fd
 * Outputs: 0
 *
 * Side Effects: None
 */
int32_t sysdir_close(int32_t fd){
    return 0;
}
/* sysdir_write
 * 
 * This function write file
 * Inputs: fd
 * Outputs: -1 as write is not accept 
 *
 * Side Effects: None
 */
int32_t sysdir_write(int32_t fd, const void* buf, int32_t nbytes){
    return FAIL;
}

