/* sys_call.S, assembly linkage for system call functions */


.global sys_call_handler, sys_call_invalid, sys_call_done, sys_call_jumptable


/* 
 * sys_call_handler
 *   Description: dispatcher function for system call function
 *        Inputs: %eax: sys_call_num,
 *                %ebx: first argument,
 *                %ecx: second argument,
 *                %edx: third argument.
 *        Output: None
 *        Return: -1 for not valid
 */
sys_call_handler:
    /* save all registers */
    pushl %esi
    pushl %edi
    pushl %esp

    /* push parameters */
    pushl %edx
    pushl %ecx
    pushl %ebx

    /* check sys_call_num (saved in eax) ?valid */
    cmpl $1, %eax   /* 1 is the minimum sys_call_num */
    jb sys_call_invalid
    cmpl $10, %eax  /* 10 is the maximum sys_call_num */
    ja sys_call_invalid

    /* if valid, call jumptable */
    call *sys_call_jumptable(,%eax,4)
    jmp sys_call_done

/* 
 * sys_call_invalid
 *   Description: returns -1 for an invalid sys_call number
 *        Inputs: None
 *        Output: None
 *        Return: -1 for not valid
 */
sys_call_invalid:
    movl $-1, %eax  /* if invaild, return -1 */

/* 
 * sys_call_done
 *   Description: handler for valid sys_call_num after call sys_call_function
 *        Inputs: None
 *        Output: None
 *        Return: None
 */
sys_call_done:
    /* pop the registers which we have pushed */
    popl %ebx
    popl %ecx
    popl %edx
    popl %esp
    popl %edi
    popl %esi

    iret

/* the jumptable for system call */
sys_call_jumptable:
    .long 0x0, halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn
/*Version 1 ML*/
