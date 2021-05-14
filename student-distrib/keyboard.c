/*keyboard.c used to init keyboard device*/

#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "keyboard.h"
#include "types.h"
#include "terminal.h"
#include "pit.h"
#include "systemfile.h"
#include "sound.h"


#define KEYBOARD_IRQ 1
#define KEYBOARD_PORT_DATA 0x60

#define SCANCODESIZE 58
#define SINGLECOMMA  39
#define DOUBLECOMMA  34
#define LOW_EIGHT_BITS 0xFF

#define LEFT_SHIFT_PRESSED 0x2A
#define RIGHT_SHIFT_PRESSED 0x36
#define LEFT_SHIFT_RELEASED 0xAA
#define RIGHT_SHIFT_RELEASED 0xB6
#define LEFT_CTRL_PRESSED 0x1D
#define LEFT_CTRL_RELEASED 0x9D
#define CAPSLOCK_PRESSED 0x3A
#define CAPSLOCK_RELEASED 0xBA
#define ALT_PRESSED 0x38
#define ALT_RELEASED 0xB8
#define F1  0x3B
#define F2  0x3C
#define F3  0x3D
#define F4  0x3E
#define F5  0x3F
#define ESC 0x01
#define TAB_PRESSED 0x0F
#define UP_PRESSED 0x48
#define DOWN_PRESSED 0X50

unsigned char key2ascii_map[SCANCODESIZE][2] = {
    {0x0, 0x0}, // 0x00 not use
    {0x0, 0x0}, // 0x01 esc
    /* 0x02 - 0x0e, "1" to backspace */
    {'1', '!'}, 
    {'2', '@'},
    {'3', '#'}, 
    {'4', '$'},
    {'5', '%'},
    {'6', '^'},
    {'7', '&'}, 
    {'8', '*'},
    {'9', '('}, 
    {'0', ')'},
    {'-', '_'}, 
    {'=', '+'},
    {'\b', '\b'}, //backspace
    /* 0x0f - 0x1b, tab to "}" */
    {' ', ' '}, // tab
    {'q', 'Q'}, 
    {'w', 'W'},
    {'e', 'E'}, 
    {'r', 'R'},
    {'t', 'T'}, 
    {'y', 'Y'},
    {'u', 'U'}, 
    {'i', 'I'},
    {'o', 'O'}, 
    {'p', 'P'},
    {'[', '{'}, 
    {']', '}'},
    /* 0x1c - 0x28, enter to "'"*/
    {'\n', '\n'}, // enter
    {0x0, 0x0}, // Left Ctrl
    {'a', 'A'},
    {'s', 'S'},
    {'d', 'D'}, 
    {'f', 'F'},
    {'g', 'G'}, 
    {'h', 'H'},
    {'j', 'J'}, 
    {'k', 'K'},
    {'l', 'L'}, 
    {';', ':'},
    {SINGLECOMMA, DOUBLECOMMA},
    /* 0x29 - 0x39 "`" to Spacebar*/
    {'`', '~'},
    {0x0, 0x0}, // Left Shift
    {'\\', '|'},
    {'z', 'Z'}, 
    {'x', 'X'},
    {'c', 'C'}, 
    {'v', 'V'},
    {'b', 'B'}, 
    {'n', 'N'},
    {'m', 'M'}, 
    {',', '<'},
    {'.', '>'}, 
    {'/', '?'},
    {0x0, 0x0}, // Right Shift
    {0x0, 0x0},
    {0x0, 0x0}, 
    {' ', ' '}, 

};


/* Define the buffers to detect whether the special scancode is inputted */
/* 0 is unpressed, 1 is pressed*/
uint8_t Ctrl_Buffer = 0;
uint8_t Shift_Buffer = 0;
uint8_t CapsLock_Buffer = 0;
uint8_t Backspace_Buffer = 0;
uint8_t Alt_Buffer = 0;
/* Define the state buffer to record the state of CapsLock */
/* 0 is lowercases, 1 is capitals*/
uint8_t CapsLock_state = 0;


/* init_keyboard()
 * 
 * This function is used init the keyboard device
 * Inputs: None
 * Outputs: None
 * Side Effects: Enable the keyboard begins the interrupt
 */
