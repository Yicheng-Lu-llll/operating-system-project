#include "systemfile.h"
#include "system_call.h"
#include "x86_desc.h"
#include "terminal.h"
#include "page.h"
#include "lib.h"
#include "rtc.h"
#include "pit.h"

PCB* PCB_array[pcb_array_size]={NULL,NULL,NULL,NULL,NULL,NULL};
PCB* current_PCB = NULL;
int32_t program_count=0;

file_op_table fop_table[NUM_DRIVERS];




// file_op_table get_terminal_fop(void){
//     file_op_table ret;
//     ret.open = terminal_open;
//     ret.close = terminal_close;
//     ret.read = terminal_read;
//     ret.write = terminal_write;
//     return ret;
// }

/* 
 * get_rtc_fop(void)
 *   Description: the function used to get the open, close, read, write funtions of rtc
 *        Inputs: None
 *        Output: None
 *        Return: a data structure file_op_table which contains the open, close, read, write functions of rtc
 */
file_op_table get_rtc_fop(void){
    file_op_table ret;
    ret.open = rtc_open;
    ret.close = rtc_close;
    ret.read = rtc_read;
    ret.write = rtc_write;
    return ret;
}


/* 
 * get_file_fop(void)
 *   Description: the function used to get the open, close, read, write funtions of system_file
 *        Inputs: None
 *        Output: None
 *        Return: a data structure file_op_table which contains the open, close, read, write functions of system_file
 */

file_op_table get_file_fop(void){
    file_op_table ret;
    ret.open = sysfile_open;
    ret.close = sysfile_close;
    ret.read = sysfile_read;
    ret.write = sysfile_write;
    return ret;
}


/* 
 * get_dir_fop(void)
 *   Description: the function used to get the open, close, read, write funtions of sysdir
 *        Inputs: None
 *        Output: None
 *        Return: a data structure file_op_table which contains the open, close, read, write functions of sysdir
 */
file_op_table get_dir_fop(void){
    file_op_table ret;
    ret.open = sysdir_open;
    ret.close = sysdir_close;
    ret.read = sysdir_read;
    ret.write = sysdir_write;
    return ret;
}

/* 
 * init_fop_table(void)
 *   Description: init the fop table which should contain the open, close, read, write funcitons of all drivers
 *        Inputs: None
 *        Output: None
 *        Return: SUCCESS (0)
 */

int32_t init_fop_table(void){
    printf("Init FOP TABLE...\n");
    //fop_table[TEMINAL_INDEX] = get_terminal_fop();
    fop_table[RTC_INDEX] = get_rtc_fop();
    fop_table[FILE_INDEX] = get_file_fop();
    fop_table[DIR_INDEX] = get_dir_fop();
    return SUCCESS;
}










/* 
 * halt (uint8_t status)
 *   Description: System call which can terminate a process
 *        Inputs: status: which will be set in eax --> the real return value
 *        Output: None
 *        Return: fake return value --> SUCCESS (0)
 */
