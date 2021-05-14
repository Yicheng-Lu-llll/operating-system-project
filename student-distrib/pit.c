/*pit.c used to init periodic interval timer device*/

#include "pit.h"
#include "types.h"
#include "i8259.h"
#include "lib.h"
#include "system_call.h"
#include "x86_desc.h"
#include "systemfile.h"
#include "terminal.h"


int32_t scheduled_index=-1;
int is_process_switch = 0;


/* void init_pit(void)
 * Description: init the periodic interval timer device and set the frequency to 100Hz
 * Inputs: None
 * Outputs: None
 * Return value: None
 * Side effects: Allow the pit interrupts with the frequency at 100Hz
 */

void init_pit(void){
    outb(PIT_MODE, PIT_COMMAND_PORT);
    outb((uint8_t)((uint32_t)(LATCH) && 0xff), PIT_DATA_PORT_CHANNEL0);
    outb((uint8_t)((uint32_t)(LATCH) >> 8), PIT_DATA_PORT_CHANNEL0);
    enable_irq(pit_irq_number);

   
}


/* void pit_handler(void)
 * Description: handle the pit interrupt, and 
 * Inputs: None
 * Outputs: None
 * Return value: None
 * Side effects: 
 */
void pit_handler(void){
    send_eoi(pit_irq_number);
    // printf("a");
    scheduling();
    

}

/* void scheduling(void)
 * Description: scheduling, need vedio memory switch and process switch
 * Inputs: None
 * Outputs: None
 * Return value: None
 * Side effects: 
 */
void scheduling(void){
    // if(PCB_array[0] == NULL && PCB_array[1] == NULL && PCB_array[2] == NULL ){
    //     init_shells((uint8_t*)"shell"); 
    // }
    process_video_switch();
    process_switch();


}
/* void flush_TLB(void)
 * Description: redo the TLB after the vedio mapping is change
 * Inputs: None
 * Outputs: None
 * Return value: None
 * Side effects: 
 */

void flush_TLB(void){
    asm volatile(
        "movl %%cr3,%%eax   ;"
        "movl %%eax,%%cr3   ;"
        : : : "eax","cc"
    );
}


/* void process_video_switch(void)
 * Description: change user vedio mapping based on chnage of process
 * Inputs: None
 * Outputs: None
 * Return value: None
 * Side effects: 
 */

void process_video_switch(void){
    cli();
    uint32_t next_running_terminal = (scheduled_index + 1)%3; 
    if(current_terminal_number!=next_running_terminal){
       
        if(last_meet == 0){
            PT_for_video[user_PT_index].ptb_add=(video_memory+four_k*(1+next_running_terminal))>>shift_twelve;
            flush_TLB();            
        }

        if(last_meet == 1){
            PT_for_video[user_PT_index].ptb_add=(video_memory+four_k*(1+next_running_terminal))>>shift_twelve;
            flush_TLB();  
            memcpy(  ( void *)(video_memory+four_k*(1+current_terminal_number))  , (const void *)backdoor,four_k);
        }
        last_meet = 0;
    }
    if(current_terminal_number==next_running_terminal){
                 
        if(last_meet == 0){
            PT_for_video[user_PT_index].ptb_add=video_memory>>shift_twelve;
            flush_TLB();    

            
        }
        if(last_meet == 1){
            memcpy(  ( void *)(video_memory+four_k*(1+current_terminal_number))  , (const void *)backdoor,four_k);
            memcpy(( void *)video_memory,(const void *)(video_memory+four_k*(1+next_running_terminal)),(uint32_t)four_k);
        }
      
        
        
        last_meet = 1;
    }
    sti();


}

/* 
 * int32_t init_shells(const uint8_t* command)
 *   Description:  load and execute a new shell, handing off the processor to the new program
 *                until it terminates.
 *        Inputs: command
 *        Output: None
 *        Return: fake return value --> SUCCESS (0) / FAIL (-1)
 */
