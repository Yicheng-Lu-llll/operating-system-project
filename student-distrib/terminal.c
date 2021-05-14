/*terminal.c used to define four terminal functions*/

#include "terminal.h"
#include "keyboard.h"
#include "types.h"
#include "lib.h"
#include "page.h"
#include "pit.h"
#include "x86_desc.h"
#include "system_call.h"

int32_t buffer_index = 0; // the cursor index of the buffer
int32_t last_meet = 0;

/* int32_t terminal_read(fd, buf, nbytes)
 * 
 * This function is used copy the content of line buffer into the terminal buffer
 * Inputs: 
 *          fd: file discriptor
 *          buf: the terminal buffer
 *          nbytes: the number of bytes which we should copy into the buffer
 * Outputs: None
 * Return: buffer_index : the index of the cursor in the buffer
 * Side Effects: copy the content of line buffer into the terminal buffer
 */
int32_t terminal_read(int32_t fd, void* buf, uint32_t nbytes)
{
// 	if(nbytes!=LINE_BUFFER_SIZE || buf==NULL){
//         return FAIL;
//     }
// 	strncpy((int8_t*) buf, line_buffer, buffer_index);
// 	return buffer_index;
//
    //printf("terminal_read called, buffer address：%x",&buf);

    terminals[scheduled_index].terminal_read_flag = 1;
    terminals[scheduled_index].stdin_enable = 0;
    if(buf == NULL){
        return FAIL;
    }
    return copy_buffer(buf);
}


/* int32_t term_write(fd, buf, nbytes)
 * 
 * This function is used put the content of the buffer onto the screen
 * Inputs: 
 *          fd: file discriptor
 *          buf: the terminal buffer
 *          nbytes: the number of bytes which we should put
 * Outputs: None
 * Return: temp_count : the number of bytes which we have written
 * Side Effects: put the content of the buffer onto the screen (video memory)
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
    int32_t temp_count;
    cli();
    if(buf == NULL){
        return FAIL;
    }
    const char* char_buf = buf;  
    for(temp_count = 0; temp_count < nbytes; temp_count++){
        putc(char_buf[temp_count]);
    }
    sti();
    /* return # of bytes written */
    return temp_count;
}

/* int32_t term_open(filename)
 * 
 * This function is used put the content of the buffer onto the screen
 * Inputs: filename: the name of the file
 * Outputs: None
 * Return: SUCCESS 0
 * Side Effects: open a file's terminal
 */
int32_t terminal_open(const uint8_t* filename) {
    return SUCCESS;
}

/* int32_t term_close(fd, buf, nbytes)
 * 
 * This function is used put the content of the buffer onto the screen
 * Inputs: 
 *          fd: file discriptor
 * Outputs: None
 * Return: SUCCESS
 * Side Effects: close the the terminal
 */
int32_t terminal_close(int32_t fd) {
    return SUCCESS;
}

/* void clear_buffer()
 * 
 * This function is used to clear the content of the buffer
 * Inputs: None
 * Outputs: None
 * Return: None
 * Side Effects: clear the buffer
 */
void clear_buffer(char* line_buffer){
    int32_t i;

    if (line_buffer == NULL) {
        return;
    }
    for(i = 0; i<LINE_BUFFER_SIZE;i++){
        line_buffer[i] = '\0';
    }
}

/* void add_buffer()
 * 
 * This function is used to add a key in th buffer
 * Inputs: uint8_t key: the key which we want to add to the buffer
 * Outputs: None
 * Return: None
 * Side Effects: add a key into the buffer
 */
void add_buffer(char* line_buffer,uint8_t key,int buffer_idx){
    


    if (line_buffer == NULL) {
        return;
    }
    if(buffer_idx == LINE_BUFFER_SIZE){
        return;
    }
    if((buffer_idx == LINE_BUFFER_SIZE - 1) && (key != '\n')){
        return;
    }

    line_buffer[buffer_idx] = key;
}
/*Version 1 ZLH*/