void
init_keyboard(void){
    // cli();
    printf("Init keyboard...\n");
    enable_irq(KEYBOARD_IRQ);
    // sti();
}

/* keyboard_handler()
 * 
 * This function is used as the handler to handle the keyboard interrupt
 * Inputs: None
 * Outputs: None
 * Side Effects: Send the scan code which sent by the keyboard device to our video memory to show it on the screen
 */
void
keyboard_handler(void){

    cli();
    send_eoi(KEYBOARD_IRQ); // send the signal of end_of_interrupt. This instruction must be here!!! Otherwise the eoi signal will not be sent, other interrupts will not be allowed to happen
    uint8_t scan_code = inb(KEYBOARD_PORT_DATA) & LOW_EIGHT_BITS;
    uint8_t keyprinted = key2ascii_map[scan_code][0]; // get the key which we want to printed, init to the lowercase

    /* for the special scancode, return */
    if (special_scancode_handler(scan_code) == 1) {
        sti();
        return; 
    }


    if (scan_code < SCANCODESIZE && scan_code > 0x01){
     /* handle the CTRL+L, clear the screen */
        if (Ctrl_Buffer == 1){
            if (keyprinted == 'l'){
                clear();
                sti();
                return;
            }
        }

        if (keyprinted == '\b'){
            if(terminals[current_terminal_number]._buffer_index == 0){
                return;
            }
            terminals[current_terminal_number]._buffer_index--;
            terminals[current_terminal_number].line_buffer[terminals[current_terminal_number]._buffer_index] = '\0'; 
            backspace();
            return;
        }

        if((terminals[current_terminal_number]._buffer_index == 127) && (keyprinted != '\n')){
            return;
        }
        if(keyprinted == '\n'){

            terminals[current_terminal_number].stdin_enable = 1;
            putc_normal(keyprinted);

            add_buffer(terminals[current_terminal_number].line_buffer,keyprinted,terminals[current_terminal_number]._buffer_index);
            terminals[current_terminal_number]._buffer_index++;







            if(terminals[current_terminal_number].terminal_read_flag == 0){ //This is used to solve the problem of terminal read when pressing enter
                clear_buffer(terminals[current_terminal_number].line_buffer);
                terminals[current_terminal_number]._buffer_index = 0;
            }

            return;
        }

        /* handle the not special keys */
        if (keyprinted >= 'a' && keyprinted <= 'z'){ // for the letters
            if (CapsLock_state != Shift_Buffer){ // the case to print the capitals
                keyprinted = key2ascii_map[scan_code][1];
            }
        }
        else{
            if (Shift_Buffer == 1){
               keyprinted = key2ascii_map[scan_code][1]; 
            }
        }

        add_buffer(terminals[current_terminal_number].line_buffer,keyprinted,terminals[current_terminal_number]._buffer_index);
        if(terminals[current_terminal_number]._buffer_index < LINE_BUFFER_SIZE){
            terminals[current_terminal_number]._buffer_index++;
            putc_normal(keyprinted);
        }

    }
    sti();
}



/* special_scancode_handler()
 * 
 * This function is used as the handler to handle the special scancode
 * Inputs: scan_code: got from the keyboard
 * Outputs: 1 for success, 0 for fail
 * Side Effects: Change the special key buffers
 */
