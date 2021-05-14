/*idt.c used to init interrupt description table*/

#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "idt.h"
#include "rtc.h"
#include "keyboard.h"
#include "assembly.h"
#include "sys_call.h"
#include "system_call.h"


/* construct_ex_handler
 * 
 * This function is used to build the exception handler
 * Inputs: function_name: the name of the exception
 *         string: the string we want to print as out "bule screen"
 * Outputs: None
 * Side Effects: None
 */
#define constuct_ex_handler(function_name, string)   \
void function_name(){                                \
    printf("%s\n",string);                           \
    halt(EXCEPTION_ERR);   }                         \

                                                  

constuct_ex_handler(ex_divide_error,                                        "_EXCEPTION_NUMBER:0, DIVIDE ERROR!");
constuct_ex_handler(ex_reserved,                                            "_EXCEPTION_NUMBER:1, RESERVED!");
constuct_ex_handler(ex_nmi_interrupt,                                       "_EXCEPTION_NUMBER:2, NMI INTERRUPT!");
constuct_ex_handler(ex_breakpoint,                                          "_EXCEPTION_NUMBER:3, BREAKPOINT!!");
constuct_ex_handler(ex_overflow,                                            "_EXCEPTION_NUMBER:4, OVERFLOW!");
constuct_ex_handler(ex_bound_range_exceeded,                                "_EXCEPTION_NUMBER:5, BOUND RANGE EXCEEDED!");
constuct_ex_handler(ex_invalid_opcode,                                      "_EXCEPTION_NUMBER:6, INVALID OPCODE!");
constuct_ex_handler(ex_device_not_available,                                "_EXCEPTION_NUMBER:7, DEVICE NOT AVAILABLE!");
constuct_ex_handler(ex_double_fault,                                        "_EXCEPTION_NUMBER:8, DOUBLE FAULT!");
constuct_ex_handler(ex_coprocessor_segment,                                 "_EXCEPTION_NUMBER:9, COPROCESSOR SEGMENT");
constuct_ex_handler(ex_invalid_tss,                                         "_EXCEPTION_NUMBER:10, INVALID TSS!");
constuct_ex_handler(ex_segment_not_present,                                 "_EXCEPTION_NUMBER:11, SEGMENT NOT PRESENT!");
constuct_ex_handler(ex_stack_segment_fault,                                 "_EXCEPTION_NUMBER:12, STACK SEGMENT FAULT!");
constuct_ex_handler(ex_general_protection,                                  "_EXCEPTION_NUMBER:13, GENERAL PROTECTION!");
constuct_ex_handler(ex_page_fault,                                          "_EXCEPTION_NUMBER:14, PAGE FAULT!");
constuct_ex_handler(ex_math_fault,                                          "_EXCEPTION_NUMBER:16, MATH FAULT!");
constuct_ex_handler(ex_alignment_check,                                     "_EXCEPTION_NUMBER:17, ALIGNMENT CHECK!");
constuct_ex_handler(ex_machine_check,                                       "_EXCEPTION_NUMBER:18, MACHINE CHECK!");
constuct_ex_handler(ex_simd_floating_point_exception,                       "_EXCEPTION_NUMBER:19, SIMD FLOATING POINT EXCEPTION!");


/* init_idt()
 * 
 * This function is used to init the idt
 * Inputs: None
 * Outputs: None
 * Side Effects: set the exception, interrupt, system call handler function pointers to out idt
 */