int32_t halt (uint8_t status){
    int32_t i;
    //int32_t flag;
    cli();

    
    


    /* free the current process in bitmap*/
    if (current_PCB == NULL){
        return FAIL;
    }
    
    program_count = program_count-1;

    /* deal with the occasion of the base shell */
    if ((current_PCB -> parent_pid) == -1){
        PCB_array[current_PCB->pid] = NULL;
        printf("This is a base shell\n");
        execute((uint8_t*)"shell");
    }

    /* get the parent process */
    PCB* Parent_PCB = PCB_array[current_PCB->parent_pid];
    
    /* update TSS */
    tss.esp0 = Parent_PCB->tss_esp0;
    tss.ss0 = KERNEL_DS;
    
    /* restore the paging */
    set_user_page(Parent_PCB->pid);
    
    /* close FD */
    fd_table* check =  current_PCB -> file_array;

    for(i=0; i<MAX_FD_NUM; i++){
        if(check[i].flags == 1){
            break;
        }
        return FAIL;
    }

    //update terminal running number
    terminals[scheduled_index].running_pid = current_PCB->parent_pid;

    close_file_array(check);
    int32_t parent_esp = current_PCB -> esp;
    int32_t parent_ebp = current_PCB -> ebp;
    PCB_array[current_PCB->pid] = NULL; 
    current_PCB = Parent_PCB;

    uint16_t E_status=(uint16_t)status;
    if (status == EXCEPTION_STATUS){E_status = EXCEPTION_RETURNVALUE;}
    
    
    if (PCB_array[3]==NULL && PCB_array[4]==NULL && PCB_array[5]==NULL){
        PT_for_video[user_PT_index].p=0;// present set to present
        int32_t PD_entry_index = user_video_start_address >>offset_22; 
        PD[PD_entry_index].k.p=0; // present set to present
    }
    // //close video page:
    


 


    asm volatile(
        "movl %%cr3,%%eax   ;"
        "movl %%eax,%%cr3   ;"
        : : : "eax","cc"
    );



    sti();
    asm volatile(
        "movl %0, %%esp ;"
        "movl %1, %%ebp ;"
        "xorl %%eax,%%eax;"
        "movw %2, %%ax ;"
        "leave;"
        "ret;"
        : 
        : "r" (parent_esp), "r" (parent_ebp), "r"(E_status)
        : "esp", "ebp", "eax"
        );
    return SUCCESS;
}



//helper function
/* 
 * close_file_array fd_table* file_array
 *   Description: close the file_array i.e. set all the values in the file array to NULL
 *        Inputs: fd_table* file_array
 *        Output: None
 *        Return: None
 */

void close_file_array(fd_table* file_array){
    int fd;
    for (fd = 2; fd<MAX_FD_NUM;fd++){
        close(fd);
    }
    file_array[0].flags = 0;
    file_array[0].ops.close(0);
    file_array[1].flags = 0;
    file_array[1].ops.close(1);
}


/* 
 * execute (const uint8_t* command)
 *   Description: System call which attempts to load and execute a new program, handing off the processor to the new program
 *                until it terminates.
 *        Inputs: command
 *        Output: None
 *        Return: fake return value --> SUCCESS (0) / FAIL (-1)
 */