int32_t special_scancode_handler(uint8_t scan_code){
    switch(scan_code){
        /* Shift pressed, change the ShiftBuffer to 1 */
        case LEFT_SHIFT_PRESSED:
            Shift_Buffer = 1;
            return 1;
        case RIGHT_SHIFT_PRESSED:
            Shift_Buffer = 1;
            return 1;
        /* Shift released, change the ShiftBuffer to 1 */
        case LEFT_SHIFT_RELEASED:
            Shift_Buffer = 0;
            return 1;
        case RIGHT_SHIFT_RELEASED:
            Shift_Buffer = 0;
            return 1;
        /* Ctrl pressed, change the ShiftBuffer to 1 */
        case LEFT_CTRL_PRESSED:
            Ctrl_Buffer = 1;
            return 1;
        /* Ctrl released, change the ShiftBuffer to 1 */
        case LEFT_CTRL_RELEASED:
            Ctrl_Buffer = 0;
            return 1;
        /* CapsLock pressed, change the CapsLock buffer to 1 */
        case CAPSLOCK_PRESSED:
            CapsLock_Buffer = 1;
            return 1;
        case CAPSLOCK_RELEASED:
            CapsLock_Buffer = 0;
            CapsLock_state = 1 - CapsLock_state; // change the state, if 0, change to 1, if 1, change to 0.
            return 1;
        case ALT_PRESSED:
            Alt_Buffer = 1;
            return 1;
        case ALT_RELEASED:
            Alt_Buffer = 0;
            return 1;
        case F1:
            if(Alt_Buffer==1){switch_terminal(0);}
            return 1;
        case F2:
            if(Alt_Buffer==1){switch_terminal(1);}
            return 1;
        case F3:
            if(Alt_Buffer==1){switch_terminal(2);}
            return 1;
        case F4:
            printf("F4");
            enable_irq(irq_sound);
   

    
            play_back_sound("music");
            return 1;
        case F5:
            printf("F5");
            DSP_write(0xD0);
            return 1;
        case ESC:
            return 1;
        case TAB_PRESSED:
            tab_handler();
            return 1;
        case UP_PRESSED:
            up_handler();
            return 1;
        case DOWN_PRESSED:
            down_handler();
            return 1;
        default:
            return 0;
    }
}



/* copy_buffer()
 * 
 * This function is used to copy buffer into terminals
 * Inputs: buf: the pointer to the copied buffer
 * Outputs: None
 * Side Effects: Copy the buffer into terminals
 */
int32_t copy_buffer(void* buf){
    int32_t num;
    int i;
    int j;
    while(terminals[scheduled_index].stdin_enable != 1){}

///////////////////////////////////////////////////////////
    int buffer_index = terminals[scheduled_index]._buffer_index;
    num = buffer_index;

    char line_buffer[LINE_BUFFER_SIZE];

    if (terminals[current_terminal_number].temp_index == 10){
        for(i = 0;i<9;i++){
            for(j = 0;j<LINE_BUFFER_SIZE; j++){
               terminals[current_terminal_number].temp[i][j] = terminals[current_terminal_number].temp[i+1][j];
            }
        }
        terminals[current_terminal_number].temp_index--;
    }


    for (i=0; i<LINE_BUFFER_SIZE; i++){
        line_buffer[i] = terminals[current_terminal_number].line_buffer[i];
        terminals[current_terminal_number].temp[terminals[current_terminal_number].temp_index][i] = terminals[current_terminal_number].line_buffer[i];
    }
    terminals[current_terminal_number].temp_index++;
    terminals[current_terminal_number].index = terminals[current_terminal_number].temp_index;

    // if (terminals[current_terminal_number].temp_index >10){
    //     terminals[current_terminal_number].temp_index = 10;
    // }

    strncpy((int8_t*) buf, line_buffer, buffer_index);

    clear_buffer(terminals[scheduled_index].line_buffer);
    terminals[scheduled_index]._buffer_index= 0;

    terminals[scheduled_index].terminal_read_flag = 0;
    return num;
}




void tab_handler(void){
    int i;
    int n;
    if (terminals[current_terminal_number]._buffer_index >= FILENAME_LEN){
        return;
    }

    int8_t current_command[FILENAME_LEN+1];
    for (i=0 ; i<FILENAME_LEN+1; i++){
        current_command[i] = NULL;
    }

    int32_t read_count = 0;
    int32_t buf_read_pos = 0;
    while(buf_read_pos != terminals[current_terminal_number]._buffer_index){
        if (terminals[current_terminal_number].line_buffer[buf_read_pos] == ' '){
            buf_read_pos++;
            read_count = 0;
            for (i=0 ; i<FILENAME_LEN+1; i++){
                current_command[i] = NULL;
            }
            continue;
        }
        current_command[read_count] = terminals[current_terminal_number].line_buffer[buf_read_pos];
        buf_read_pos++;
        read_count++;
    }

    if (read_count>FILENAME_LEN){
        return;
    }

    current_command[read_count] = '\0';

    int8_t current_filename[FILENAME_LEN+1];
    for (i=0 ; i<FILENAME_LEN+1; i++){
        current_filename[i] = NULL;
    }

    int8_t result[FILENAME_LEN+1];
    for (i=0 ; i<FILENAME_LEN+1; i++){
        result[i] = NULL;
    }

    // int32_t flag = 0;
    int32_t target_length = 0;

    int32_t current_filename_length = 0;
    for(n=0; n<17; n++){
        current_filename_length = sysdir_read_for_tab(current_filename,n);
        if(0 == strncmp(current_filename, current_command, read_count)){
            // if(flag == 1) {return;}
            strncpy(result, current_filename, FILENAME_LEN+1);
            // flag = 1;
            target_length = current_filename_length;
        }
    }

    int32_t difference_index = 0;
    while(result[difference_index] == current_command[difference_index]){
        difference_index++;
    }

    int32_t current_index = 0;
    current_index = difference_index;
    while(current_index < target_length){
        putc_normal(result[current_index]);
        terminals[current_terminal_number].line_buffer[terminals[current_terminal_number]._buffer_index] = result[current_index];
        terminals[current_terminal_number]._buffer_index++;
        current_index++;

        if(terminals[current_terminal_number]._buffer_index == (LINE_BUFFER_SIZE - 1)){
            return;
        }
    }
}