int32_t init_shells(const uint8_t* command){  
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
    
    program_count+=1;

    

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



    //save process information to current_PCB
    current_PCB->pid=new_pid;
    current_PCB->parent_pid=terminals[new_pid].running_pid;
    //update terminal's running_pid
    terminals[new_pid].running_pid = new_pid;
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
    void process_switch(void)
 * Description: shift around three process, at least 3 basic shell should be run
 * Inputs: None
 * Outputs: None
 * Return value: None
 * Side effects: 
 */

void process_switch(void){


      
    // terminals[current_terminal_number].cursor_x = screen_x;
    // terminals[current_terminal_number].cursor_y = screen_y;

    // if(current_PCB!=NULL){
    //     uint32_t curr_esp = 0;
    //     uint32_t curr_ebp = 0;
    //     asm volatile(
    //         "movl %%esp, %0 ;"
    //         "movl %%ebp, %1 ;"
    //         : "=r" (curr_esp) ,"=r" (curr_ebp) 
    //     );
    //     current_PCB->cur_esp = curr_esp;
    //     current_PCB->cur_ebp = curr_ebp;
    //     current_PCB->cur_esp0 =  tss.esp0;
    // }




    // scheduled_index = (scheduled_index + 1)%3; 
    // uint32_t next_pid = terminals[scheduled_index].running_pid;
    
    // // uint32_t cur_pid = current_PCB->pid;
    // // PCB* cur_process = PCB_array[cur_pid];
    // // PCB* next_process = PCB_array[next_pid];
    // if(next_pid == -1){
    //     init_shells((uint8_t*)"shell"); 
    // }
    // else{
    //     uint32_t curr_esp = PCB_array[next_pid]->esp;
    //     uint32_t curr_ebp = PCB_array[next_pid]->ebp;
    //     tss.esp0 = PCB_array[next_pid]->tss_esp0;
    //     tss.ss0 = KERNEL_DS; 

    //     current_PCB = PCB_array[next_pid];        
    //     asm volatile(
    //         "movl %0, %%esp ;"
    //         "movl %1, %%ebp ;"
    //         : 
    //         : "r" (curr_esp), "r" (curr_ebp)
    //         : "esp", "ebp"
    //     );


    // }

    // asm volatile(
    //     "leave  ;"
    //     "ret    ;"
    // );


    cli();
    terminals[current_terminal_number].cursor_x = screen_x;
    terminals[current_terminal_number].cursor_y = screen_y;

    scheduled_index = (scheduled_index + 1)%3; 

    if(PCB_array[0] == NULL || PCB_array[1] == NULL || PCB_array[2] == NULL ){
        if(current_PCB!=NULL){
            asm volatile("movl %%esp, %0":"=r" (current_PCB->cur_esp));
            asm volatile("movl %%ebp, %0":"=r" (current_PCB->cur_ebp));    
        }
        init_shells((uint8_t*)"shell"); 
    }
    is_process_switch = 1;

    
    uint32_t next_pid = terminals[scheduled_index].running_pid;
    uint32_t cur_pid = current_PCB->pid;
    PCB* cur_process = PCB_array[cur_pid];
    PCB* next_process = PCB_array[next_pid];

   
    current_PCB = PCB_array[next_pid];
    //save cur process's esp, ebp
    asm volatile("movl %%esp, %0":"=r" (cur_process->cur_esp));
    asm volatile("movl %%ebp, %0":"=r" (cur_process->cur_ebp));
  

    //change TSS's esp0
    tss.esp0 = (uint32_t)next_process +KERNAL_STACK_SIZE-avoid_page_fault_fence;
    set_user_page(next_pid);
    
    //printf("scheduled_index is:%d \n", scheduled_index);

    sti();
    
    //load next_process's esp ebp
    asm volatile ("         \n\
        movl %0, %%esp      \n\
        movl %1, %%ebp      \n\
        "
        :
        : "r" (next_process->cur_esp), "r" (next_process->cur_ebp)
        : "memory"
    );  


//change EIP：
    asm volatile(
        "leave  ;"
        "ret    ;"
    );
    
}