int32_t execute(const uint8_t* command){
    cli(); //unable interupt
    int8_t filename[MAX_FILE_LEN];
    int8_t arg[BUF_SIZE];
    uint8_t ELF_buf[ELF_buf_size];

    dentry  tar_dentry;
    int32_t new_pid=-1; //the idx of pcb_array
    int32_t i=0;
    if(command==NULL){
        return FAIL;
    }
    if(split_argument(command,filename, arg)==FAIL){ //split the command into arg and filename
        return FAIL;
    }
    //check if shell>3
    int32_t g=0;//index
    uint8_t* SHELL=(uint8_t*)"shell";
    int32_t is_shell=1;
    for(g=0;g<5;g++){
        if (SHELL[g]!=filename[g]){ 
            is_shell = 0;
            break;}
    }
    if(program_count >=6 ){
        return 0;
    }
    
    // if (is_shell==1){
    //     shell_count = shell_count + 1;
    // }

    
    if(read_dentry_by_name((uint8_t*)filename,&tar_dentry)==FAIL){//check if it is a file in file system
        return FAIL;
    }

    if(read_data(tar_dentry.inode_num, 0, ELF_buf , LEN_ELF)==FAIL){
        return FAIL;
    }
    if(ELF_buf[0]!=mgc_num0|| ELF_buf[1]!=mgc_num1||ELF_buf[2]!=mgc_num2||ELF_buf[3]!=mgc_num3){// check whether file is excutable
        return FAIL;
    }
    

    for(i=0;i<pcb_array_size;i++){   //map new process to a free pcb position
        if(PCB_array[i]==NULL){
            new_pid=i;//new_pid is the idx of the PCB_array
            break;
        }
    }

    if(new_pid==-1){
        return FAIL;
    }

    set_user_page(new_pid);

    //copy file data to newly allocated page 
    uint32_t length= ((inode*)(inode_men_start+tar_dentry.inode_num))->length;
    //uint8_t data_buf[length];
    if(read_data(tar_dentry.inode_num, 0, (uint8_t*)file_data_base_add ,length)==FAIL){
        return FAIL;
    }
    //memcpy((void*)file_data_base_add, (const void*)data_buf,length );

    //create new PCB
    create_new_PCB(arg,new_pid);


    //update current PCB;
    current_PCB=PCB_array[new_pid];
    program_count+=1;


    //save process information to current_PCB
    current_PCB->pid=new_pid;
    if(new_pid==0 || new_pid==1|| new_pid==2){
        current_PCB->parent_pid=-1;
        terminals[current_terminal_number].running_pid=new_pid;

    }
    else{
        current_PCB->parent_pid=terminals[current_terminal_number].running_pid;
        terminals[current_terminal_number].running_pid=new_pid;
    }

    
    // current_PCB->parent_pid=terminals[current_terminal_number].running_pid;
    // //update terminal's running_pid
    // terminals[current_terminal_number].running_pid = new_pid;
    current_PCB->tss_esp0=(uint32_t)current_PCB +KERNAL_STACK_SIZE-avoid_page_fault_fence;
    if(is_shell == 1){
        current_PCB->shell_indicator=1;
    }
    else{
        current_PCB->shell_indicator=0;
    }
    


    //prepare for "context switch"
    uint32_t SS=USER_DS;
    uint32_t ESP=user_page_start_address + user_stack_size -avoid_page_fault_fence;//bottom of virtual address
    //EFLAG
    uint32_t CS=USER_CS;
    uint32_t return_address=*(uint32_t*)(((int8_t*)file_data_base_add) +24);

    //create fake "iret stack structure" and change tss
    tss.esp0 = (uint32_t)current_PCB +KERNAL_STACK_SIZE -avoid_page_fault_fence;
    tss.ss0 = KERNEL_DS;


    //save old esp,ebp
    asm volatile("movl %%esp,%%eax;"
        : "=a"(current_PCB->esp)
        :
        : "memory");

    asm volatile("movl %%ebp,%%eax;" 
        : "=a"(current_PCB->ebp)
        :
        : "memory");

        
    sti(); //enable interupt

    asm volatile(
            "movw  %%ax, %%ds;"
            "pushl %%eax;"
            "pushl %%ebx;"
            "pushfl  ;"
            "pushl %%ecx;"
            "pushl %%edx;"
            "IRET"
            :
            : "a"(SS), "b"(ESP), "c"(CS), "d"(return_address)
            : "cc", "memory"
        );

    return 0;
}



/* 
 * create_new_PCB(int8_t* arg, int32_t new_pid)
 *   Description: create a new process control block, which contains stdin and stdout
 *        Inputs: arg: not for this checkpoint
 *                new_pid: the pid of the new PCB
 *        Output: None
 *        Return: None
 */
void create_new_PCB(int8_t* arg, int32_t new_pid){
    int i=0;
    PCB* cur_pcb=(PCB*) (kernel_end_add-pcb_stack_size*(new_pid+1));
    PCB_array[new_pid]=cur_pcb;

    PCB_array[new_pid]->file_array[0].file_position=0;
    PCB_array[new_pid]->file_array[0].inode_index=NULL;
    PCB_array[new_pid]->file_array[0].flags=1;
    PCB_array[new_pid]->file_array[0].ops.open=bad_call_open;
    PCB_array[new_pid]->file_array[0].ops.read=terminal_read;
    PCB_array[new_pid]->file_array[0].ops.write=bad_call_write;
    PCB_array[new_pid]->file_array[0].ops.close=bad_call_close;


    PCB_array[new_pid]->file_array[1].file_position=0;
    PCB_array[new_pid]->file_array[1].inode_index=NULL;
    PCB_array[new_pid]->file_array[1].flags=1;
    PCB_array[new_pid]->file_array[1].ops.open=bad_call_open;
    PCB_array[new_pid]->file_array[1].ops.read=bad_call_read;
    PCB_array[new_pid]->file_array[1].ops.write=terminal_write;
    PCB_array[new_pid]->file_array[1].ops.close=bad_call_close;

    for (i=2;i<MAX_FD_NUM;i++){
        PCB_array[new_pid]->file_array[i].flags=0;
    }
    
    PCB_array[new_pid]->arg_buffer=(uint8_t *)arg;
    return ;
}