void up_handler(void){
    if (terminals[current_terminal_number].index<1){
        return;
    }
    int a = (terminals[current_terminal_number].index - 1);
    int i;
    for(i = 0;i<129;i++){
        if(terminals[current_terminal_number]._buffer_index == 0){
            continue;
        }
        terminals[current_terminal_number]._buffer_index--;
        terminals[current_terminal_number].line_buffer[terminals[current_terminal_number]._buffer_index] = '\0'; 
        backspace();
    }

    for (i = 0;i<LINE_BUFFER_SIZE;i++){
        if(terminals[current_terminal_number].temp[a][i] == '\n'){
            continue;
        }
        terminals[current_terminal_number].line_buffer[i] = terminals[current_terminal_number].temp[a][i];
        if(terminals[current_terminal_number].temp[a][i] != '\0'){
            putc_normal(terminals[current_terminal_number].temp[a][i]);
            terminals[current_terminal_number]._buffer_index ++;
        }
    }
    terminals[current_terminal_number].index--;
}

void down_handler(void){
    if (terminals[current_terminal_number].index>terminals[current_terminal_number].temp_index-1){
        return;
    }
    int b = (terminals[current_terminal_number].index+1);
    int i;
    for(i = 0;i<129;i++){
        if(terminals[current_terminal_number]._buffer_index == 0){
            continue;
        }
        terminals[current_terminal_number]._buffer_index--;
        terminals[current_terminal_number].line_buffer[terminals[current_terminal_number]._buffer_index] = '\0'; 
        backspace();
    }

    for (i = 0;i<LINE_BUFFER_SIZE;i++){
        if(terminals[current_terminal_number].temp[b][i] == '\n'){
            continue;
        }
        terminals[current_terminal_number].line_buffer[i] = terminals[current_terminal_number].temp[b][i];
        if(terminals[current_terminal_number].temp[b][i] != '\0'){
            putc_normal(terminals[current_terminal_number].temp[b][i]);
            terminals[current_terminal_number]._buffer_index ++;
        }
    }
    terminals[current_terminal_number].index++;
}


int32_t sysdir_read_for_tab(void* buf, uint32_t  n){
    //int32_t test=10/0;//used to test ecxeption handler

    dentry tar_dentry;
    int i;
    //if(current_PCB->file_array[fd].filename!=(uint8_t*)"."){return FAIL; }
    if(n>=17){return 0;}//we have 17 files.

    int return_val= read_dentry_by_index(n, &tar_dentry); //check return is valid?
	if(return_val== -1){
		return FAIL;
	}
    for (i = 0; i < MAX_FILE_NAME_LEN+1; i++){ 
        ((int8_t*)(buf))[i] = '\0'; // init buf all with \0
    }

    strncpy((int8_t*)buf, (const int8_t*)tar_dentry.filename,strlen((int8_t*)tar_dentry.filename));
    ((int8_t*)(buf))[MAX_FILE_NAME_LEN] = '\0';//32 is the last position of a filename.
    if( (int32_t)strlen((int8_t*)tar_dentry.filename) ==MAX_FILE_NAME_LEN+1){return MAX_FILE_NAME_LEN;}
    return (int32_t)strlen((int8_t*)tar_dentry.filename);



}
