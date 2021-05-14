
.global rtc_interrupt_handler,keyboard_interrupt_handler,pit_interrupt_handler,mouse_interrupt_handler,sound_interrupt_handler




/* rtc_interrupt_handler()
 * 
 * This function is used as the handler to handle the RTC interrupt, it save all the registes and restore them after excute the handler
 * Inputs: None
 * Outputs: None
 * Side Effects: the characters on screen will alternate between a set of different characters at every fixed interval.
 */
rtc_interrupt_handler:
    pushal
    pushfl      /*save all gereral purpose register and EGLAG*/
    call __rtc_interrupt_handler__
    popfl
    popal
    iret       /*we should return to program after interrupt*/



/* keyboard_handler()
 * 
 * This function is used as the handler to handle the keyboard interrupt, it save all the registes and restore them after excute the handler
 * Inputs: None
 * Outputs: None
 * Side Effects: Send the scan code which sent by the keyboard device to our video memory to show it on the screen
 */
keyboard_interrupt_handler:
    pushal
    pushfl      /*save all gereral purpose register and EGLAG*/
    call keyboard_handler
    popfl
    popal
    iret       /*we should return to program after interrupt*/
/*Version 1 LYC*/


pit_interrupt_handler:
    pushal
    pushfl      /*save all gereral purpose register and EGLAG*/
    call pit_handler
    popfl
    popal
    iret       /*we should return to program after interrupt*/
/*Version 1 LYC*/


mouse_interrupt_handler:
    pushal
    pushfl      /*save all gereral purpose register and EGLAG*/
    call mouse_handler
    popfl
    popal
    iret       /*we should return to program after interrupt*/



sound_interrupt_handler:
    pushal
    pushfl      /*save all gereral purpose register and EGLAG*/
    call sound_handler
    popfl
    popal
    iret       /*we should return to program after interrupt*/