/* 
 * getargs (uint8_t* buf, int32_t nbytes)
 *   Description: get the args in current PCB and copy it into a buffer which is in user space
 *        Inputs: buf: the pointer of the buffer which we should copy into
 *                nbytes: the length of bytes contained in buf
 *        Output: copy the args into the buffer
 *        Return: FAIL (-1) / SUCCESS (0)
 */
int32_t getargs (uint8_t* buf, int32_t nbytes){
    /* check whether the buffer is in the user space */
    if( (int32_t)buf < USER_SPACE_START || (int32_t)buf + nbytes > USER_SPACE_END - 4 ){
        return FAIL;
        }
    /* if there is no args return FAIL */
    if (current_PCB->arg_buffer[0] == NULL){
        return FAIL;
    }
    /* if there is args, get the args into buffer */
    strncpy((int8_t *)buf, (const int8_t *)current_PCB->arg_buffer, (uint32_t)nbytes);         
    return SUCCESS;
}







/* 
 * bad_call_open(const uint8_t* filename)
 *   Description: a fake open system call
 *        Inputs: filename
 *        Output: None
 *        Return: FAIL (-1)
 */
int32_t bad_call_open(const uint8_t* filename){
    return FAIL;
}

/* 
 * bad_call_write(int32_t fd, const void* buf, int32_t nbytes)
 *   Description: a fake write system call
 *        Inputs: fd: file discriptor
 *                buf: the buffer which contains the content
 *                nbytes: the length of bytes contained in buf
 *        Output: None
 *        Return: FAIL (-1)
 */
int32_t bad_call_write(int32_t fd, const void* buf, int32_t nbytes){
    return FAIL;
}

/* 
 * bad_call_close(int32_t fd)
 *   Description: a fake close system call
 *        Inputs: fd: file discriptor
 *        Output: None
 *        Return: FAIL (-1)
 */
int32_t bad_call_close(int32_t fd){
    return FAIL;
}

/* 
 * bad_call_read(int32_t fd, void* buf,uint32_t length)
 *   Description: a fake read system call
 *        Inputs: fd: file discriptor
 *                buf: the buffer which contains the content
 *                nbytes: the length of bytes contained in buf
 *        Output: None
 *        Return: FAIL (-1)
 */
int32_t bad_call_read(int32_t fd, void* buf,uint32_t length){
    return FAIL;
}


/* 
 * split_argument(const uint8_t* command,int8_t* filename,int8_t* arg)
 *   Description: split the command into arg and filename
 *        Inputs: command 
 *                filename: buffer store filename
*                 arg: buffer store arg
 *        Output: None
 *        Return: FAIL (-1)
 */
int32_t split_argument(const uint8_t* command,int8_t* filename,int8_t* arg){
    int32_t count=0;
    int32_t flag= 0, is_filename=0, final_signal=0;
    int32_t arg_idx=0,file_idx=0;

    while (command[count]!='\0'){
        if(command[count]==' ' && flag==0){
            count++;
            continue;
        }
        if(command[count]!=' '&&is_filename==0){
            if(file_idx>=(MAX_FILE_LEN-1)){
                return FAIL;
            }
            filename[file_idx]=(int8_t)command[count];
            count++;

            file_idx++;
            flag=1;  
        }
        if(command[count]==' ' && flag==1 && final_signal==0){
            count++;
            is_filename=1;
            continue;
        }
        if(is_filename==1){//command[count]!=' '&&
            final_signal=1;
            if(arg_idx>=(BUF_SIZE-1)){
                return FAIL;
            }
            arg[arg_idx]=(int8_t)command[count];

            arg_idx++;
            count++;
        }
    }

    filename[file_idx]='\0';
    arg[arg_idx]='\0';
    return 0;
}

/* 
 * set_user_page(int32_t new_pid)
 *   Description: set new user page at 128 MB,change different virtual to physical map due to pid. 
 *        Inputs: mew_pid
 *        Output: None
 *        Return: FAIL (-1)
 */