void init_idt(){
    printf("Init IDT...\n");
    cli();
    int i; //loop index

// /* An interrupt descriptor entry (goes into the IDT) */
// typedef union idt_desc_t {
//     uint32_t val[2];
//     struct {
//         uint16_t offset_15_00;
//         uint16_t seg_selector;
//         uint8_t  reserved4;
//         uint32_t reserved3 : 1;
//         uint32_t reserved2 : 1;
//         uint32_t reserved1 : 1;
//         uint32_t size      : 1;
//         uint32_t reserved0 : 1;
//         uint32_t dpl       : 2;
//         uint32_t present   : 1;
//         uint16_t offset_31_16;
//     } __attribute__ ((packed));
// } idt_desc_t;

    for (i = 0; i<NUM_VEC; i++){
        // do not touch offset!

        // init
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4 = 0x0;
        idt[i].reserved3 = 0x1;     // it should be set to 0, when interrupt handlers
        idt[i].reserved2 = 0x1;
        idt[i].reserved1 = 0x1;
        idt[i].size = 0x1;          // size<-1 means that 32-bit space for each interrupt handler 
        idt[i].reserved0 = 0x0;
        idt[i].dpl = 0;             // user level (system call) : 3, kernel level : 0
        idt[i].present = 0;         // present should be set to 1 to use
    }

    // call the SET_IDT_ENTRY function for each exception
    SET_IDT_ENTRY(idt[0], ex_divide_error);
    SET_IDT_ENTRY(idt[1], ex_reserved);
    SET_IDT_ENTRY(idt[2], ex_nmi_interrupt);
    SET_IDT_ENTRY(idt[3], ex_breakpoint);
    SET_IDT_ENTRY(idt[4], ex_overflow);
    SET_IDT_ENTRY(idt[5], ex_bound_range_exceeded);
    SET_IDT_ENTRY(idt[6], ex_invalid_opcode);
    SET_IDT_ENTRY(idt[7], ex_device_not_available);
    SET_IDT_ENTRY(idt[8], ex_double_fault);
    SET_IDT_ENTRY(idt[9], ex_coprocessor_segment);
    SET_IDT_ENTRY(idt[10], ex_invalid_tss);
    SET_IDT_ENTRY(idt[11], ex_segment_not_present);
    SET_IDT_ENTRY(idt[12], ex_stack_segment_fault);
    SET_IDT_ENTRY(idt[13], ex_general_protection);
    SET_IDT_ENTRY(idt[14], ex_page_fault);
    //SET_IDT_ENTRY[idt[15], ]; DO NOT USE!
    SET_IDT_ENTRY(idt[16], ex_math_fault);
    SET_IDT_ENTRY(idt[17], ex_alignment_check);
    SET_IDT_ENTRY(idt[18], ex_machine_check);
    SET_IDT_ENTRY(idt[19], ex_simd_floating_point_exception);

    for (i=0; i<15; i++){
        idt[i].present = 1;
    }
    for (i=16; i<20; i++){
        idt[i].present = 1;
    }


    // set the special case
    SET_IDT_ENTRY(idt[RTC_IDT_INDEX],rtc_interrupt_handler);
    idt[RTC_IDT_INDEX].present = 1;
    idt[RTC_IDT_INDEX].reserved3 = 0;

    SET_IDT_ENTRY(idt[KEYBOARD_IDT_INDEX],keyboard_interrupt_handler);
    idt[KEYBOARD_IDT_INDEX].present = 1;
    idt[RTC_IDT_INDEX].reserved3 = 0;
    

    SET_IDT_ENTRY(idt[SYSTEM_IDT_INDEX],sys_call_handler);
    idt[SYSTEM_IDT_INDEX].present = 1;
    idt[SYSTEM_IDT_INDEX].dpl = 3;  


    SET_IDT_ENTRY(idt[PIT_IDT_INDEX],pit_interrupt_handler);
    idt[PIT_IDT_INDEX].present = 1;
    idt[RTC_IDT_INDEX].reserved3 = 0;

    SET_IDT_ENTRY(idt[MOUSE_INDEX],mouse_interrupt_handler);
    idt[MOUSE_INDEX].present = 1;
    idt[MOUSE_INDEX].reserved3 = 0;

    SET_IDT_ENTRY(idt[SOUND_INDEX],sound_interrupt_handler);
    idt[SOUND_INDEX].present = 1;
    idt[SOUND_INDEX].reserved3 = 0;

    lidt(idt_desc_ptr);
    sti();
    return;
}