TCB terminals[max_terminal_number];
int32_t current_terminal_number = 0;
int32_t previous_terminal_number = 0;


int32_t
init_terminal_structure(){
    int32_t i; // index loop;
    for(i=0; i<max_terminal_number; i++){
        terminals[i]._buffer_index = 0;
        terminals[i].cursor_x = 0;
        terminals[i].cursor_y = 0;
        terminals[i].running_pid = -1;
        terminals[i].tid = i;
        terminals[i].next_terminal = &terminals[(i+1)%max_terminal_number];

        terminals[i].current_process = NULL;
        terminals[i].stdin_enable = 0;
        terminals[i].vidmap = 0;
        terminals[i].buffer_memory_index = 0;
        terminals[i].temp_index = 0;
        terminals[i].index = 0;
    }
    return SUCCESS;
}

int is_terminal_switch = 1;
int32_t switch_terminal(int32_t terminal_number){

    is_terminal_switch = 0;
    int32_t _screen_x = 0;
    int32_t _screen_y = 0;


    // int i; // loop index
    /* if the terminal is already the terminal_number th terminal, return -1 */
    if (terminal_number == current_terminal_number){
        is_terminal_switch = 1;
        return FAIL;
    }

    /* from the terminals table, get the current and new terminal */
    // TCB* current_terminal_pointer = &terminals[current_terminal_number];
    TCB* new_terminal_pointer = &terminals[terminal_number];

    /* save the buffer index*/
    // current_terminal_pointer->buffer_index = buffer_index;

    // /* load the new terminal's information */
    // buffer_index = new_terminal_pointer->buffer_index;
    // // line_buffer = new_terminal_pointer->line_buffer;
    // for (i=0; i<LINE_BUFFER_SIZE; i++){
    //     line_buffer[i] = new_terminal_pointer->line_buffer[i];
    // }

    screen_x = new_terminal_pointer->cursor_x;
    screen_y = new_terminal_pointer->cursor_y;
    update_cursor(screen_x, screen_y);
    
    /* switch the video memory */
    previous_terminal_number = current_terminal_number;
    current_terminal_number = terminal_number;

    
    if(current_terminal_number==scheduled_index){
    
        if(last_meet == 0){
            PT_for_video[user_PT_index].ptb_add=video_memory>>shift_twelve;
            flush_TLB();
            
            memcpy(( void *)video_memory,(const void *)(video_memory+four_k*(1+scheduled_index)),four_k);
        }

        if(last_meet == 1){
            memcpy(( void *)(video_memory+four_k*(1+previous_terminal_number)) ,(const void *)backdoor,four_k);
            memcpy(( void *)video_memory,(const void *)(video_memory+four_k*(1+scheduled_index)),four_k);
        }
        
        
        last_meet = 1;
    }
    if(current_terminal_number!=scheduled_index){
        if(last_meet == 1){
            PT_for_video[user_PT_index].ptb_add=(video_memory+four_k*(1+scheduled_index))>>shift_twelve;
            flush_TLB();
            memcpy(  ( void *)(video_memory+four_k*(1+previous_terminal_number))  , (const void *)backdoor,four_k);
            memcpy(( void *)backdoor,(const void *)(video_memory+four_k*(1+current_terminal_number)),four_k);
        }


        if(last_meet == 0){
            PT_for_video[user_PT_index].ptb_add=(video_memory+four_k*(1+scheduled_index))>>shift_twelve;
            flush_TLB();
            memcpy(( void *)backdoor,(const void *)(video_memory+four_k*(1+current_terminal_number)),four_k);
            
        }
        last_meet = 0;

    }


    _screen_x = screen_x;
    _screen_y = screen_y;
    screen_x = 0;
    screen_y = 0;
    printf("This is Terminal # %d",terminal_number);
    is_terminal_switch = 1;
    update_cursor(20, 0);
    if((_screen_x != 0) || (_screen_y != 0)){
        screen_x = _screen_x;
        screen_y = _screen_y;
        update_cursor(screen_x, screen_y);
    }

    return SUCCESS;
}