int32_t set_user_page(int32_t new_pid){
    int32_t idx=user_page_idx;
    PD[idx].M.p=1;
    PD[idx].M.r_w=1;
    PD[idx].M.u_s=1;
    PD[idx].M.pwt=0;
    PD[idx].M.pcd=0;
    PD[idx].M.a=0;
    PD[idx].M.d=0;
    PD[idx].M.ps=1;
    PD[idx].M.g=0;
    PD[idx].M.avail=0;
    PD[idx].M.pat=0;
    PD[idx].M.reserved=0;
    PD[idx].M.ptb_add=new_pid+2;//map to coresponding physical memory position, first task begin at 8MB

    asm volatile(
        "movl %%cr3,%%eax   ;"
        "movl %%eax,%%cr3   ;"
        : : : "eax","cc"
    );
    return 0;
}

/* 
 * open(const uint8_t* filename)
 *   Description: a open system call
 *        Inputs: filename: the name of the file
 *        Output: None
 *        Return: FAIL (-1) / fd: file discriptor
 */
int32_t open(const uint8_t* filename){
    int32_t i; // loop index
    dentry current_dentry;
    int32_t fd = -1;

    /* if the file name is NULL, return fail */
    if (filename == NULL){
        printf("Filename is NULL!");
        return FAIL;
    }

    /* check whether filename is out of range */
    if ( (int)filename < USER_SPACE_START || (int)filename > USER_SPACE_END -4 ){
        return FAIL;
    }

    /* choose the first table position which is not used*/
    for(i=0; i<max_open_files; i++){
        if(current_PCB->file_array[i].flags == 0){
            fd = i;
            break;
        }
    }

    /* if no position, return fail */
    if (fd == -1){
        return FAIL;
    }

    /* if the file does not exist, return fail */
    if(read_dentry_by_name(filename,&current_dentry) == -1){
        //printf("The file is not exist!");
        return FAIL; 
    }

    /* set the corresponding fd's value */
    current_PCB->file_array[fd].ops = fop_table[current_dentry.filetype];
    current_PCB->file_array[fd].file_position = 0;
    current_PCB->file_array[fd].flags = 1;
    current_PCB->file_array[fd].filename = (uint8_t*)filename;

    /* deal with the RTC occasion */
    if (current_dentry.filetype == FILE_TYPE_RTC){
        current_PCB->file_array[fd].inode_index = RTC_INODE_INDEX;
    }
    else{
        current_PCB->file_array[fd].inode_index = current_dentry.inode_num;
    }

    /* if there is no open function, return fail*/
    if(current_PCB->file_array[fd].ops.open(filename) == FAIL){   
        return FAIL;
    }

    return fd;
}

/* 
 * close(int32_t fd)
 *   Description: a close system call
 *        Inputs: fd: file discriptor
 *        Output: None
 *        Return: FAIL (-1) / close funciton of the certain driver
 */
int32_t close(int32_t fd){
    if ((fd <= 1) || (fd >= max_open_files) || (current_PCB->file_array[fd].flags == 0)){
        return FAIL;
    }
    current_PCB->file_array[fd].flags = 0;
    return current_PCB->file_array[fd].ops.close(fd);
}

/* 
 * read(int32_t fd, void* buf, uint32_t length)
 *   Description: a read system call
 *        Inputs: fd: file discriptor
 *                buf: the buffer which contains the content
 *                nbytes: the length of bytes contained in buf
 *        Output: None
 *        Return: FAIL (-1) / read function of the certain driver
 */
int32_t read(int32_t fd, void* buf, uint32_t length){
    /* check the fail occasions */
    if( (int)buf < USER_SPACE_START || (int)buf + length > USER_SPACE_END - 4 ){
        return FAIL;
    }
    if (length <= 0){
        return FAIL;
    }
    if (fd < 0 || fd >= max_open_files || fd == 1){
        return FAIL;
    }
    if (current_PCB->file_array[fd].flags == 0){
        return FAIL;
    }
    if (current_PCB->file_array[fd].ops.read == NULL){
        return FAIL;
    }

    /* get the return value */
    int32_t ret = (current_PCB->file_array[fd].ops.read)(fd, buf, length);
    return ret;
}

/* 
 * write(int32_t fd, const void* buf, int32_t nbytes)
 *   Description: a write system call
 *        Inputs: fd: file discriptor
 *                buf: the buffer which contains the content
 *                nbytes: the length of bytes contained in buf
 *        Output: None
 *        Return: FAIL (-1) / write function of the certain driver
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes){
    /* check the fail occasions */
    
    if (fd <= 0 || fd >= max_open_files ){
        return FAIL;
    }
    if( (int)buf < USER_SPACE_START || (int)buf + nbytes > USER_SPACE_END - 4 ){
        return FAIL;
    }
    if (nbytes <= 0){
        return FAIL;
    }
    if (buf == NULL){
        return FAIL;
    }
    if (current_PCB->file_array[fd].flags == 0){
        return FAIL;
    }
    if (current_PCB->file_array[fd].ops.write == NULL){
        return FAIL;
    }

    /* get the return value */
    int32_t ret = (current_PCB->file_array[fd].ops.write)(fd, buf, nbytes);
    return ret;
}




/* 
 * vidmap (uint8_t** screen_start)
 *   Description: a  vidmap system call
 *        Inputs: ** screen_start: a ptr to a vedio memory page 
 *        in virtual which contain a ptr to the video memory in physical
 *        Output: None
 *        Return: 
 */
int32_t vidmap (uint8_t** screen_start){
    int32_t PD_index = ((int32_t)screen_start) >>offset_22; 
    
    
    //check if screen_start is vaild
    if(   PD_index != user_PD_index          ){return FAIL;}
    if( (int32_t)screen_start < USER_SPACE_START || (int32_t)screen_start > USER_SPACE_END -avoid_page_fault_fence ){return FAIL;} 

    
    //open page

    //open PD_entry
    int32_t PD_entry_index = user_video_start_address >>offset_22; 
    PD[PD_entry_index].k.p=1; // present set to present
    PD[PD_entry_index].k.r_w=1;// enable write and read
    PD[PD_entry_index].k.u_s=1; //set supervisor priviledge level
    PD[PD_entry_index].k.pwt=0; // make write cache enable
    PD[PD_entry_index].k.pcd=0; // make page table to be cached
    PD[PD_entry_index].k.a=0; //initially set to 0, wait user to visit for first time
    PD[PD_entry_index].k.reserved=0; //basically set to 0
    PD[PD_entry_index].k.ps=0; // clear for page size 4kb
    PD[PD_entry_index].k.g=0;//not frequently use 
    PD[PD_entry_index].k.avail=0; //all 32 bits are available to software
    PD[PD_entry_index].k.ptb_add=(int) PT_for_video>>shift_twelve; //get the most significant 20 bit


    //open PT
    PT_for_video[user_PT_index].p=1;// present set to present
    PT_for_video[user_PT_index].r_w=1;//// enable write and read
    PT_for_video[user_PT_index].u_s=1; //set supervisor priviledge level
    PT_for_video[user_PT_index].pwt=0; // make write cache enable
    PT_for_video[user_PT_index].pcd=0;// make page table to be cached
    PT_for_video[user_PT_index].a=0; //initially set to 0, wait user to visit for first time
    PT_for_video[user_PT_index].d=1; //set when point to page 
    PT_for_video[user_PT_index].pat=0;//set 0 fro processor not support
    PT_for_video[user_PT_index].g=0;//frequently use, global page
    PT_for_video[user_PT_index].avail=0;//all 32 bits are available to software



    //PT_for_video[user_PT_index].ptb_add=video_memory>>shift_twelve;
   
    if(scheduled_index!=current_terminal_number){
        PT_for_video[user_PT_index].ptb_add=(video_memory+four_k*(1+scheduled_index))>>shift_twelve;
    }
    if(scheduled_index==current_terminal_number){
        PT_for_video[user_PT_index].ptb_add=video_memory>>shift_twelve;
    }
    


    
    asm volatile(
        "movl %%cr3,%%eax   ;"
        "movl %%eax,%%cr3   ;"
        : : : "eax","cc"
    );

    *screen_start = (uint8_t*)(user_video_start_address);
    
    return SUCCESS;



}
int32_t set_handler (int32_t signum, void* handler_address){return FAIL;}
int32_t sigreturn (void){return FAIL;}

